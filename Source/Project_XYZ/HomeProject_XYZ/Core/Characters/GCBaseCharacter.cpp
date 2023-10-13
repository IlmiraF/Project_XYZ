// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../Components/LedgeDetectorComponent.h"
#include "Curves/CurveVector.h"
#include "Components/CapsuleComponent.h"
#include "../Actors/Interactive/InteractiveActor.h"
#include "../Actors/Interactive/Environment/Ladder.h"
#include "../Actors/Interactive/Environment/Zipline.h"
#include "../../../XYZ_ProjectTypes.h"
#include "../Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "../Components/CharacterComponents/CharacterAttributeComponent.h"
#include "../Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "GameFramework/PhysicsVolume.h"
#include "../Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "AIController.h"
#include "../Actors/Environment/PlatformTrigger.h"
#include "Net/UnrealNetwork.h"
#include "../Actors/Interactive/Interface/Interactive.h"
#include "Components/WidgetComponent.h"
#include "../Actors/Equipment/EquipableItem.h"
#include "../UI/Widget/World/GCAttributeProgressBar.h"
#include "../Inventory/Items/InventoryItem.h"
#include "../Components/CharacterComponents/CharacterInventoryComponent.h"

AGCBaseCharacter::AGCBaseCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UGCBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GCBaseCharacterMovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(GetCharacterMovement());

	IKScale = GetActorScale3D().Z;
	IKTraceDistance = 50.0f * IKScale;

	LedgeDetectorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>(TEXT("Ledge Detector"));

	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;

	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributeComponent>(TEXT("CharacterAttributes"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipment"));
	CharacterInventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("InventoryComponent"));

	HealthBarProgressComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarProgressComponent"));
	HealthBarProgressComponent->SetupAttachment(GetCapsuleComponent());
}

void AGCBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGCBaseCharacter, bIsMantling);
}

void AGCBaseCharacter::OnLevelDeserialized_Implementation()
{
}

void AGCBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		FGenericTeamId TeamId((uint8)Team);
		AIController->SetGenericTeamId(TeamId);
	}
}

void AGCBaseCharacter::ChangeCrouchState()
{
	if (!GetMovementComponent()->IsCrouching() && !GCBaseCharacterMovementComponent->IsProning() && !GCBaseCharacterMovementComponent->IsMantling())
	{
		Crouch();
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, FString::Printf(TEXT("Crouch")));
	}
}

void AGCBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void AGCBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

void AGCBaseCharacter::UpdateStamina(bool bIsOutOfStamina)
{
	GetBaseCharacterMovementComponent()->SetIsOutOfStamina(bIsOutOfStamina);
}

void AGCBaseCharacter::ChangeProneState()
{
	if (GetMovementComponent()->IsCrouching() && !GCBaseCharacterMovementComponent->IsProning())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("Prone")));
		Prone();
	}
	else if(!GetMovementComponent()->IsCrouching() && GCBaseCharacterMovementComponent->IsProning())
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, FString::Printf(TEXT("UnProne")));
		UnProne(false);
	}
}

void AGCBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryChangeSprintState(DeltaTime);

	//IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForASocket(RightFootSocketName), DeltaTime, IKInterpSpeed);
	//IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForASocket(LeftFootSocketName), DeltaTime, IKInterpSpeed);

	TraceLineOfSight();
}

void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	CharacterScale = GetActorScale3D().Z;
	CapsuleHalfHeightBase = (GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()) * CharacterScale;
	CapsuleHalfHeightCrouch = GCBaseCharacterMovementComponent->GetCrouchedHalfHeight() * CharacterScale;

	CharacterAttributesComponent->OnDeathEvent.AddUObject(this, &AGCBaseCharacter::OnDeath);
	CharacterAttributesComponent->OutOfStaminaEvent.AddUObject(this, &AGCBaseCharacter::UpdateStamina);

	InitializeHealthProgress();

	if (bIsSignificanceEnabled)
	{
		USignificanceManager* SignificanceManager = FSignificanceManagerModule::Get(GetWorld());
		if (IsValid(SignificanceManager))
		{
			SignificanceManager->RegisterObject(
				this, 
				SignificanceTagCharacter, 
				[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint) -> float
				{
					return SignificanceFunction(ObjectInfo, ViewPoint);
				},
				USignificanceManager::EPostSignificanceType::Sequential,
				[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal)
				{
					PostSignificanceFunction(ObjectInfo, OldSignificance, Significance, bFinal);
				}
			);
		}
	}

}

