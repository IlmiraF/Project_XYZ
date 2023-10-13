// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "InventoryItem.generated.h"

/**
 * 
 */

class AGCBaseCharacter;
class UInventoryItem;
class AEquipableItem;
class APickableItem;

USTRUCT(BlueprintType)
struct FInventoryItemDescription : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item view")
	UTexture2D* Icon;
};

USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon view")
	TSubclassOf<APickableItem> PickableActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon view")
	TSubclassOf<AEquipableItem> EquipableActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon view")
	FInventoryItemDescription WeaponItemDescription;
};

USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon view")
	TSubclassOf<APickableItem> PickableActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon view")
	TSubclassOf<UInventoryItem> InventoryItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon view")
	FInventoryItemDescription InventoryItemDescription;
};

UCLASS(Blueprintable)
class PROJECT_XYZ_API UInventoryItem : public UObject
{
	GENERATED_BODY()

public:

	void Initialize(FName DataTableID_In, const FInventoryItemDescription& Description_In);

	FName GetDataTableID() const;
	const FInventoryItemDescription& GetDescription() const;
 
	virtual bool IsEquipable() const;
	virtual bool IsConsumable() const;

	virtual bool Consume(AGCBaseCharacter* ConsumeTarget) PURE_VIRTUAL(UInventoryItem::Consume, return false;);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item")
	FName DataTableIDName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item")
	FInventoryItemDescription Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item")
	bool bIsEquipable = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Item")
	bool bIsConsumable = false;

private:
	bool bIsInitialized = false;
};
