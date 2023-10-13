// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetworkWidget.h"
#include "JoinSessionWidget.generated.h"

UENUM(BlueprintType)
enum class ESearchingSessionState : uint8
{
	None,
	Searching,
	SessionIsFound
};

class UProject_XYZ_GameInstance;
UCLASS()
class PROJECT_XYZ_API UJoinSessionWidget : public UNetworkWidget
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "Network Session")
	ESearchingSessionState SearchingSessionState;

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void FindOnlineSession();

	UFUNCTION(BlueprintCallable)
	void JoinOnlineSession();

	UFUNCTION(BlueprintNativeEvent)
	void OnMatchFound(bool bIsSuccesful);

	virtual void CloseWidget() override;

private:

	TWeakObjectPtr<UProject_XYZ_GameInstance> Project_XYZ_GameInstance;
};