void AGCBaseCharacter::EndPlay(const EEndPlayReason::Type Reason)
{
	if (OnInteractableObjectFound.IsBound())
	{
		OnInteractableObjectFound.Unbind();
	}
	Super::EndPlay(Reason);
}

void AGCBaseCharacter::RecalculateBaseEyeHeight()
{
	if (bIsCrouched)
	{
		BaseEyeHeight = CrouchedEyeHeight;
	}
	else if (bIsProned)
	{
		BaseEyeHeight = PronedEyeHeight;
	}
	else
	{
		Super::RecalculateBaseEyeHeight();
	}
}

void AGCBaseCharacter::Jump()
{
	if (!GCBaseCharacterMovementComponent->IsProning())
	{
		Super::Jump();
	}
	else
	{
		UnProne(true);
	}
}

void AGCBaseCharacter::Mantle(bool bForce)
{
	if (!(CanMantle() || bForce))
	{
		return;
	}

	FLedgeDescription LedgeDescription;
	if (LedgeDetectorComponent->DetectLedge(LedgeDescription) && !GetBaseCharacterMovementComponent()->IsMantling())
	{
		bIsMantling = true;

		FMantlingMovementParameters MantlingParameters;
		MantlingParameters.InitialLocation = GetActorLocation();
		MantlingParameters.InitialRotation = GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDescription.Location;
		MantlingParameters.TargetRotation = LedgeDescription.Rotation;

		float MantlingHeight = (MantlingParameters.TargetLocation - MantlingParameters.InitialLocation).Z;
		const FMantlingSettings& MantlingSettings = GetMantlingSettings(MantlingHeight);

		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);

		MantlingParameters.Duration = MaxRange - MinRange;

		MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;

		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);

		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;

		if (IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
		{
			GetBaseCharacterMovementComponent()->StartMantle(MantlingParameters);
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.0f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
		OnMantle(MantlingSettings, MantlingParameters.StartTime);
	}
}

void AGCBaseCharacter::OnRep_IsMantling(bool bWasMantling)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		if (!bWasMantling && bIsMantling)
		{
			Mantle(true);
		}
	}
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && !GetBaseCharacterMovementComponent()->IsMantling();
}

