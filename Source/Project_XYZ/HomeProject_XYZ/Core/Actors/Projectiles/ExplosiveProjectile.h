// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCProjectile.h"
#include "ExplosiveProjectile.generated.h"

/**
 * 
 */
class UExplosionComponent;
UCLASS()
class PROJECT_XYZ_API AExplosiveProjectile : public AGCProjectile
{
	GENERATED_BODY()

public:
	AExplosiveProjectile();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UExplosionComponent* ExplosionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float DetonationTime = 2.0f;

	virtual void OnProjectileLaunched() override;
	
private:

	void OnDetonationTimerElapsed();

	AController* GetController();

	FTimerHandle DetonationTimer;
};
