// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeHitRegistrator.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "../../Utils/Project_XYZ_TraceUtils.h"
#include "Kismet/GameplayStatics.h"
#include "../../Subsystems/DebugSubsystem.h"

UMeleeHitRegistrator::UMeleeHitRegistrator()
{
	PrimaryComponentTick.bCanEverTick = true;
	SphereRadius = 5.0f;
	SetCollisionProfileName(CollisionProfileNoCollision);
}

void UMeleeHitRegistrator::ProccessHitRegistration()
{
	FVector CurrentLocation = GetComponentLocation();
	FHitResult HitResult;

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryMeleeWeapon);
#else
	bool bIsDebugEnabled = false;
#endif

	bool bHasHit = Project_XYZ_TraceUtils::SweepSphereSingleByChannel(GetWorld(), HitResult, PreviousComponentLocation, CurrentLocation, GetScaledSphereRadius(), ECC_Melee, FCollisionQueryParams::DefaultQueryParam, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, 5.0f);
	if (bHasHit)
	{
		FVector Direction = (CurrentLocation - PreviousComponentLocation).GetSafeNormal();
		if (OnMeleeHitRegistred.IsBound())
		{
			OnMeleeHitRegistred.Broadcast(HitResult, Direction);
		}
	}
}

void UMeleeHitRegistrator::SetIsHitRegistrationEnabled(bool bIsEnabled_In)
{
	bIsHitRegistrationEnabled = bIsEnabled_In;
}

void UMeleeHitRegistrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsHitRegistrationEnabled)
	{
		ProccessHitRegistration();
	}
	PreviousComponentLocation = GetComponentLocation();
}
