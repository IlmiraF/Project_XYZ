// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "../Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "../Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "../Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Subsystems/Streaming/StreamingSubsystemUtils.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = 1;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	Team = ETeams::Player;
}

//void APlayerCharacter::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//	SpringArmTimeline.TickTimeline(DeltaTime);
//}

void APlayerCharacter::MoveForward(float value)
{
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(value, 1e-6f))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector ForwardVector = YawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, value);
		//AddMovementInput(GetActorForwardVector(), value);
	}
}

void APlayerCharacter::MoveRight(float value)
{
	if ((GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(value, 1e-6f))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, value);
		//AddMovementInput(GetActorRightVector(), value);
	}
}

void APlayerCharacter::Turn(float value)
{
	if (IsValid(CharacterEquipmentComponent->GetCurrentRangeWeapon()))
	{
		AddControllerYawInput(value * CharacterEquipmentComponent->GetCurrentRangeWeapon()->GetAimTurnModifier());
	}
	else
	{
		AddControllerYawInput(value);
	}
}

void APlayerCharacter::LookUp(float value)
{
	if (IsValid(CharacterEquipmentComponent->GetCurrentRangeWeapon()))
	{
		AddControllerPitchInput(value * CharacterEquipmentComponent->GetCurrentRangeWeapon()->GetAimLookUpModifier());
	}
	else
	{
		AddControllerPitchInput(value);
	}
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, HalfHeightAdjust);
}

bool APlayerCharacter::CanJumpInternal_Implementation() const
{
	return !(GCBaseCharacterMovementComponent->IsOutOfStamina()) && (bIsCrouched || Super::CanJumpInternal_Implementation()) && !GCBaseCharacterMovementComponent->IsMantling();
}


void APlayerCharacter::OnJumped_Implementation()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void APlayerCharacter::OnSprintStart_Implementation()
{
	Super::OnSprintStart_Implementation();
	SpringArmTimeline.Play();
	//SpringArmComponent->TargetArmLength = SpringArmResult;
}

void APlayerCharacter::OnSprintEnd_Implementation()
{
	Super::OnSprintEnd_Implementation();
	SpringArmTimeline.Reverse();
	//SpringArmComponent->TargetArmLength = SpringArmResult;
}

void APlayerCharacter::SwimForward(float value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(value, 1e-6f))
	{
		FRotator PitchYawRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.0f);
		FVector ForwardVector = PitchYawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, value);
	}
}

void APlayerCharacter::SwimRight(float value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(value, 1e-6f))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, value);
	}
}

void APlayerCharacter::SwimUp(float value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(value, 1e-6f))
	{
		AddMovementInput(FVector::UpVector, value);
	}
}

void APlayerCharacter::SpringArmSprintTimelineUpdate(float Alpha)
{
	UE_LOG(LogTemp, Log, TEXT("Enter"));
	SpringArmComponent->TargetArmLength = FMath::Lerp(DefaultArmLength, SpringArmLength, Alpha);
}

void APlayerCharacter::OnStartAimingInternal()
{
	Super::OnStartAimingInternal();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}
	APlayerCameraManager* CameraManager = GetController<APlayerController>()->PlayerCameraManager;
	if (IsValid(CameraManager))
	{
		ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		CameraManager->SetFOV(CurrentRangeWeapon->GetAimFOV());
	}
	//AimingFOVTimeline.Play();
}

void APlayerCharacter::OnStopAimingInternal()
{
	Super::OnStopAimingInternal();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}
	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	if (IsValid(CameraManager))
	{
		CameraManager->UnlockFOV();
	}
}

void APlayerCharacter::AimingFOVTimelineUpdate(float Alpha)
{
	APlayerCameraManager* CameraManager = GetController<APlayerController>()->PlayerCameraManager;
	const APlayerCameraManager* DefaultCameraManager = GetDefault<APlayerCameraManager>(GetClass());
	if (IsValid(CameraManager))
	{
		ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		CameraManager->SetFOV(FMath::InterpCircularIn(DefaultCameraManager->GetFOVAngle(), CurrentRangeWeapon->GetAimFOV(), Alpha));
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultArmLength = SpringArmComponent->TargetArmLength;

	if (IsValid(TimelineCurve))
	{
		FOnTimelineFloatStatic SprintTimelineUpdate;
		SprintTimelineUpdate.BindUObject(this, &APlayerCharacter::SpringArmSprintTimelineUpdate);
		SpringArmTimeline.AddInterpFloat(TimelineCurve, SprintTimelineUpdate);

		FOnTimelineFloatStatic AimingTimelineUpdate;
		AimingTimelineUpdate.BindUObject(this, &APlayerCharacter::AimingFOVTimelineUpdate);
		AimingFOVTimeline.AddInterpFloat(TimelineCurve, AimingTimelineUpdate);
	}

	UStreamingSubsystemUtils::CheckCharacterOverlapStreamingSubsystemVolume(this);
}
