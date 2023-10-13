// Fill out your copyright notice in the Description page of Project Settings.


#include "PickableWeapon.h"
#include "../../../Characters/GCBaseCharacter.h"
#include "../../../../../XYZ_ProjectTypes.h"
#include "../../../Utils/GCDataTableUtils.h"
#include "Engine/DataTable.h"
#include "../../../Inventory/Items/InventoryItem.h"
#include "../../../Inventory/Items/Equipables/WeaponInventoryItem.h"

APickableWeapon::APickableWeapon()
{
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon Mesh"));
	SetRootComponent(WeaponMesh);
}

void APickableWeapon::Interact(AGCBaseCharacter* Character)
{
	FWeaponTableRow* WeaponRow = GCDataTableUtils::FindWeaponData(DataTableIDName);
	if (WeaponRow)
	{
		TWeakObjectPtr<UWeaponInventoryItem> Weapon = NewObject<UWeaponInventoryItem>(Character);
		Weapon->Initialize(DataTableIDName, WeaponRow->WeaponItemDescription);
		Weapon->SetEquipWeaponClass(WeaponRow->EquipableActor);
		Character->PickupItem(Weapon);
		Destroy();
	}
}

FName APickableWeapon::GetActionEventName() const
{
	return ActionInteract;
}
