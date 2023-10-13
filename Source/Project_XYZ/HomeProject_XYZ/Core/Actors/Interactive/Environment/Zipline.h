// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Zipline.generated.h"

/**
 * 
 */

class UCapsuleComponent;
class UStaticMeshComponent;
class UAnimMontage;
UCLASS()
class PROJECT_XYZ_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AZipline();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UAnimMontage* GetAttachAnimMontage() const;
	FVector GetAttachAnimMontageStartingLocation() const;

	float GetZiplineHeight() const;
	float GetZiplineWidth() const;

	UStaticMeshComponent* GetCableMeshComponent() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters")
	float ZiplineHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters")
	float ZiplineWidth = 300.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* RightRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* LeftRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* CableMeshComponent;

	UCapsuleComponent* GetZiplineInteractionCapsule() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters")
	UAnimMontage* AttachAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline Parameters")
	FVector AttachAnimMontageInitialOffset = FVector::ZeroVector;

private:
	float Angle = 0.0f;
	float Hypotenuse = 0.0f;
};
