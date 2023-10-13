// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCAIController.h"
#include "AITurretController.generated.h"

/**
 * 
 */
class ATurret;
UCLASS()
class PROJECT_XYZ_API AAITurretController : public AGCAIController
{
	GENERATED_BODY()

public:

	virtual void SetPawn(APawn* InPawn) override;

	virtual void ActorsPerceptionUpdated(const TArray<AActor *>& UpdatedActors) override;

	void OnReportDamageEvent(UObject* WorldContextObject, AActor* DamagedActor, AActor* Instigator, float DamageAmount, FVector EventLocation, FVector HitLocation);

private:

	TWeakObjectPtr<ATurret> CachedTurret;
	
};
