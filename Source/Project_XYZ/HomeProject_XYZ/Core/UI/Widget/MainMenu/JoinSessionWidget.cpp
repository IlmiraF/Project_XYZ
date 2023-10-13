// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinSessionWidget.h"
#include "Kismet/GameplayStatics.h"
#include "../../../../../Project_XYZ_GameInstance.h"

void UJoinSessionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UProject_XYZ_GameInstance>());
	Project_XYZ_GameInstance = StaticCast<UProject_XYZ_GameInstance*>(GetGameInstance());
}

void UJoinSessionWidget::FindOnlineSession()
{
	Project_XYZ_GameInstance->OnMatchFound.AddUFunction(this, FName("OnMatchFound"));
	Project_XYZ_GameInstance->FindAMatch(bIsLAN);
	SearchingSessionState = ESearchingSessionState::Searching;
}

void UJoinSessionWidget::JoinOnlineSession()
{
	Project_XYZ_GameInstance->JoinOnlineGame();
}

void UJoinSessionWidget::CloseWidget()
{
	Project_XYZ_GameInstance->OnMatchFound.RemoveAll(this);
	Super::CloseWidget();
}

void UJoinSessionWidget::OnMatchFound_Implementation(bool bIsSuccesful)
{
	SearchingSessionState = bIsSuccesful ? ESearchingSessionState::SessionIsFound : ESearchingSessionState::None;
	Project_XYZ_GameInstance->OnMatchFound.RemoveAll(this);
}