void AGCBaseCharacter::ClimbLadderUp(float Value)
{
	if (GetBaseCharacterMovementComponent()->IsOnLadder() && !FMath::IsNearlyZero(Value))
	{
		FVector LadderUpVector = GetBaseCharacterMovementComponent()->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

void AGCBaseCharacter::InteractWithLadder()
{
	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		GetBaseCharacterMovementComponent()->DetachFromLadder(EDetachFromLadderMethod::JumpOff);
	}
	else
	{
		const ALadder* AvailableLadder = GetAvailableLadder();
		if (IsValid(AvailableLadder))
		{
			if (AvailableLadder->GetIsOnTop())
			{
				PlayAnimMontage(AvailableLadder->GetAttachFromTopAnimMontage());
			}
			GetBaseCharacterMovementComponent()->AttachToLadder(AvailableLadder);
		}
	}
}

void AGCBaseCharacter::InteractWithZipline()
{
	if (GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		GetBaseCharacterMovementComponent()->DetachFromZipline();
	}
	else
	{
		const AZipline* AvailableZipline = GetAvailableZipline();
		if (IsValid(AvailableZipline))
		{
			//PlayAnimMontage(AvailableZipline->GetAttachAnimMontage());
			GetBaseCharacterMovementComponent()->AttachToZipline(AvailableZipline);
		}
	}
}

const ALadder* AGCBaseCharacter::GetAvailableLadder() const
{
	const ALadder* Result = nullptr;
	for (const AInteractiveActor* InteractiveActor : AvailableInteractiveActors)
	{
		if (InteractiveActor->IsA<ALadder>())
		{
			Result = StaticCast<const ALadder*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

const AZipline* AGCBaseCharacter::GetAvailableZipline() const
{
	const AZipline* Result = nullptr;
	for (const AInteractiveActor* InteractiveActor : AvailableInteractiveActors)
	{
		if (InteractiveActor->IsA<AZipline>())
		{
			Result = StaticCast<const AZipline*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

void AGCBaseCharacter::Falling()
{
	Super::Falling();
	GetBaseCharacterMovementComponent()->bNotifyApex = true;
}

void AGCBaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();
	CurrentFallApex = GetActorLocation();
}

void AGCBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	float FallHeight = (CurrentFallApex - GetActorLocation()).Z * 0.01f;
	if (IsValid(FallDamageCurve))
	{
		float DamageAmount = FallDamageCurve->GetFloatValue(FallHeight);
		TakeDamage(DamageAmount, FDamageEvent(), GetController(), Hit.GetActor());
	}
}

const UCharacterEquipmentComponent* AGCBaseCharacter::GetCharacterEquipmentComponent() const
{
	return CharacterEquipmentComponent;
}

UCharacterEquipmentComponent* AGCBaseCharacter::GetCharacterEquipmentComponent_Mutable() const
{
	return CharacterEquipmentComponent;
}

void AGCBaseCharacter::StartFire()
{
	if (CharacterEquipmentComponent->IsSelectingWeapon())
	{
		return;
	}

	if (CharacterEquipmentComponent->IsEquipping())
	{
		return;
	}
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StartFire();
	}
}

void AGCBaseCharacter::StopFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
	}
}

void AGCBaseCharacter::StartAiming()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
	{
		return;
	}
	bIsAiming = true;
	CurrentAimMovementSpeed = CurrentRangeWeapon->GetAimMovementMaxSpeed();
	CurrentRangeWeapon->StartAim();
	OnStartAiming();
}

void AGCBaseCharacter::StopAiming()
{
	if (!bIsAiming)
	{
		return;
	}

	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopAim();
	}

	bIsAiming = false;
	CurrentAimMovementSpeed = 0.0f;
	OnStopAiming();
}

FRotator AGCBaseCharacter::GetAimOffset()
{
	FVector AimDirectionWorld = GetBaseAimRotation().Vector();
	FVector AimDirectionLocal = GetTransform().InverseTransformVectorNoScale(AimDirectionWorld);
	FRotator Result = AimDirectionLocal.ToOrientationRotator();
	return Result;
}

void AGCBaseCharacter::OnStartAiming_Implementation()
{
	OnStartAimingInternal();
}

void AGCBaseCharacter::OnStopAiming_Implementation()
{
	OnStopAimingInternal();
}

bool AGCBaseCharacter::IsAiming() const
{
	return bIsAiming;
}

float AGCBaseCharacter::GetAimingMovementSpeed() const
{	
	return CurrentAimMovementSpeed;
}

void AGCBaseCharacter::NextItem()
{
	CharacterEquipmentComponent->EquipNextItem();
}

void AGCBaseCharacter::PreviousItem()
{
	CharacterEquipmentComponent->EquipPreviousItem();
}

bool AGCBaseCharacter::IsSwimmingUnderWater() const
{
	FVector HeadPosition = GetMesh()->GetSocketLocation(FName("head"));
	if (GetCharacterMovement()->IsSwimming())
	{
		APhysicsVolume* Volume = GetCharacterMovement()->GetPhysicsVolume();
		float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;
		if (VolumeTopPlane >= HeadPosition.Z)
		{
			return true;
		}
	}
	return false;
}

void AGCBaseCharacter::EquipPrimaryItem()
{
	CharacterEquipmentComponent->EquipItemInSlot(EEquipmentSlots::PrimaryItemSlot);
	//CharacterEquipmentComponent->LaunchCurrentThrowableItem();
}

