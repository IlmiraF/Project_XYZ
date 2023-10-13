// Fill out your copyright notice in the Description page of Project Settings.


#include "PickablePowerups.h"
#include "../../../../../XYZ_ProjectTypes.h"
#include "../../../Utils/GCDataTableUtils.h"
#include "../../../Inventory/Items/InventoryItem.h"
#include "../../../Characters/GCBaseCharacter.h"

APickablePowerups::APickablePowerups()
{
	PowerupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerupMesh"));
	SetRootComponent(PowerupMesh);
}

void APickablePowerups::Interact(AGCBaseCharacter* Character)
{
	FItemTableRow* ItemData = GCDataTableUtils::FindInventoryItemData(DataTableID());

	if (ItemData == nullptr)
	{
		return;
	}

	TWeakObjectPtr<UInventoryItem> Item = TWeakObjectPtr<UInventoryItem>(NewObject<UInventoryItem>(Character, ItemData->InventoryItemClass));
	Item->Initialize(DataTableIDName, ItemData->InventoryItemDescription);

	const bool bPickedUp = Character->PickupItem(Item);
	if (bPickedUp)
	{
		Destroy();
	}
}

FName APickablePowerups::GetActionEventName() const
{
	return ActionInteract;
}
