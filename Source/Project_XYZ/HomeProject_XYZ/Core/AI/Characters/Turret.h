// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "Turret.generated.h"

UENUM(BlueprintType)
enum class ETurretState : uint8
{
	Searching,
	Firing
};

class UWeaponBarellComponent;
class UTurretAttributeComponent;
UCLASS()
class PROJECT_XYZ_API ATurret : public APawn
{
	GENERATED_BODY()

public:
	ATurret();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void Tick(float DeltaTime) override;

	void OnCurrentTargetSet();

	virtual FVector GetPawnViewLocation() const override;

	virtual FRotator GetViewRotation() const override;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentTarget)
	AActor* CurrentTarget = nullptr;


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBaseComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TurretBarellComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponBarellComponent* WeaponBarell;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTurretAttributeComponent* AttributeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float BaseSearchingRotationRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float BarellPitchRotationRate = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxBarellPitchAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinBarellPitchAngle = -30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters")
	float BaseFiringInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters|Fire", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float RateOfFire = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters|Fire")
	float BulletSpreadAngle = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters|Fire")
	float FireDelayTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Turret Parameters|Team")
	ETeams Team = ETeams::Enemy;

	void OnDeath();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion|VFX")
	UParticleSystem* ExplosionVFX;

private:

	void SearchingMovement(float DeltaTime);
	void FiringMovement(float DeltaTime);

	void SetCurrentTurretState(ETurretState NewState);
	ETurretState CurrentTurretState = ETurretState::Searching;

	UFUNCTION()
	void OnRep_CurrentTarget();

	float GetFireInterval() const;

	FTimerHandle ShotTimer;

	void MakeShot();
};
