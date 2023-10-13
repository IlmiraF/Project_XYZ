// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ThrowableWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_XYZ_API UThrowableWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable")
	int32 Count;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Throwable")
	int32 TotalCount;

private:

	UFUNCTION()
		void UpdateThrowableCount(int32 NewCount, int32 NewTotalCount);
	
};
