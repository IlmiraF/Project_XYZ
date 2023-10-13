// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EquipableItem.h"
#include "../../../../../XYZ_ProjectTypes.h"
#include "MeleeWeaponItem.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FMeleeAttackDescription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack")
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float DamageAmount = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee Attack")
	class UAnimMontage* AttackMontage;
};

class UMeleeHitRegistrator;
UCLASS(Blueprintable)
class PROJECT_XYZ_API AMeleeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()

public:

	AMeleeWeaponItem();

	void StartAttack(EMeleeAttackTypes AttackType);

	void SetIsHitRegistrationEnabled(bool bIsRegistrationEnabled);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack")
	TMap<EMeleeAttackTypes, FMeleeAttackDescription> Attacks;

	virtual void BeginPlay() override;

private:

	void OnAttackTimerElapsed();
	FTimerHandle AttackTimer;
	FMeleeAttackDescription* CurrentAttack;

	TArray<UMeleeHitRegistrator*> HitRegistrators;
	TSet<AActor*> HitActors;

	UFUNCTION()
	void ProccessHit(const FHitResult& HitResult, const FVector& HitDirection);
};
