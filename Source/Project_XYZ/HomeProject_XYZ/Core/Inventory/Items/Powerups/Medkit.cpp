// Fill out your copyright notice in the Description page of Project Settings.


#include "Medkit.h"
#include "../../../Components/CharacterComponents/CharacterAttributeComponent.h"
#include "../../../Characters/GCBaseCharacter.h"

bool UMedkit::Consume(AGCBaseCharacter* ConsumeTarget)
{
    UCharacterAttributeComponent* CharacterAttributes = ConsumeTarget->GetCharacterAttributeComponent_Mutable();
    CharacterAttributes->AddHealth(Health);
    this->ConditionalBeginDestroy();
    return true;
}
