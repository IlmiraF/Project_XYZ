// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryItem.h"
#include "../../Actors/Interactive/Pickable/PickableItem.h"
#include "../../Actors/Equipment/EquipableItem.h"
#include "../../Characters/GCBaseCharacter.h"

void UInventoryItem::Initialize(FName DataTableID_In, const FInventoryItemDescription& Description_In)
{
	DataTableIDName = DataTableID_In;
	Description.Icon = Description_In.Icon;
	Description.Name = Description_In.Name;
	bIsInitialized = true;
}

FName UInventoryItem::GetDataTableID() const
{
	return DataTableIDName;
}

const FInventoryItemDescription& UInventoryItem::GetDescription() const
{
	return Description;
}

bool UInventoryItem::IsEquipable() const
{
	return bIsEquipable;
}

bool UInventoryItem::IsConsumable() const
{
	return bIsConsumable;
}