void AGCBaseCharacter::PrimaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackTypes::PrimaryAttack);
	}
}

void AGCBaseCharacter::SecondaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = CharacterEquipmentComponent->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackTypes::SecondaryAttack);
	}
}

FGenericTeamId AGCBaseCharacter::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)Team);
}

void AGCBaseCharacter::Interact()
{
	if (LineOfSightObject.GetInterface())
	{
		LineOfSightObject->Interact(this);
	}
}

void AGCBaseCharacter::InitializeHealthProgress()
{
	UGCAttributeProgressBar* Widget = Cast<UGCAttributeProgressBar>(HealthBarProgressComponent->GetUserWidgetObject());
	if (!IsValid(Widget))
	{
		HealthBarProgressComponent->SetVisibility(false);
		return;
	}

	if (IsPlayerControlled() && IsLocallyControlled())
	{
		HealthBarProgressComponent->SetVisibility(false);
	}

	CharacterAttributesComponent->OnHealthChangedEvent.AddUObject(Widget, &UGCAttributeProgressBar::SetProgressPercentage);
	CharacterAttributesComponent->OnDeathEvent.AddLambda([=]() {HealthBarProgressComponent->SetVisibility(false);});
	Widget->SetProgressPercentage(CharacterAttributesComponent->GetHealthPercent());
}

bool AGCBaseCharacter::PickupItem(TWeakObjectPtr<UInventoryItem> ItemToPickup)
{
	bool Result = false;
	if (CharacterInventoryComponent->HasFreeSlot())
	{
		CharacterInventoryComponent->AddItem(ItemToPickup, 1);
		Result = true;
	}
	return Result;
}

void AGCBaseCharacter::UseInventory(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return;
	}
	if (!CharacterInventoryComponent->IsViewVisible())
	{
		CharacterInventoryComponent->OpenViewInventory(PlayerController);
		CharacterEquipmentComponent->OpenViewEquipment(PlayerController);
		PlayerController->SetInputMode(FInputModeGameAndUI{});
		PlayerController->bShowMouseCursor = true;
	}
	else
	{
		CharacterInventoryComponent->CloseViewInventory();
		CharacterEquipmentComponent->CloseViewEquipment();
		PlayerController->SetInputMode(FInputModeGameOnly{});
		PlayerController->bShowMouseCursor = false;
	}
}

void AGCBaseCharacter::ConfirmWeaponSelection()
{
	if (CharacterEquipmentComponent->IsSelectingWeapon())
	{
		CharacterEquipmentComponent->ConfirmWeaponSelection();
	}
}

//void AGCBaseCharacter::Client_ActivatePlatformTrigger_Implementation(APlatformTrigger* PlatformTrigger, bool bIsActivated)
//{
//	PlatformTrigger->SetIsActivated(bIsActivated);
//}
//
//void AGCBaseCharacter::Server_ActivatePlatformTrigger_Implementation(APlatformTrigger* PlatformTrigger, bool bIsActivated)
//{
//	PlatformTrigger->Multicast_SetIsActivated(bIsActivated);
//}

void AGCBaseCharacter::Reload()
{
	if (IsValid(CharacterEquipmentComponent->GetCurrentRangeWeapon()))
	{
		CharacterEquipmentComponent->ReloadCurrentWeapon();
	}
}

void AGCBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.AddUnique(InteractiveActor);
}

void AGCBaseCharacter::UnregisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.RemoveSingleSwap(InteractiveActor);
}

void AGCBaseCharacter::OnSprintStart_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("void AGCBaseCharacter::OnSprintStart_Implementation()"));
}

void AGCBaseCharacter::OnSprintEnd_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("void AGCBaseCharacter::OnSprintEnd_Implementation()"));
}

bool AGCBaseCharacter::CanSprint()
{
	return !(GCBaseCharacterMovementComponent->IsOutOfStamina() || GCBaseCharacterMovementComponent->IsCrouching());
}

