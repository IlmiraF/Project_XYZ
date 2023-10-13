// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Inventory/Items/InventoryItem.h"

namespace GCDataTableUtils
{
	FWeaponTableRow* FindWeaponData(FName WeaponID);
	FItemTableRow* FindInventoryItemData(const FName ItemID);
};
