// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

/**
 * 
 */
class UHighlightInteractable;
UCLASS()
class PROJECT_XYZ_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	class UReticleWidget* GetReticleWidget();

	class UAmmoWidget* GetAmmoWidget();

	class UThrowableWidget* GetThrowableWidget();

	void SetHighlightInteractableVisibility(bool bIsVisible);

	void SetHighlightInteractableActionText(FName KeyName);

protected:
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable)
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintCallable)
	float GetOxygenPercent() const;

	UFUNCTION(BlueprintCallable)
	bool GetStaminaVisibility() const;

	UFUNCTION(BlueprintCallable)
	bool GetOxygenVisibility() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Names")
	FName ReticleWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Names")
	FName AmmoWidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Names")
	FName ThrowableWidgetName;

	UPROPERTY(meta = (BindWidget))
	UHighlightInteractable* InteractableKey;

private:

	bool StaminaVisibility;
	bool OxygenVisibility;
	
};
