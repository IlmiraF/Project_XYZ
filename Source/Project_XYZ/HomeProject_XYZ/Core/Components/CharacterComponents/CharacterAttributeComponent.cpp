
#include "CharacterAttributeComponent.h"
#include "../../Characters/GCBaseCharacter.h"
#include "../../Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "../../Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "Net/UnrealNetwork.h"

UCharacterAttributeComponent::UCharacterAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCharacterAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterAttributeComponent, Health);
}

float UCharacterAttributeComponent::GetHealthPercent() const
{
	return Health / MaxHealth;
}

float UCharacterAttributeComponent::GetStaminaPercent() const
{
	return CurrentStamina / MaxStamina;
}

float UCharacterAttributeComponent::GetOxygenPercent() const
{
	return CurrentOxygen / MaxOxygen;
}

void UCharacterAttributeComponent::AddHealth(float HealthToAdd)
{
	Health = FMath::Clamp(Health + HealthToAdd, 0.0f, MaxHealth);
	OnHealthChanged();
}

void UCharacterAttributeComponent::RestoreFullStamina()
{
	CurrentStamina = MaxStamina;
}

void UCharacterAttributeComponent::OnLevelDeserialized_Implementation()
{
	OnHealthChanged();
}

void UCharacterAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(MaxHealth > 0.0f, TEXT("UCharacterAttributeComponent::BeginPlay() max health cannot be equal to 0"));

	checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("UCharacterAttributesComponent::BeginPlay UCharacterAttributesComponent can be used only with AGCBaseCharacter"));
	CachedBaseCharacterOwner = StaticCast<AGCBaseCharacter*>(GetOwner());

	Health = MaxHealth;

	if (GetOwner()->HasAuthority())
	{
		CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributeComponent::OnTakeAnyDamage);
	}

	CurrentStamina = MaxStamina;
	CurrentOxygen = MaxOxygen;
}

bool UCharacterAttributeComponent::CanRestoreStamina()
{
	return !CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting();
}

void UCharacterAttributeComponent::OnRep_Health()
{
	OnHealthChanged();
}

void UCharacterAttributeComponent::OnHealthChanged()
{
	if (OnHealthChangedEvent.IsBound())
	{
		OnHealthChangedEvent.Broadcast(GetHealthPercent());
	}

	if (Health <= 0.0f)
	{
		//UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributeComponent::OnTakeAnyDamage character %s is killed by an actor %s"), *CachedBaseCharacterOwner->GetName(), *DamageCauser->GetName());
		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
void UCharacterAttributeComponent::DebugDrawAttributes()
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}

	FVector TextLocation = CachedBaseCharacterOwner->GetActorLocation() + (CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 5.0f)*FVector::UpVector;
	DrawDebugString(GetWorld(), TextLocation, FString::Printf(TEXT("Health : %.2f"), Health), nullptr, FColor::Green, 0.0f, true);

	if (CurrentStamina < MaxStamina)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Stamina: %.2f"), CurrentStamina));
	}

	if (CurrentOxygen < MaxOxygen)
	{
		GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Blue, FString::Printf(TEXT("Oxygen: %.2f"), CurrentOxygen));
	}
}
#endif

void UCharacterAttributeComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive()) {
		return;
	}

	UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributeComponent::OnTakeAnyDamage %s received %.2f amound of damage from %s"), *CachedBaseCharacterOwner->GetName(), Damage, *DamageCauser->GetName());
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	OnHealthChanged();
}

void UCharacterAttributeComponent::UpdateStamina(float DeltaTime)
{
	if (!CanRestoreStamina())
	{
		CurrentStamina -= SprintStaminaConsumptionVelocity * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}

	if (CanRestoreStamina())
	{
		CurrentStamina += StaminaRestoreVelocity * DeltaTime;
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, MaxStamina);
	}

	if (FMath::IsNearlyZero(CurrentStamina))
	{
		if (OutOfStaminaEvent.IsBound())
		{
			OutOfStaminaEvent.Broadcast(true);
		}
	}
	if(FMath::IsNearlyEqual(CurrentStamina, MaxStamina))
	{
		if (OutOfStaminaEvent.IsBound())
		{
			OutOfStaminaEvent.Broadcast(false);
		}
	}
}

void UCharacterAttributeComponent::UpdateOxygen(float DeltaTime)
{
	if (CachedBaseCharacterOwner->IsSwimmingUnderWater())
	{
		CurrentOxygen -= SwimOxygenConsumptionVelocity * DeltaTime;
		CurrentOxygen = FMath::Clamp(CurrentOxygen, 0.0f, MaxOxygen);
	}
	else
	{
		CurrentOxygen += SwimOxygenConsumptionVelocity * DeltaTime;
		CurrentOxygen = FMath::Clamp(CurrentOxygen, 0.0f, MaxOxygen);
	}
}

void UCharacterAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif

	UpdateStamina(DeltaTime);
	UpdateOxygen(DeltaTime);
}

