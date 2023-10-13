// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "../../Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "CharacterEquipmentComponent.generated.h"

typedef TArray<class AEquipableItem*, TInlineAllocator<(uint32)EEquipmentSlots::MAX>> TItemsArray;
typedef TArray<int32, TInlineAllocator<(uint32)EAmunitionType::MAX>> TAmunitionArray;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponAmmoChanged, int32, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentThrowableChanged, int32, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedItemChanged, const AEquipableItem*)

class ARangeWeaponItem;
class AThrowableItem;
class AMeleeWeaponItem;
class UEquipmentViewWidget;
class UWeaponWheelWidget;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_XYZ_API UCharacterEquipmentComponent : public UActorComponent, public ISaveSubsystemInterface
{
	GENERATED_BODY()

public:

	UCharacterEquipmentComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	EEquipableItemType GetCurrentEquippedItemType() const;

	ARangeWeaponItem* GetCurrentRangeWeapon() const;

	AThrowableItem* GetCurrentThrowableItem() const;

	FOnCurrentWeaponAmmoChanged OnCurrentWeaponAmmoChangedEvent;
	FOnCurrentThrowableChanged OnCurrentThrowableChangedEvent;

	FOnEquippedItemChanged OnEquippedItemChanged;

	void ReloadCurrentWeapon();

	void EquipItemInSlot(EEquipmentSlots Slot);

	void EquipNextItem();
	void EquipPreviousItem();

	bool IsEquipping() const;

	void AttachCurrentItemToEquippedSocket();

	void ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo = 0, bool bCheckIsFull = false);

	void LaunchCurrentThrowableItem();

	AMeleeWeaponItem* GetCurrentMeleeWeapon() const;

	bool AddEquipmentItemToSlot(const TSubclassOf<class AEquipableItem> EquipableItemClass, int32 SlotIndex);
	void RemoveItemFromSlot(int32 SlotIndex);

	void OpenViewEquipment(APlayerController* PlayerController);
	void CloseViewEquipment();
	bool IsViewVisible() const;
	
	const TArray<AEquipableItem*>& GetItems() const;

	void OpenWeaponWheel(APlayerController* PlayerController);
	bool IsSelectingWeapon() const;
	void ConfirmWeaponSelection() const;

	virtual void OnLevelDeserialized_Implementation() override;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TMap<EAmunitionType, int32> MaxAmunitionAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TMap<EEquipmentSlots, TSubclassOf<AEquipableItem>> ItemsLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TSet<EEquipmentSlots> IgnoreSlotsWhileSwitching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	EEquipmentSlots AutoEquipItemInSlot = EEquipmentSlots::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
	TSubclassOf<UEquipmentViewWidget> ViewWidgetClass;

	void CreateEquipmentWidget(APlayerController* PlayerController);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
	TSubclassOf<UWeaponWheelWidget> WeaponWheelClass;

private:

	UFUNCTION(Server, Reliable)
	void Server_EquipItemInSlot(EEquipmentSlots Slot);

	void CreateLoadout();

	void AutoEquip();

	ARangeWeaponItem* CurrentEquippedWeapon;
	TWeakObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;

	UFUNCTION()
	void OnCurrentWeaponAmmoChanged(int32 Ammo);

	UFUNCTION()
	void OnCurrentThrowableChanged(int32 Count);

	UPROPERTY(Replicated, SaveGame)
	TArray<int32> AmunitionArray;

	UPROPERTY(ReplicatedUsing=OnRep_ItemsArray, SaveGame)
	TArray<AEquipableItem*> ItemsArray;

	UFUNCTION()
	void OnRep_ItemsArray();

	int32 GetAvailableAmunitionForCurrentWeapon();
	int32 GetAvailableAmunitionForCurrentThrowable();

	UFUNCTION()
	void OnWeaponReloadComplete();

	AEquipableItem* CurrentEquippedItem;
	//EEquipmentSlots CurrentEquippedSlot;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentEquippedSlot, SaveGame)
	EEquipmentSlots CurrentEquippedSlot;

	UFUNCTION()
	void OnRep_CurrentEquippedSlot(EEquipmentSlots CurrentEquippedSlot_Old);

	FDelegateHandle OnCurrentWeaponAmmoChangedHandle;
	FDelegateHandle OnCurrentWeaponReloadedHandle;
	FDelegateHandle OnCurrentThrowableHandle;

	uint32 NextItemsArraySlotIndex(uint32 CurrentSlotIndex);
	uint32 PreviousItemsArraySlotIndex(uint32 CurrentSlotIndex);

	void UnEquipCurrentItem();

	bool bIsEquipping = false;
	FTimerHandle EquipTimer;

	void EquipAnimationFinished();

	EEquipmentSlots PreviousEquippedSlot;
	AThrowableItem* CurrentThrowableItem;

	AMeleeWeaponItem* CurrentMeleeWeapon;

	UEquipmentViewWidget* ViewWidget;
	UWeaponWheelWidget* WeaponWheelWidget;
};