bool AGCBaseCharacter::CanMantle() const
{
	return !GetBaseCharacterMovementComponent()->IsOnLadder() && !GetBaseCharacterMovementComponent()->IsOnZipline();
}

void AGCBaseCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime)
{
}

void AGCBaseCharacter::Prone()
{
	if (GCBaseCharacterMovementComponent)
	{
		if (CanProne())
		{
			GCBaseCharacterMovementComponent->bWantsToProne = true;
		}
	}
}

void AGCBaseCharacter::UnProne(bool bIsFullHeight)
{
	if (GCBaseCharacterMovementComponent)
	{
		GCBaseCharacterMovementComponent->bWantsToProne = false;
		GCBaseCharacterMovementComponent->bIsFullHeight = bIsFullHeight;
	}
}

bool AGCBaseCharacter::CanProne() const
{
	return !bIsProned && GCBaseCharacterMovementComponent && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AGCBaseCharacter::OnStartProne(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	if (GetMesh() && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HeightAdjust + GCBaseCharacterMovementComponent->CrouchedHalfHeight;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z + HeightAdjust + GCBaseCharacterMovementComponent->CrouchedHalfHeight;
	}
}

void AGCBaseCharacter::OnEndProne(float HeightAdjust, float ScaledHeightAdjust)
{
	RecalculateBaseEyeHeight();

	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	const float HeightDifference = GCBaseCharacterMovementComponent->bIsFullHeight ? 0.0f : GCBaseCharacterMovementComponent->CrouchedHalfHeight - GCBaseCharacterMovementComponent->PronedHalfHeight;
	if (GetMesh() && DefaultChar->GetMesh())
	{
		FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
		MeshRelativeLocation.Z = DefaultChar->GetMesh()->GetRelativeLocation().Z + HeightDifference;
		BaseTranslationOffset.Z = MeshRelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z + HeightDifference;
	}
}

void AGCBaseCharacter::OnDeath()
{
	GetCharacterMovement()->DisableMovement();
	float Duration = PlayAnimMontage(OnDeathAnimMontage);
	if (Duration == 0.0f)
	{
		EnableRagdoll();
	}
}

void AGCBaseCharacter::OnStartAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(true);
	}
}

void AGCBaseCharacter::OnStopAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(false);
	}
}

const UCharacterAttributeComponent* AGCBaseCharacter::GetCharacterAttributeComponent() const
{
	return CharacterAttributesComponent;
}

UCharacterAttributeComponent* AGCBaseCharacter::GetCharacterAttributeComponent_Mutable() const
{
	return CharacterAttributesComponent;
}

void AGCBaseCharacter::TraceLineOfSight()
{
	if (!IsPlayerControlled())
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	APlayerController* PlayerController = GetController<APlayerController>();
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector ViewDirection = ViewRotation.Vector();
	FVector	TraceEnd = ViewLocation + ViewDirection * LineOfSightDistance;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility);
	if (LineOfSightObject.GetObject() != HitResult.GetActor())
	{
		LineOfSightObject = HitResult.GetActor();

		FName ActionName;
		if (LineOfSightObject.GetInterface())
		{
			ActionName = LineOfSightObject->GetActionEventName();
		}
		else
		{
			ActionName = NAME_None;
		}
		OnInteractableObjectFound.ExecuteIfBound(ActionName);
	}
}

void AGCBaseCharacter::TryChangeSprintState(float DeltaTime)
{
	if (bIsSprintRequested && !GCBaseCharacterMovementComponent->IsSprinting() && CanSprint() && !GCBaseCharacterMovementComponent->IsProning())
	{
		GCBaseCharacterMovementComponent->StartSprint();
		OnSprintStart_Implementation();
	}

	if (GCBaseCharacterMovementComponent->IsSprinting() && !(bIsSprintRequested && CanSprint()))
	{
		GCBaseCharacterMovementComponent->StopSprint();
		OnSprintEnd();
	}
}

