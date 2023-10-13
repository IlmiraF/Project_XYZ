// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_GrenadeThrow.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Characters/GCBaseCharacter.h"
#include "../../Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "../../Actors/Equipment/Throwables/ThrowableItem.h"
#include "../../../../XYZ_ProjectTypes.h"

UBTService_GrenadeThrow::UBTService_GrenadeThrow()
{
	NodeName = "GrenadeThrow";
}

void UBTService_GrenadeThrow::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!IsValid(AIController) || !IsValid(Blackboard))
	{
		return;
	}

	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(AIController->GetPawn());
	if (!IsValid(Character))
	{
		return;
	}

	UCharacterEquipmentComponent* EquipmentComponent = Character->GetCharacterEquipmentComponent_Mutable();

	AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!IsValid(CurrentTarget))
	{
		return;
	}

	float DistSq = FVector::DistSquared(CurrentTarget->GetActorLocation(), Character->GetActorLocation());
	if (DistSq < FMath::Square(MaxFireDistance))
	{
		EquipmentComponent->EquipItemInSlot(EEquipmentSlots::PrimaryItemSlot);
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Throw")));
		return;
	}
}
