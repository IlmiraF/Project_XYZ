// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "WeaponBarellComponent.generated.h"


UENUM(BlueprintType)
enum class EHitRegistrationType : uint8
{
	HitScan,
	Projectile
};

USTRUCT(BlueprintType)
struct FDecalInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal Info")
	UMaterialInterface* DecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal Info")
	FVector DecalSize = FVector(5.0f, 5.0f, 5.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal Info")
	float DecalLifeTime = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal Info")
	float DecalFadeOutTime = 5.0f;
};

USTRUCT(BlueprintType)
struct FShotInfo
{
	GENERATED_BODY()

	FShotInfo() : Location_Mul_10(FVector_NetQuantize100::ZeroVector), Direction(FVector::ZeroVector) {};
	FShotInfo(FVector Location, FVector Direction) : Location_Mul_10(Location * 10.0f), Direction(Direction) {};

	UPROPERTY()
	FVector_NetQuantize100 Location_Mul_10;

	UPROPERTY()
	FVector_NetQuantizeNormal Direction;

	FVector GetLocation() const { return Location_Mul_10 * 0.1f; }
	FVector GetDirection() const { return Direction; }
};

class AGCProjectile;
class UNiagaraSystem;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_XYZ_API UWeaponBarellComponent : public USceneComponent
{
	GENERATED_BODY()

public:	

	UWeaponBarellComponent();

	void Shot(FVector ShotStart, FVector ShotDirection, float SpreadAngle);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes")
	float FiringRange = 5000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes", meta = (ClampMin = 1, UIMin = 1))
	int32 BulletsPerShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes|Damage")
	float DamageAmount = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes")
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes|Damage")
	UCurveFloat* FalloffDiagram;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes|VFX")
	UNiagaraSystem* MuzzleFlashFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes|VFX")
	UNiagaraSystem* TraceFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes|Decals")
	FDecalInfo DefaultDecalInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes|Hit Registration")
	EHitRegistrationType HitRegistration = EHitRegistrationType::HitScan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes|Hit Registration", meta = (ClampMin = 1, UIMin = 1))
	int32 ProjectilePoolSize = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barell Attributes|Hit Registration", meta = (EditCondition = "HitRegistration == EHitRegistrationType::Projectile"))
	TSubclassOf<AGCProjectile> ProjectileClass;

	virtual void BeginPlay() override;

private:
	void ShotInternal(const TArray<FShotInfo>& ShotsInfo);

	UFUNCTION(Server, Reliable)
		void Server_Shot(const TArray<FShotInfo>& ShotsInfo);

	UPROPERTY(ReplicatedUsing = OnRep_LastShotsInfo)
		TArray<FShotInfo> LastShotsInfo;

	UPROPERTY(Replicated)
		TArray<AGCProjectile*> ProjectilePool;

	UPROPERTY(Replicated)
		int32 CurrentProjectileIndex;

	UFUNCTION()
		void OnRep_LastShotsInfo();

	APawn* GetOwningPawn() const;
	AController* GetController() const;

	UFUNCTION()
		void ProcessProjectileHit(AGCProjectile* Projectile, const FHitResult& HitResult, const FVector& Direction);

	UFUNCTION()
		void ProcessHit(const FHitResult& HitResult, const FVector& Direction);
	bool HitScan(FVector ShotStart, OUT FVector& ShotEnd, FVector ShotDirection);
	void LaunchProjectile(const FVector& LaunchStart, const FVector& LaunchDirection);

	FVector GetBulletSpreadOffset(float Angle, FRotator ShotRotation) const;

	const FVector ProjectilePoolLocation = FVector(0.0f, 0.0f, 100.0f);
};
