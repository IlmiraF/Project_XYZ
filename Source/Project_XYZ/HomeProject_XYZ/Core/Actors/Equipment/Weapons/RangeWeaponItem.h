// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../EquipableItem.h"
#include "../../../../../XYZ_ProjectTypes.h"
#include "../../../Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "RangeWeaponItem.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnReloadComplete);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, int32);

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	Single,
	FullAuto
};

UENUM(BlueprintType)
enum class EReloadType : uint8
{
	FullClip,
	ByBullet
};

/**
 * 
 */
class UAnimMontage;
UCLASS(Blueprintable)
class PROJECT_XYZ_API ARangeWeaponItem : public AEquipableItem, public ISaveSubsystemInterface
{
	GENERATED_BODY()

public:
	ARangeWeaponItem();

	void StartFire();
	void StopFire();

	bool IsFiring() const;

	FTransform GetForeGripTransform() const;

	float GetAimFOV() const;
	float GetAimMovementMaxSpeed() const;

	void StartAim();
	void StopAim();

	int32 GetAmmo() const;
	void SetAmmo(int32 NewAmmo);

	bool CanShot() const;

	FOnAmmoChanged OnAmmoChanged;

	EAmunitionType GetAmmoType() const;

	int32 GetMaxAmmo() const;

	void StartReload();
	void EndReload(bool bIsSuccess);

	bool IsReloading() const;

	FOnReloadComplete OnReloadComplete;

	float GetAimTurnModifier() const;
	float GetAimLookUpModifier() const;

	virtual EReticleType GetReticleType() const;

	virtual void OnLevelDeserialized_Implementation() override;

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWeaponBarellComponent* WeaponBarell;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Weapon")
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Weapon")
	UAnimMontage* WeaponReloadMontage;

	//FullClip reload type adds ammo only when the whole reload animation is successfully played
	//ByBullet reload type requires section "ReloadEnd" in character reload animation 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Weapon")
	EReloadType ReloadType = EReloadType::FullClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Character")
	UAnimMontage* CharacterFireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Character")
	UAnimMontage* CharacterReloadMontage;

	//Rate of fire in rounds per minute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters", meta = (ClampMin = 1.0f, UIMin = 1.0f))
	float RateOfFire = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters")
	EWeaponFireMode WeaponFireMode = EWeaponFireMode::Single;

	//Bullet spread half angle in degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float SpreadAngle = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 2.0f, UIMax = 2.0f))
	float AimSpreadAngle = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aiming")
	float AimMovementMaxSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aiming")
	float AimFOV = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float AimTurnModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Aiming", meta = (ClampMin = 0.0f, UIMin = 0.0f, ClampMax = 1.0f, UIMax = 1.0f))
	float AimLookUpModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Ammo", meta = (ClampMin = 1, UIMin = 1))
	int32 MaxAmmo = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Ammo")
	EAmunitionType AmmoType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Parameters|Ammo")
	bool bAutoReload = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reticle")
	EReticleType AimReticleType = EReticleType::Default;

private:

	float PlayAnimMontage(UAnimMontage* AnimMontage);

	FTimerHandle ShotTimer;
	float GetShotTimerInterval() const;

	void MakeShot();

	bool bIsAiming;

	float GetCurrentBulletSpreadAngle() const;

	UPROPERTY(SaveGame)
	int32 Ammo = 0;

	bool bIsReloading = false;
	FTimerHandle ReloadTimer;

	void StopAnimMontage(UAnimMontage* AnimMontage, float BlendOutTime = 0.0f);

	bool bIsFiring = false;
	void OnShotTimerElapsed();
};
