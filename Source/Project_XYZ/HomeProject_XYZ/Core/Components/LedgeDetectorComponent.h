// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LedgeDetectorComponent.generated.h"


USTRUCT(BlueprintType)
struct FLedgeDescription
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Description")
	FVector Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Description")
	FVector LedgeNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Description")
	FRotator Rotation;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_XYZ_API ULedgeDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	bool DetectLedge(OUT FLedgeDescription& LedgeDescription);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection Setting", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinimumLedgeHeight = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection Setting", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaximumLedgeHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection Setting", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ForwardCheckDistance = 100.0f;
		
private:

	TWeakObjectPtr<class ACharacter> CachedCharacterOwner;

	float NormalCapsuleRadius;
	float NormalCapsuleHalfHeight;
};
