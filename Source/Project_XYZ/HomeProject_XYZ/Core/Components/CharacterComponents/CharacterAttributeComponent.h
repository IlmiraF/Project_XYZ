// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "CharacterAttributeComponent.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnDeathEventSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOutOfStaminaEventSignature, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_XYZ_API UCharacterAttributeComponent : public UActorComponent, public ISaveSubsystemInterface
{
	GENERATED_BODY()

public:	
	UCharacterAttributeComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FOnDeathEventSignature OnDeathEvent;
	FOutOfStaminaEventSignature OutOfStaminaEvent;
	FOnHealthChanged OnHealthChangedEvent;

	bool IsAlive() { return Health > 0.0f; }

	float GetHealthPercent() const;

	float GetStaminaPercent() const;

	float GetOxygenPercent() const;

	void AddHealth(float HealthToAdd);

	void RestoreFullStamina();

	virtual void OnLevelDeserialized_Implementation() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.0f))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Stamina")
	float StaminaRestoreVelocity = 25.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Stamina")
	float SprintStaminaConsumptionVelocity = 20.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Oxygen")
	float MaxOxygen = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Oxygen")
	float OxygenRestoreVelocity = 15.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Oxygen")
	float SwimOxygenConsumptionVelocity = 2.0f;

	bool CanRestoreStamina();

private:
	UPROPERTY(ReplicatedUsing=OnRep_Health, SaveGame)
	float Health = 0.0f;

	UFUNCTION()
	void OnRep_Health();

	void OnHealthChanged();

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes();
#endif

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	TWeakObjectPtr<class AGCBaseCharacter> CachedBaseCharacterOwner;

	void UpdateStamina(float DeltaTime);
	float CurrentStamina = 0.0f;

	void UpdateOxygen(float DeltaTime);
	float CurrentOxygen;
};
