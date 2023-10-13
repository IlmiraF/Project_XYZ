// Fill out your copyright notice in the Description page of Project Settings.


#include "AITurretController.h"
#include "../Characters/Turret.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameFramework/Actor.h"

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("AAITurretController::SetPawn AAITurretController can possess only turrets"));
		CachedTurret = StaticCast<ATurret*>(InPawn);
	}
	else
	{
		CachedTurret = nullptr;
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);
	if (!CachedTurret.IsValid())
	{
		return;
	}

	AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	if (IsValid(ClosestActor))
	{
		CachedTurret->CurrentTarget = ClosestActor;
		CachedTurret->OnCurrentTargetSet();
	}

	//CachedTurret->SetCurrentTarget(ClosestActor);
}

void AAITurretController::OnReportDamageEvent(UObject* WorldContextObject, AActor* DamagedActor, AActor* InstigatorBy, float DamageAmount, FVector EventLocation, FVector HitLocation)
{
	UAISense_Damage::ReportDamageEvent(WorldContextObject, DamagedActor, InstigatorBy, DamageAmount, EventLocation, HitLocation);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, InstigatorBy->GetName());
	CachedTurret->CurrentTarget = InstigatorBy;
	CachedTurret->OnCurrentTargetSet();
}
