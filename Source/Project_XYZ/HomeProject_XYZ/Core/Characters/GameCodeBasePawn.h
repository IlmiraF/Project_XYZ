// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameCodeBasePawn.generated.h"

UCLASS()
class PROJECT_XYZ_API AGameCodeBasePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGameCodeBasePawn();

	UPROPERTY(VisibleAnywhere)
	class UPawnMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComponent;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float value);
	void MoveRight(float value);
	void Jump();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetInputForward() { return InputForward; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetInputRight() { return InputRight; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base Pawn")
	float CollisionSphereRadius = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Pawn")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Pawn")
	class UCameraComponent* CameraComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Base Pawn")
	class UArrowComponent* ArrowComponent;
#endif

private:
	UFUNCTION()
	void OnBlendComplete();

	float InputForward = 0.0f;
	float InputRight = 0.0f;

	class AActor* CurrentViewActor;
};
