// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GCBasePawnMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_XYZ_API UGCBasePawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	void JumpStart();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool IsFalling() const override;

protected:
	UPROPERTY(EditAnywhere)
	float MaxSpeed = 1200.0f;

	UPROPERTY(EditAnywhere)
	float InitialJumpVelocity = 500.0f;

	UPROPERTY(EditAnywhere)
	bool bEnableGravity;

private:
	FVector VerticalVelocity = FVector::ZeroVector;
	bool bIsFalling = false;
	
};
