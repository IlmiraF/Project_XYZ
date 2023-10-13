// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformBase.h"
#include "Components/StaticMeshComponent.h"
#include "PlatformTrigger.h"

APlatformBase::APlatformBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	SetRootComponent(PlatformMesh);
}

void APlatformBase::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(MovementCurve))
	{
		FOnTimelineFloatStatic OnTimelineUpdate;
		OnTimelineUpdate.BindUFunction(this, FName("TickPlatformTimeline"));
		PlatformTimeline.AddInterpFloat(MovementCurve, OnTimelineUpdate);

		FOnTimelineEventStatic OnTimelineFinished;
		OnTimelineFinished.BindUFunction(this, FName("OnPlatfromEndReached"));
		PlatformTimeline.SetTimelineFinishedFunc(OnTimelineFinished);
	}

	if (PlatformBehavior == EPlatformBehavior::Loop)
	{
		MovePlatform();
	}

	StartLocation = GetActorLocation();
	EndLocation = GetActorTransform().TransformPosition(EndPlatformLocation);

	if (IsValid(PlatformTrigger))
	{
		PlatformTrigger->OnTriggerActivated.AddDynamic(this, &APlatformBase::OnPlatformTriggerActivated);
	}
}

void APlatformBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PlatformTimeline.TickTimeline(DeltaTime);
}

void APlatformBase::MovePlatform()
{
	if (bIsMovingForward)
	{
		PlatformTimeline.Reverse();
		bIsMovingForward = false;
	}
	else
	{
		PlatformTimeline.Play();
		bIsMovingForward = true;
	}
}

void APlatformBase::OnPlatformTriggerActivated(bool bIsActivated)
{
	MovePlatform();
}

void APlatformBase::TickPlatformTimeline(float Value)
{
	FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, Value);
	SetActorLocation(NewLocation);
}

void APlatformBase::OnPlatfromEndReached()
{
	if (PlatformBehavior == EPlatformBehavior::Loop)
	{
		MovePlatform();
	}
	else if (ReturnTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ReturnTimer, this, &APlatformBase::MovePlatform, ReturnTime, false);
		return;
	}
}