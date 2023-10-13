// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "../GameCodeBasePawn.h"
#include "../../Components/MovementComponents/GCBasePawnMovementComponent.h"
#include "GCBasePawnAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_XYZ_API UGCBasePawnAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Base Pawn Animation Instance")
	float InputForward;
	
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Base Pawn Animation Instance")
	float InputRight;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Base Pawn Animation Instance")
	bool bIsInAir;

private:

	TWeakObjectPtr<class AGameCodeBasePawn> CachedBasePawn;
};
