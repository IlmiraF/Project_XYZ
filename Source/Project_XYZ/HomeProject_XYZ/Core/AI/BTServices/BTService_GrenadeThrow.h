// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_GrenadeThrow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_XYZ_API UBTService_GrenadeThrow : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_GrenadeThrow();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float MaxFireDistance = 800.0f;
};
