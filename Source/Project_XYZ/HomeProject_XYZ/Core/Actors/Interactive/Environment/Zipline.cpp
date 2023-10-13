// Fill out your copyright notice in the Description page of Project Settings.

#include "Zipline.h"
#include "../../../../../XYZ_ProjectTypes.h"
#include <cmath>
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ZiplineRoot"));

	LeftRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Rail"));
	LeftRailMeshComponent->SetupAttachment(RootComponent);

	RightRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Rail"));
	RightRailMeshComponent->SetupAttachment(RootComponent);

	CableMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cable"));
	CableMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Interaction Volume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	LeftRailMeshComponent->SetRelativeLocation(FVector(0.0f, -ZiplineWidth * 0.5f, 0.0f));
	RightRailMeshComponent->SetRelativeLocation(FVector(0.0f, ZiplineWidth * 0.5f, 0.0f));
	CableMeshComponent->SetRelativeLocation(FVector(0.0f, -ZiplineWidth * 0.5f, ZiplineHeight));

	UStaticMesh* LeftRailMesh = LeftRailMeshComponent->GetStaticMesh();
	if (IsValid(LeftRailMesh))
	{
		float MeshHeight = LeftRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			LeftRailMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, ZiplineHeight / MeshHeight));
		}
	}

	UStaticMesh* RightRailMesh = RightRailMeshComponent->GetStaticMesh();
	if (IsValid(RightRailMesh))
	{
		float MeshHeight = RightRailMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			RightRailMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, (ZiplineHeight / MeshHeight) * 2.0f));
		}
	}

	UStaticMesh* CableMesh = CableMeshComponent->GetStaticMesh();
	if (IsValid(CableMesh))
	{
		float MeshWidth = CableMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshWidth))
		{
			Hypotenuse = std::sqrt(std::pow(ZiplineHeight, 2) + std::pow(ZiplineWidth, 2));
			Angle = FMath::RadiansToDegrees(1.57f - FMath::Sin(ZiplineHeight / Hypotenuse));
			CableMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, Hypotenuse / MeshWidth));
			CableMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, Angle));
		}
	}

	UCapsuleComponent* CableCapsuleComponent = GetZiplineInteractionCapsule();
	if (IsValid(CableCapsuleComponent))
	{
		CableCapsuleComponent->SetRelativeLocation(FVector(0.0f, 0.0f, ZiplineHeight * 1.5f));
		CableCapsuleComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, Angle));
		CableCapsuleComponent->SetCapsuleRadius(50.0f);
		CableCapsuleComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, (Hypotenuse / CableCapsuleComponent->GetUnscaledCapsuleHalfHeight()) * 0.5f));
	}
}

void AZipline::BeginPlay()
{
	Super::BeginPlay();
}

UAnimMontage* AZipline::GetAttachAnimMontage() const
{
	return AttachAnimMontage;
}

FVector AZipline::GetAttachAnimMontageStartingLocation() const
{
	FRotator OrientationRotation = GetActorForwardVector().ToOrientationRotator();
	FVector Offset = OrientationRotation.RotateVector(AttachAnimMontageInitialOffset);

	FVector LadderTop = GetActorLocation() - GetActorUpVector() * ZiplineWidth;
	return LadderTop - Offset;
}

float AZipline::GetZiplineHeight() const
{
	return ZiplineHeight;
}

float AZipline::GetZiplineWidth() const
{
	return ZiplineWidth;
}

UStaticMeshComponent* AZipline::GetCableMeshComponent() const
{
	return CableMeshComponent;
}

UCapsuleComponent* AZipline::GetZiplineInteractionCapsule() const
{
	return StaticCast<UCapsuleComponent*>(InteractionVolume);
}
