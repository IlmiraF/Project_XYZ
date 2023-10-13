// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GCPlayerController.generated.h"
/**
 * 
 */
UCLASS()
class PROJECT_XYZ_API AGCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	bool GetIgnoreCameraPitch() const;
	void SetIgnoreCameraPitch(bool bIgnoreCameraPitchIn);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void SetupInputComponent() override;

	void MoveForward(float value);
	void MoveRight(float value);
	void Turn(float value);
	void LookUp(float value);
	void Mantle();
	void Jump();
	void ChangeCrouchState();
	void ChangeProneState();

	void StartSprint();
	void StopSprint();

	void ClimbLadderUp(float value);
	void InteractWithLadder();
	void InteractWithZipline();

	void SwimForward(float value);
	void SwimRight(float value);
	void SwimUp(float value);

	void PlayerStartFire();
	void PlayerStopFire();

	void StartAiming();
	void StopAiming();

	void Reload();

	void NextItem();
	void PreviousItem();

	void EquipPrimaryItem();

	TSoftObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UPlayerHUDWidget> PlayerHUDWidgetClass;

	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

	void Interact();

	void UseInventory();

	void ConfirmWeaponWheelSelection();

	void QuickSaveGame();
	void QuickLoadGame();

private:

	void OnInteractableObjectFound(FName ActionName);

	void CreateAndInitializeWidgets();

	UPlayerHUDWidget* PlayerHUDWidget = nullptr;

	bool bIgnoreCameraPitch = false;

	void ToggleMainMenu();

	UUserWidget* MainMenuWidget = nullptr;
	
};
