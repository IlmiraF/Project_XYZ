// Fill out your copyright notice in the Description page of Project Settings.


#include "HostSessionWidget.h"
#include "Kismet/GameplayStatics.h"
#include "../../../../../Project_XYZ_GameInstance.h"

void UHostSessionWidget::CreateSession()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	check(GameInstance->IsA<UProject_XYZ_GameInstance>());
	UProject_XYZ_GameInstance* Project_XYZ_GameInstance = StaticCast<UProject_XYZ_GameInstance*>(GetGameInstance());
	Project_XYZ_GameInstance->LaunchLobby(4, ServerName, bIsLAN);
}
