// Fill out your copyright notice in the Description page of Project Settings.


#include "GCDataTableUtils.h"
#include "Engine/DataTable.h"
#include "../Inventory/Items/InventoryItem.h"
#include "../../../XYZ_ProjectTypes.h"

FWeaponTableRow* GCDataTableUtils::FindWeaponData(FName WeaponID)
{
	static const FString ContextString(TEXT("Find Weapon Data"));

	UDataTable* WeaponDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/HomeProject_XYZ/Core/Data/DataTables/DT_WeaponList.DT_WeaponList"));

	if (WeaponDataTable == nullptr)
	{
		return nullptr;
	}

	return WeaponDataTable->FindRow<FWeaponTableRow>(WeaponID, ContextString);
}

FItemTableRow* GCDataTableUtils::FindInventoryItemData(const FName ItemID)
{
	static const FString ContextString(TEXT("Find Item Data"));

	UDataTable* InventoryItemDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/HomeProject_XYZ/Core/Data/DataTables/DT_InvenotryItemList.DT_InvenotryItemList"));

	if (InventoryItemDataTable == nullptr)
	{
		return nullptr;
	}

	return InventoryItemDataTable->FindRow<FItemTableRow>(ItemID, ContextString);
}