// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "../../Characters/GCBaseCharacter.h"
#include "../../Components/CharacterComponents/CharacterAttributeComponent.h"
#include "ReticleWidget.h"
#include "AmmoWidget.h"
#include "ThrowableWidget.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/Pawn.h"
#include "HighlightInteractable.h"

UReticleWidget* UPlayerHUDWidget::GetReticleWidget()
{
    return WidgetTree->FindWidget<UReticleWidget>(ReticleWidgetName);
}

UAmmoWidget* UPlayerHUDWidget::GetAmmoWidget()
{
    return WidgetTree->FindWidget<UAmmoWidget>(AmmoWidgetName);
}

UThrowableWidget* UPlayerHUDWidget::GetThrowableWidget()
{
    return WidgetTree->FindWidget<UThrowableWidget>(ThrowableWidgetName);
}

void UPlayerHUDWidget::SetHighlightInteractableVisibility(bool bIsVisible)
{
    if (!IsValid(InteractableKey))
    {
        return;
    }

    if (bIsVisible)
    {
        InteractableKey->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        InteractableKey->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerHUDWidget::SetHighlightInteractableActionText(FName KeyName)
{
    if (IsValid(InteractableKey))
    {
        InteractableKey->SetActionText(KeyName);
    }
}

float UPlayerHUDWidget::GetHealthPercent() const
{
    float Result = 1.0f;
    APawn* Pawn = GetOwningPlayerPawn();
    AGCBaseCharacter* GCBaseCharacter = Cast<AGCBaseCharacter>(Pawn);
    if (IsValid(GCBaseCharacter))
    {
        const UCharacterAttributeComponent* CharacterAttribute = GCBaseCharacter->GetCharacterAttributeComponent();
        Result = CharacterAttribute->GetHealthPercent();
    }
    return Result;
}

float UPlayerHUDWidget::GetStaminaPercent() const
{
    float Result = 1.0f;
    APawn* Pawn = GetOwningPlayerPawn();
    AGCBaseCharacter* GCBaseCharacter = Cast<AGCBaseCharacter>(Pawn);
    if (IsValid(GCBaseCharacter))
    {
        const UCharacterAttributeComponent* CharacterAttribute = GCBaseCharacter->GetCharacterAttributeComponent();
        Result = CharacterAttribute->GetStaminaPercent();
    }
    return Result;
}

float UPlayerHUDWidget::GetOxygenPercent() const
{
    float Result = 1.0f;
    APawn* Pawn = GetOwningPlayerPawn();
    AGCBaseCharacter* GCBaseCharacter = Cast<AGCBaseCharacter>(Pawn);
    if (IsValid(GCBaseCharacter))
    {
        const UCharacterAttributeComponent* CharacterAttribute = GCBaseCharacter->GetCharacterAttributeComponent();
        Result = CharacterAttribute->GetOxygenPercent();
    }
    return Result;
}

bool UPlayerHUDWidget::GetStaminaVisibility() const
{
    return GetStaminaPercent() < 1.0f;
}

bool UPlayerHUDWidget::GetOxygenVisibility() const
{
    return GetOxygenPercent() < 1.0f;
}
