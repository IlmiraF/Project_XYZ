// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveCameraActor.h"

AInteractiveCameraActor::AInteractiveCameraActor()
{
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Camera Interaction Volume"));
	BoxComponent->SetBoxExtent(FVector(500.0f, 500.0f, 500.0f));
	BoxComponent->SetCollisionObjectType(ECC_WorldDynamic);
	BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxComponent->SetupAttachment(RootComponent);

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AInteractiveCameraActor::OnBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AInteractiveCameraActor::OnEndOverlap);
}

void AInteractiveCameraActor::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractiveCameraActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, FString::Printf(TEXT("J")));
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FViewTargetTransitionParams TransitionToCameraParams;
	TransitionToCameraParams.BlendTime = TransitionToCameraTime;
	PlayerController->SetViewTarget(this, TransitionToCameraParams);
}

void AInteractiveCameraActor::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	APawn* Pawn = PlayerController->GetPawn();
	FViewTargetTransitionParams TransitionToPawnParams;
	TransitionToPawnParams.BlendTime = TransitionToPawnTime;
	TransitionToPawnParams.bLockOutgoing = true;
	PlayerController->SetViewTarget(Pawn, TransitionToPawnParams);
}
