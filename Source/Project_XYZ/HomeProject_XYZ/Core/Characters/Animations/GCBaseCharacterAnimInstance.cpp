// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterAnimInstance.h"
#include "../GCBaseCharacter.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "../../Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "../../Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "../../Components/CharacterComponents/CharacterEquipmentComponent.h"

void UGCBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AGCBaseCharacter>(), TEXT("UGCBaseCharacterAnimInstance::NativeBeginPlay() UGCBaseCharacterAnimInstance can be used only with AGCBaseCharacter"));
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(TryGetPawnOwner());
}

void UGCBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!CachedBaseCharacter.IsValid())
	{
		return;
	}

	bIsAiming = CachedBaseCharacter->IsAiming();

	UGCBaseCharacterMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();

	bIsSprinting = CharacterMovement->IsSprinting();
	bIsOutOfStamina = CharacterMovement->IsOutOfStamina();
	bIsProne = CharacterMovement->IsProning();
	bIsSwimming = CharacterMovement->IsSwimming();
	bIsOnLadder = CharacterMovement->IsOnLadder();
	if (bIsOnLadder)
	{
		LadderSpeedRatio = CharacterMovement->GetLadderSpeedRatio();
	}

	bIsStrafing = !CharacterMovement->bOrientRotationToMovement;
	Direction = CalculateDirection(CharacterMovement->Velocity, CachedBaseCharacter->GetActorRotation());

	RightFootEffectorLocation = FVector(CachedBaseCharacter->GetIKRightFootOffset(), 0.0f, 0.0f);
	LeftFootEffectorLocation = FVector(CachedBaseCharacter->GetIKLeftFootOffset(), 0.0f, 0.0f);

	AimRotation = CachedBaseCharacter->GetAimOffset();

	const UCharacterEquipmentComponent* CharacterEquipment = CachedBaseCharacter->GetCharacterEquipmentComponent();
	CurrentEquippedItemType = CharacterEquipment->GetCurrentEquippedItemType();

	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipment->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		ForeGripSocketTransform = CurrentRangeWeapon->GetForeGripTransform();
	}
}