float AGCBaseCharacter::GetIKOffsetForASocket(const FName& SocketName)
{
	float Result = 0.0f;

	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector TraceEnd = TraceStart - IKTraceDistance * FVector::UpVector;

	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = (TraceEnd.Z - HitResult.Location.Z) / IKScale;
	}
	else if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceEnd, TraceEnd - IKTraceExtendDistance * FVector::UpVector, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = (TraceEnd.Z - HitResult.Location.Z) / IKScale;
	}

	return Result;
}

const FMantlingSettings& AGCBaseCharacter::GetMantlingSettings(float LedgeHeight) const
{
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

void AGCBaseCharacter::EnableRagdoll()
{
	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}

float AGCBaseCharacter::SignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint)
{
	if (ObjectInfo->GetTag() == SignificanceTagCharacter)
	{
		AGCBaseCharacter* Character = StaticCast<AGCBaseCharacter*>(ObjectInfo->GetObject());
		if (!IsValid(Character))
		{
			return SignificanceValueVeryHigh;
		}
		if (Character->IsPlayerControlled() && Character->IsLocallyControlled())
		{
			return SignificanceValueVeryHigh;
		}

		float DistToSquared = FVector::DistSquared(Character->GetActorLocation(), ViewPoint.GetLocation());
		if (DistToSquared <= FMath::Square(VeryHighSignificanceDistance))
		{
			return SignificanceValueVeryHigh;
		}
		else if (DistToSquared <= FMath::Square(HighSignificanceDistance))
		{
			return SignificanceValueHigh;
		}
		else if (DistToSquared <= FMath::Square(MediumSignificanceDistance))
		{
			return SignificanceValueMedium;
		}
		else if (DistToSquared <= FMath::Square(LowSignificanceDistance))
		{
			return SignificanceValueLow;
		}
		else
		{
			return SignificanceValueVeryLow;
		}
	}
	return VeryHighSignificanceDistance;
}

void AGCBaseCharacter::PostSignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal)
{
	if (OldSignificance == Significance)
	{
		return;
	}
	if (ObjectInfo->GetTag() != SignificanceTagCharacter)
	{
		return;
	}

	AGCBaseCharacter* Character = StaticCast<AGCBaseCharacter*>(ObjectInfo->GetObject());
	if (!IsValid(Character))
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	AAIController* AIController = Character->GetController<AAIController>();
	UWidgetComponent* Widget = Character->HealthBarProgressComponent;

	if (Significance == SignificanceValueVeryHigh)
	{
		MovementComponent->SetComponentTickInterval(0.0f);
		Widget->SetVisibility(true);
		Character->GetMesh()->SetComponentTickEnabled(true);
		Character->GetMesh()->SetComponentTickInterval(0.0f);
		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(0.0f);
		}
	}
	else if (Significance == SignificanceValueHigh)
	{
		MovementComponent->SetComponentTickInterval(0.0f);
		Widget->SetVisibility(true);
		Character->GetMesh()->SetComponentTickEnabled(true);
		Character->GetMesh()->SetComponentTickInterval(0.05f);
		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(0.0f);
		}
	}
	else if (Significance == SignificanceValueMedium)
	{
		MovementComponent->SetComponentTickInterval(0.1f);
		Widget->SetVisibility(false);
		Character->GetMesh()->SetComponentTickEnabled(true);
		Character->GetMesh()->SetComponentTickInterval(0.1f);
		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(0.1f);
		}
	}
	else if (Significance == SignificanceValueLow)
	{
		MovementComponent->SetComponentTickInterval(1.0f);
		Widget->SetVisibility(false);
		Character->GetMesh()->SetComponentTickEnabled(true);
		Character->GetMesh()->SetComponentTickInterval(1.0f);
		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(1.0f);
		}
	}
	else if (Significance == SignificanceValueVeryLow)
	{
		MovementComponent->SetComponentTickInterval(5.0f);
		Widget->SetVisibility(false);
		Character->GetMesh()->SetComponentTickEnabled(false);
		if (IsValid(AIController))
		{
			AIController->SetActorTickInterval(10.0f);
		}
	}
}
