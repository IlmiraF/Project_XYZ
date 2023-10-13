// Fill out your copyright notice in the Description page of Project Settings.


#include "Adrenaline.h"
#include "../../../Components/CharacterComponents/CharacterAttributeComponent.h"
#include "../../../Characters/GCBaseCharacter.h"

bool UAdrenaline::Consume(AGCBaseCharacter* ConsumeTarget)
{
	UCharacterAttributeComponent* CharacterAttributes = ConsumeTarget->GetCharacterAttributeComponent_Mutable();
	CharacterAttributes->RestoreFullStamina();
	this->ConditionalBeginDestroy();
	return true;
}
