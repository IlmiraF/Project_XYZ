// Fill out your copyright notice in the Description page of Project Settings.


#include "TurretAttributeComponent.h"
#include "../Characters/Turret.h"
#include "../Controllers/AITurretController.h"

void UTurretAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ATurret>(), TEXT("UTurretAttributeComponent::BeginPlay() UTurretAttributeComponent can be used only with ATurret"));
	CachedTurret = StaticCast<ATurret*>(GetOwner());

	Health = MaxHealth;

	CachedTurret->OnTakeAnyDamage.AddDynamic(this, &UTurretAttributeComponent::OnTakeAnyDamage);
	
}

void UTurretAttributeComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("Shot")));
	if (!(Health > 0.0f)) {
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	if (Health <= 0.0f)
	{
		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
	AAITurretController* TurretContoller = Cast<AAITurretController>(CachedTurret->GetController());
	TurretContoller->OnReportDamageEvent(GetWorld(), DamagedActor, DamageCauser, Damage, DamageCauser->GetActorLocation(), DamagedActor->GetActorLocation());
}

