// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCBaseCharacter.h"
#include "Components/TimelineComponent.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECT_XYZ_API APlayerCharacter : public AGCBaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	//virtual void Tick(float DeltaTime) override;

	virtual void MoveForward(float value) override;
	virtual void MoveRight(float value) override;
	virtual void Turn(float value) override;
	virtual void LookUp(float value) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;

	virtual void OnSprintStart_Implementation() override;
	virtual void OnSprintEnd_Implementation() override;

	virtual void SwimForward(float value) override;
	virtual void SwimRight(float value) override;
	virtual void SwimUp(float value) override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera")
	class USpringArmComponent* SpringArmComponent;

	float DefaultArmLength = 0.0f;
	float SpringArmLength = 400.0f;

	FTimeline SpringArmTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* TimelineCurve;

	void SpringArmSprintTimelineUpdate(float Alpha);

	virtual void OnStartAimingInternal() override;
	virtual void OnStopAimingInternal() override;

	FTimeline AimingFOVTimeline;
	void AimingFOVTimelineUpdate(float Alpha);
};
