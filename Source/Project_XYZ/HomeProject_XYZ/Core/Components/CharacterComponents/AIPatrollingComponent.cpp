// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPatrollingComponent.h"
#include "../../Actors/Navigation/PatrollingPath.h"

bool UAIPatrollingComponent::CanPatrol() const
{
	return IsValid(PatrollingPath) && PatrollingPath->GetWaypoints().Num() > 0;
}

FVector UAIPatrollingComponent::SelectClosestWaypoint()
{
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	const TArray<FVector> WayPoints = PatrollingPath->GetWaypoints();
	FTransform PathTransform = PatrollingPath->GetActorTransform();

	FVector ClosestWayPoint;
	float MinSquaredDistance = FLT_MAX;
	for (int32 i = 0; i < WayPoints.Num(); ++i)
	{
		FVector WayPointWorld = PathTransform.TransformPosition(WayPoints[i]);
		float CurrentSquaredDistance = (OwnerLocation - WayPointWorld).SizeSquared();
		if (CurrentSquaredDistance < MinSquaredDistance)
		{
			MinSquaredDistance = CurrentSquaredDistance;
			ClosestWayPoint = WayPointWorld;
			CurrentWayPointIndex = i;
		}
	}
	return ClosestWayPoint;
}

FVector UAIPatrollingComponent::SelectNextWaypoint()
{
	switch (PatrollingMethod)
	{
		case EPatrollingPathMethod::Circle:
		{
			return PatrollingCircleMethod();
			break;
		}
		case EPatrollingPathMethod::PingPong:
		{
			return PatrollingPingPongMethod();
			break;
		}
		default:
		{
			return PatrollingCircleMethod();
			break;
		}
	}
}

FVector UAIPatrollingComponent::PatrollingCircleMethod()
{
	++CurrentWayPointIndex;
	const TArray<FVector> WayPoints = PatrollingPath->GetWaypoints();
	if (CurrentWayPointIndex == WayPoints.Num())
	{
		CurrentWayPointIndex = 0;
	}

	FTransform PathTransform = PatrollingPath->GetActorTransform();
	FVector WayPoint = PathTransform.TransformPosition(WayPoints[CurrentWayPointIndex]);
	return WayPoint;
}

FVector UAIPatrollingComponent::PatrollingPingPongMethod()
{
	const TArray<FVector> WayPoints = PatrollingPath->GetWaypoints();

	if (bIsForwardPatrol)
	{
		++CurrentWayPointIndex;
		if (CurrentWayPointIndex == WayPoints.Num() - 1)
		{
			bIsForwardPatrol = false;
		}
	}
	else
	{
		--CurrentWayPointIndex;
		if (CurrentWayPointIndex == 0)
		{
			bIsForwardPatrol = true;
		}
	}

	FTransform PathTransform = PatrollingPath->GetActorTransform();
	FVector WayPoint = PathTransform.TransformPosition(WayPoints[CurrentWayPointIndex]);
	return WayPoint;
}
