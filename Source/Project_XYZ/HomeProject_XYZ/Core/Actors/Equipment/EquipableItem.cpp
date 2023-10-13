// Fill out your copyright notice in the Description page of Project Settings.

#include "EquipableItem.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "../../Characters/GCBaseCharacter.h"

AEquipableItem::AEquipableItem()
{
	SetReplicates(true);
}

void AEquipableItem::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);
	if (IsValid(NewOwner))
	{
		checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("AEquipableItem::SetOwner() only character can be an owner of an equipable item"));
		CachedCharacterOwner = StaticCast<AGCBaseCharacter*>(GetOwner());
		if (GetLocalRole() == ROLE_Authority)
		{
			SetAutonomousProxy(true);
		}
	}
	else
	{
		CachedCharacterOwner = nullptr;
	}
}

EEquipableItemType AEquipableItem::GetItemType() const
{
	return ItemType;
}

FName AEquipableItem::GetUnEquippedSocketName() const
{
	return UnEquippedSocketName;
}

FName AEquipableItem::GetEquippedSocketName() const
{
	return EquippedSocketName;
}

UAnimMontage* AEquipableItem::GetCharacterEquipMontage() const
{
	return CharacterEquipAnimMontage;
}

void AEquipableItem::Equip()
{
	if (OnEquipmentStateChanged.IsBound())
	{
		OnEquipmentStateChanged.Broadcast(true);
	}
}

void AEquipableItem::UnEquip()
{
	if (OnEquipmentStateChanged.IsBound())
	{
		OnEquipmentStateChanged.Broadcast(false);
	}
}

EReticleType AEquipableItem::GetReticleType() const
{
	return ReticleType;
}

FName AEquipableItem::GetDataTableID() const
{
	return DataTableIDName;
}

bool AEquipableItem::IsSlotCompatable(EEquipmentSlots Slot)
{
	return CompatableEquipmentSlots.Contains(Slot);
}

AGCBaseCharacter* AEquipableItem::GetCharacterOwner() const
{
	return CachedCharacterOwner.IsValid() ? CachedCharacterOwner.Get() : nullptr;
}
