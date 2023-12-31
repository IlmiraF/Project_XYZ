// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCBaseCharacterAnimInstance.h"
#include "FPPlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_XYZ_API UFPPlayerAnimInstance : public UGCBaseCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Animations|First Person")
	float PlayerCameraPitchAngle = 0.0f;

	TWeakObjectPtr<class AFPPlayerCharacter> CachedFirstPersonCharacterOwner;

private:
	float CalculateCameraPitchAngle() const;
};
