// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIPatrollingComponent.generated.h"


UENUM(BlueprintType)
enum class EPatrollingPathMethod : uint8
{
	Circle,
	PingPong
};

class APatrollingPath;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_XYZ_API UAIPatrollingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	bool CanPatrol() const;

	FVector SelectClosestWaypoint();
	FVector SelectNextWaypoint();

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Path")
	APatrollingPath* PatrollingPath;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Path")
	EPatrollingPathMethod PatrollingMethod = EPatrollingPathMethod::Circle;

private:
	int32 CurrentWayPointIndex = -1;

	FVector PatrollingCircleMethod();
	FVector PatrollingPingPongMethod();

	bool bIsForwardPatrol = true;

};
