// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Ladder.generated.h"

/**
 *
 */

class UAnimMontage;
class UBoxComponent;
class UStaticMeshComponent;
UCLASS()
class PROJECT_XYZ_API ALadder : public AInteractiveActor
{
	GENERATED_BODY()

public:
	ALadder();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	float GetLadderHeight() const;

	bool GetIsOnTop() const;

	UAnimMontage* GetAttachFromTopAnimMontage() const;

	FVector GetAttachFromTopAnimMontageStartingLocation() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Parameters")
	float LadderHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Parameters")
	float LadderWidth = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Parameters")
	float StepsInterval = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Parameters")
	float BottomStepOffset = 50.0f;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* RightRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* LeftRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* StepsMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* TopInteractionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Parameters")
	UAnimMontage* AttachFromTopAnimMontage;

	//Offset from ladder's top for starting anim montage
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder Parameters")
	FVector AttachFromTopAnimMontageInitialOffset = FVector::ZeroVector;

	UBoxComponent* GetLadderInteractionBox() const;

	virtual void OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

private:
 	bool bIsOnTop = false;
};
