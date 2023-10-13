// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "../../Characters/GCBaseCharacter.h"
#include "../../Actors/Interactive/Environment/Ladder.h"
#include "../../Actors/Interactive/Environment/Zipline.h"

FNetworkPredictionData_Client* UGCBaseCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UGCBaseCharacterMovementComponent* MutableThis = const_cast<UGCBaseCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_GC(*this);
	}
	return ClientPredictionData;
}

void UGCBaseCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bool bWasMantling = GetBaseCharacterOwner()->bIsMantling;
	bIsSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bool bIsMantling = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;

	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_Authority)
	{
		if (!bWasMantling && bIsMantling)
		{
			GetBaseCharacterOwner()->Mantle(true);
		}
	}

}

float UGCBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();
	if (bIsSprinting)
	{
		Result = SprintSpeed;
	}
	else if (bIsOutOfStamina)
	{
		Result = OutOfStaminaSpeed;
	}
	else if (IsOnLadder())
	{
		Result = ClimbingOnLadderMaxSpeed;
	}
	else if (IsProning())
	{
		Result = MaxProneSpeed;
	}
	else if (GetBaseCharacterOwner()->IsAiming())
	{
		Result = GetBaseCharacterOwner()->GetAimingMovementSpeed();
	}
	else if (IsOnZipline())
	{
		Result = ZiplineMovementSpeed;
	}
	return Result;
}

void UGCBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UGCBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

bool UGCBaseCharacterMovementComponent::IsProning() const
{
	return GetBaseCharacterOwner() && GetBaseCharacterOwner()->bIsProned;
}

void UGCBaseCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
	const bool bIsProning = IsProning();
	if (bIsProning && (!bWantsToProne || !CanProneInCurrentState()))
	{
		UnProne();
	}
	else if (!bIsProning && bWantsToProne && CanProneInCurrentState())
	{
		Prone();
	}
}

bool UGCBaseCharacterMovementComponent::CanProneInCurrentState() const
{
	return IsMovingOnGround() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

void UGCBaseCharacterMovementComponent::Prone()
{
	if (!HasValidData())
	{
		return;
	}

	if (!CanProneInCurrentState())
	{
		return;
	}

	if (GetBaseCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == PronedHalfHeight)
	{
		GetBaseCharacterOwner()->bIsProned = true;
		bWantsToCrouch = false;
		GetBaseCharacterOwner()->bIsCrouched = false;
		GetBaseCharacterOwner()->OnStartProne(0.f, 0.f);
		return;
	}

	// Change collision size to crouching dimensions
	const float ComponentScale = GetBaseCharacterOwner()->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = GetBaseCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = GetBaseCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedPronedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, PronedHalfHeight);
	GetBaseCharacterOwner()->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedPronedHalfHeight);
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedPronedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	// Crouching to a larger height? (this is rare)
	if (ClampedPronedHalfHeight > OldUnscaledHalfHeight)
	{
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, GetBaseCharacterOwner());
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);
		const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, ScaledHalfHeightAdjust), FQuat::Identity,
			UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

		// If encroached, cancel
		if (bEncroached)
		{
			GetBaseCharacterOwner()->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, OldUnscaledHalfHeight);
			return;
		}
	}

	if (bProneMaintainsBaseLocation)
	{
		// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
		UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	}

	GetBaseCharacterOwner()->bIsProned = true;
	bWantsToCrouch = false;
	GetBaseCharacterOwner()->bIsCrouched = false;

	bForceNextFloorCheck = true;

	// OnStartCrouch takes the change from the Default size, not the current one (though they are usually the same).
	//const float MeshAdjust = ScaledHalfHeightAdjust;
	ACharacter* DefaultCharacter = GetBaseCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - CrouchedHalfHeight - ClampedPronedHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	GetBaseCharacterOwner()->OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void UGCBaseCharacterMovementComponent::UnProne()
{
	if (!HasValidData())
	{
		return;
	}

	ACharacter* DefaultCharacter = GetBaseCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>();
	float DesiredHeight = bIsFullHeight ? DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() : CrouchedHalfHeight;

	if (GetBaseCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DesiredHeight)
	{
		GetBaseCharacterOwner()->bIsProned = false;
		bWantsToCrouch = !bIsFullHeight;
		GetBaseCharacterOwner()->bIsCrouched = !bIsFullHeight;
		GetBaseCharacterOwner()->OnEndProne(0.f, 0.f);
		return;
	}

	const float CurrentPronedHalfHeight = GetBaseCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = GetBaseCharacterOwner()->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = GetCharacterOwner()->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	float HalfHeightAdjust = DesiredHeight - OldUnscaledHalfHeight;
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	check(GetBaseCharacterOwner()->GetCapsuleComponent());

	const UWorld* MyWorld = GetWorld();
	const float SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, GetBaseCharacterOwner());
	FCollisionResponseParams ResponseParams;
	InitCollisionParams(CapsuleParams, ResponseParams);

	const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
	bool bEncroached = true;

	if (!bProneMaintainsBaseLocation)
	{
		bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParams);
		if (bEncroached)
		{
			if (ScaledHalfHeightAdjust > 0.f)
			{
				float PawnRadius, PawnHalfHeight;
				GetBaseCharacterOwner()->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
				const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
				const float	TraceDist = PawnHalfHeight - ShrinkHalfHeight;
				const FVector Down = FVector(0.f, 0.f, -TraceDist);

				FHitResult Hit(1.f);
				const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
				const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, FQuat::Identity, CollisionChannel, ShortCapsuleShape);
				if (Hit.bStartPenetrating)
				{
					bEncroached = true;
				}
				else
				{
					const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
					const FVector NewLoc = FVector(PawnLocation.X, PawnLocation.Y, PawnLocation.Z - DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight);
					bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParams);
					if (!bEncroached)
					{
						UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags);
					}
				}
			}
		}
	}
	else
	{
		FVector StandingLocation = PawnLocation + FVector(0.f, 0.f, StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentPronedHalfHeight);
		bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParams);
			
		if (bEncroached)
		{
			if (IsMovingOnGround())
			{
				const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
				if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
				{
					StandingLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
					bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParams);
				}
			}
		}

		if (!bEncroached)
		{
			UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags);
			bForceNextFloorCheck = true;
		}
	}

	if (bEncroached)
	{
		return;
	}

	GetBaseCharacterOwner()->bIsProned = false;
	bWantsToCrouch = !bIsFullHeight;
	GetBaseCharacterOwner()->bIsCrouched = !bIsFullHeight;

	GetBaseCharacterOwner()->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DesiredHeight, true);

	HalfHeightAdjust = bIsFullHeight ? CrouchedHalfHeight - PronedHalfHeight : DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - DesiredHeight - PronedHalfHeight;
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	GetBaseCharacterOwner()->OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);
}



void UGCBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(EMovementMode::MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);

}

void UGCBaseCharacterMovementComponent::EndMantle()
{
	GetBaseCharacterOwner()->bIsMantling = false;
	SetMovementMode(MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling;
}

void UGCBaseCharacterMovementComponent::AttachToLadder(const ALadder* Ladder)
{
	CurrentLadder = Ladder;

	FRotator TargetOrientationRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw += 180.0f;

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderForwardVector = CurrentLadder->GetActorForwardVector();
	float Projection = GetActorToCurrentLadderProjection(GetActorLocation());

	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + Projection * LadderUpVector + LadderToCharacterOffset * LadderForwardVector;
	if (CurrentLadder->GetIsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachFromTopAnimMontageStartingLocation();
	}

	GetOwner()->SetActorLocation(NewCharacterLocation);

	GetOwner()->SetActorRotation(TargetOrientationRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

void UGCBaseCharacterMovementComponent::DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod)
{
	switch (DetachFromLadderMethod)
	{
	case EDetachFromLadderMethod::ReachingTheTop:
	{
		GetBaseCharacterOwner()->Mantle(true);
		break;
	}
	case EDetachFromLadderMethod::ReachingTheBottom:
	{
		SetMovementMode(MOVE_Walking);
		break;
	}
	case EDetachFromLadderMethod::JumpOff:
	{
		FVector JumpDirection = CurrentLadder->GetActorForwardVector();
		SetMovementMode(MOVE_Falling);
		FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;
		ForceTargetRotation = JumpDirection.ToOrientationRotator();
		bForceRotation = true;
		Launch(JumpVelocity);
		break;
	}
	case EDetachFromLadderMethod::Fall:
	default:
	{
		SetMovementMode(MOVE_Falling);
		break;
	}
	}
}

bool UGCBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder;
}

const ALadder* UGCBaseCharacterMovementComponent::GetCurrentLadder()
{
	return CurrentLadder;
}

void UGCBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();
		CurrentRotation.DiagnosticCheckNaN(TEXT("UGCBaseCharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("UGCBaseCharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;
			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
		}
		else
		{
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
		return;
	}

	if (IsOnLadder())
	{
		return;
	}
	Super::PhysicsRotation(DeltaTime);

}

float UGCBaseCharacterMovementComponent::GetActorToCurrentLadderProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentLadder), TEXT("float UGCBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection() cannot be invoked when current ladder is null"))
	
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderToCharacterDistance = Location - CurrentLadder->GetActorLocation();
	float ActorToLadderProjection = FVector::DotProduct(LadderUpVector, LadderToCharacterDistance);
	
	return ActorToLadderProjection;
}

float UGCBaseCharacterMovementComponent::GetLadderSpeedRatio() const
{
	checkf(IsValid(CurrentLadder), TEXT("float UGCBaseCharacterMovementComponent::GetLadderSpeedRatio() cannot be invoked when current ladder is null"))
	
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	return FVector::DotProduct(LadderUpVector, Velocity) / ClimbingOnLadderMaxSpeed;
}

void UGCBaseCharacterMovementComponent::AttachToZipline(const AZipline* Zipline)
{
	CurrentZipline = Zipline;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("Enter")));

	FRotator TargetOrientationRotation = CurrentZipline->GetActorRightVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw += 180.0f;

	GetOwner()->SetActorRotation(TargetOrientationRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);

	UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(CurrentZipline->GetAttachAnimMontage(), 1.0f / (CurrentZipline->GetZiplineWidth() / ZiplineMovementSpeed), EMontagePlayReturnType::Duration, 0.0f);

}

