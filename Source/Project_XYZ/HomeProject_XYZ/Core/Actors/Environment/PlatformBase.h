// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "PlatformBase.generated.h"

UENUM(BlueprintType)
enum class EPlatformBehavior : uint8
{
	OnDemand = 0,
	Loop
};

class APlatformTrigger;
class UStaticMeshComponent;
UCLASS()
class PROJECT_XYZ_API APlatformBase : public AActor
{
	GENERATED_BODY()

public:
	APlatformBase();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void MovePlatform();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform movement")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Platform movement")
	APlatformTrigger* PlatformTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform movement")
	UCurveFloat* MovementCurve;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Platform movement", meta = (MakeEditWidget))
	FVector EndPlatformLocation;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Platform movement", meta = (MakeEditWidget))
	float ReturnTime = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform movement")
	EPlatformBehavior PlatformBehavior;

	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnPlatformTriggerActivated(bool bIsActivated);

	UFUNCTION()
	void TickPlatformTimeline(float Value);

	UFUNCTION()
	void OnPlatfromEndReached();
	FTimeline PlatformTimeline;

	FVector StartLocation = FVector::ZeroVector;
	FVector EndLocation = FVector::ZeroVector;
	bool bIsMovingForward = false;

	FTimerHandle ReturnTimer;
};
