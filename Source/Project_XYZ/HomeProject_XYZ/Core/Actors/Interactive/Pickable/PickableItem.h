// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/Interactive.h"
#include "PickableItem.generated.h"

class IInteractable;
UCLASS(Abstract, NotBlueprintable)
class PROJECT_XYZ_API APickableItem : public AActor, public IInteractable
{
	GENERATED_BODY()

public:

	const FName& DataTableID() const;
	
protected:	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName DataTableIDName = NAME_None;
};
