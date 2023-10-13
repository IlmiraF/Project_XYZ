// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformTrigger.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "../../Characters/GCBaseCharacter.h"

APlatformTrigger::APlatformTrigger()
{
	bReplicates = true;
	NetUpdateFrequency = 2.0f;
	MinNetUpdateFrequency = 2.0f;
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	SetRootComponent(TriggerBox);
}

void APlatformTrigger::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//FDoRepLifetimeParams RepParams;
	//RepParams.RepNotifyCondition = REPNOTIFY_Always;
	//DOREPLIFETIME_WITH_PARAMS(APlatformTrigger, bIsActivated, RepParams);
	DOREPLIFETIME(APlatformTrigger, bIsActivated);
}

void APlatformTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APlatformTrigger::OnTriggerOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &APlatformTrigger::OnTriggerOverlapEnd);
}

void APlatformTrigger::SetIsActivated(bool bIsActivated_In)
{
	if (OnTriggerActivated.IsBound())
	{
		OnTriggerActivated.Broadcast(bIsActivated_In);
	}
}

//bool APlatformTrigger::Multicast_SetIsActivated_Validate(bool bIsActivated_In)
//{
//	return bIsActivated_In;
//}

//void APlatformTrigger::Multicast_SetIsActivated_Implementation(bool bIsActivated_In)
//{
//	bIsActivated = bIsActivated_In;
//	if (OnTriggerActivated.IsBound())
//	{
//		OnTriggerActivated.Broadcast(bIsActivated_In);
//	}
//}

void APlatformTrigger::OnRep_IsActivated(bool bIsActivated_Old)
{
	SetIsActivated(bIsActivated);
}

void APlatformTrigger::OnTriggerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AGCBaseCharacter* OtherPawn = Cast<AGCBaseCharacter>(OtherActor);
	if (!IsValid(OtherPawn))
	{
		return;
	}

	if (OtherPawn->IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
	{
		OverlappedPawns.AddUnique(OtherPawn);

		if (!bIsActivated && OverlappedPawns.Num() > 0)
		{
			//OtherPawn->Server_ActivatePlatformTrigger(this, true);
			SetIsActivated(true);
			bIsActivated = true;
		}
	}
}

void APlatformTrigger::OnTriggerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AGCBaseCharacter* OtherPawn = Cast<AGCBaseCharacter>(OtherActor);
	if (!IsValid(OtherPawn))
	{
		return;
	}

	if (OtherPawn->IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
	{
		OverlappedPawns.RemoveSingleSwap(OtherPawn);

		if (bIsActivated && OverlappedPawns.Num() == 0)
		{
			//OtherPawn->Server_ActivatePlatformTrigger(this, false);
			SetIsActivated(false);
			bIsActivated = false;
		}
	}
}