void UGCBaseCharacterMovementComponent::DetachFromZipline()
{
	SetMovementMode(MOVE_Falling);
	UAnimInstance* AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Stop(0.0f);
}

bool UGCBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Zipline;
}

const AZipline* UGCBaseCharacterMovementComponent::GetCurrentZipline()
{
	return CurrentZipline;
}

void UGCBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (MovementMode == MOVE_Walking)
	{
		bProneMaintainsBaseLocation = true;
	}
	else 
	{
		bProneMaintainsBaseLocation = false;
	}

	if (MovementMode == MOVE_Swimming)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
	{
		CurrentLadder = nullptr;
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
	{
		CurrentZipline = nullptr;
	}

	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
			case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{
				GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UGCBaseCharacterMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
				break;
			}
			case (uint8)ECustomMovementMode::CMOVE_Zipline:
			{
				GetWorld()->GetTimerManager().SetTimer(ZiplineTimer, this, &UGCBaseCharacterMovementComponent::DetachFromZipline, CurrentZipline->GetZiplineWidth() / ZiplineMovementSpeed, false);
				break;
			}
			default:
				break;
		}
	}
}

void UGCBaseCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}

	switch (CustomMovementMode)
	{
		case (uint8) ECustomMovementMode::CMOVE_Mantling:
		{
			PhysMantling(DeltaTime, Iterations);
			break;
		}
		case (uint8) ECustomMovementMode::CMOVE_Ladder:
		{
			PhysLadder(DeltaTime, Iterations);
			break;
		}
		case (uint8) ECustomMovementMode::CMOVE_Zipline:
		{
			PhysZipline(DeltaTime, Iterations);
			break;
		}
		default:
			break;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UGCBaseCharacterMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;

	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);

	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectionAlpha = MantlingCurveValue.Z;

	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);


	FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CurrentMantlingParameters.TargetLocation, PositionAlpha);
	FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);

	FVector Delta = NewLocation - GetActorLocation();
	Velocity = Delta / DeltaTime;

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, NewRotation, false, Hit);
}

void UGCBaseCharacterMovementComponent::PhysLadder(float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, 1.0f, false, ClimbingOnLadderBrakingDecelaration);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}

	FVector NewPos = GetActorLocation() + Delta;
	float NewPosProjection = GetActorToCurrentLadderProjection(NewPos);

	if (NewPosProjection < MinLadderBottomOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		return;
	}
	else if(NewPosProjection > (CurrentLadder->GetLadderHeight() - MaxLadderTopOffset))
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UGCBaseCharacterMovementComponent::PhysZipline(float DeltaTime, int32 Iterations)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(ZiplineTimer);

	FVector NewLocation = FMath::Lerp(GetActorLocation(), FVector(CurrentZipline->GetActorLocation().X, CurrentZipline->GetActorLocation().Y - CurrentZipline->GetZiplineWidth() * 0.5f + 100.0f, CurrentZipline->GetActorLocation().Z + CurrentZipline->GetZiplineHeight() - 80.0f), ElapsedTime);

	FVector Delta = NewLocation - GetActorLocation();

	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
}

AGCBaseCharacter* UGCBaseCharacterMovementComponent::GetBaseCharacterOwner() const
{
	return StaticCast<AGCBaseCharacter*>(CharacterOwner);
}

void FSavedMove_GC::Clear()
{
	Super::Clear();
	bSavedIsSprinting = 0;
	bSavedIsMantling = 0;
}

uint8 FSavedMove_GC::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedIsSprinting)
	{
		Result |= FLAG_Custom_0;
	}
	if (bSavedIsMantling)
	{
		Result &= ~FLAG_JumpPressed;
		Result |= FLAG_Custom_1;
	}
	return Result;
}

bool FSavedMove_GC::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_GC* NewMove = StaticCast<const FSavedMove_GC*>(NewMovePtr.Get());
	if (bSavedIsSprinting != NewMove->bSavedIsSprinting
		|| bSavedIsMantling != NewMove->bSavedIsMantling)
	{
		return false;
	}
	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void FSavedMove_GC::SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(InCharacter, InDeltaTime, NewAccel, ClientData);
	check(InCharacter->IsA<AGCBaseCharacter>());
	AGCBaseCharacter* InBaseCharacter = StaticCast<AGCBaseCharacter*>(InCharacter);
	UGCBaseCharacterMovementComponent* MovementComponent = InBaseCharacter->GetBaseCharacterMovementComponent();
	bSavedIsSprinting = MovementComponent->bIsSprinting;
	bSavedIsMantling = InBaseCharacter->bIsMantling;
}

void FSavedMove_GC::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UGCBaseCharacterMovementComponent* MovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(Character->GetMovementComponent());
	MovementComponent->bIsSprinting = bSavedIsSprinting;
}

FNetworkPredictionData_Client_Character_GC::FNetworkPredictionData_Client_Character_GC(const UCharacterMovementComponent& ClientMovement)
	:Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_Character_GC::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_GC());
}
