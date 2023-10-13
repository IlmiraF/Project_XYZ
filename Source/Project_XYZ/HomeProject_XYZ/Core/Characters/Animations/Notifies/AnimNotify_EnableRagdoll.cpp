// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify_EnableRagdoll.h"
#include "../../../../../XYZ_ProjectTypes.h"

void UAnimNotify_EnableRagdoll::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	MeshComp->SetCollisionProfileName(CollisionProfileRagdoll);
	MeshComp->SetSimulatePhysics(true);
}
