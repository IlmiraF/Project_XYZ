// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../../XYZ_ProjectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "UObject/ScriptInterface.h"
#include "../Subsystems/SaveSubsystem/SaveSubsystemInterface.h"
#include "SignificanceManager.h"
#include "GCBaseCharacter.generated.h"

USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* FPMantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveVector* MantlingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionXY = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionZ = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeightStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeightStartTime = 0.5f;
};


DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)
DECLARE_DELEGATE_OneParam(FOnInteractableObjectFound, FName)

class AInteractiveActor;
class UGCBaseCharacterMovementComponent;
class UCharacterEquipmentComponent;
class UCharacterAttributeComponent;
class IInteractable;
class UWidgetComponent;
class UInventoryItem;
class UCharacterInventoryComponent;

typedef TArray<AInteractiveActor*, TInlineAllocator<10>> TInteractiveActorsArray;

UCLASS(Abstract, NotBlueprintable)
class PROJECT_XYZ_API AGCBaseCharacter : public ACharacter, public IGenericTeamAgentInterface, public ISaveSubsystemInterface
{
	GENERATED_BODY()

public:
	AGCBaseCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//SubsystemInterface
	virtual void OnLevelDeserialized_Implementation() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void MoveForward(float value) {};
	virtual void MoveRight(float value) {};
	virtual void Turn(float value) {};
	virtual void LookUp(float value) {};
	virtual void ChangeCrouchState();
	virtual void StartSprint();
	virtual void StopSprint();
	virtual void ChangeProneState();
	virtual void Jump();

	virtual void Tick(float DeltaTime) override;

	virtual void SwimForward(float Value) {};
	virtual void SwimRight(float Value) {};
	virtual void SwimUp(float Value) {};

	UFUNCTION(BlueprintCallable)
	void Mantle(bool bForce = false);

	UPROPERTY(ReplicatedUsing = OnRep_IsMantling)
	bool bIsMantling;

	UFUNCTION()
	void OnRep_IsMantling(bool bWasMantling);

	virtual bool CanJumpInternal_Implementation() const override;

	bool bIsProned = false;
	void Prone();
	void UnProne(bool bIsFullHeight);
	bool CanProne() const;
	virtual void OnStartProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);
	virtual void OnEndProne(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	FORCEINLINE UGCBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const { return GCBaseCharacterMovementComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightFootOffset() const { return IKRightFootOffset; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFootOffset() const { return IKLeftFootOffset; }

	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(AInteractiveActor* InteractiveActor);

	void ClimbLadderUp(float Value);
	void InteractWithLadder();

	void InteractWithZipline();

	const class ALadder* GetAvailableLadder() const;
	const class AZipline* GetAvailableZipline() const;

	virtual void Falling() override;
	virtual void NotifyJumpApex() override;
	virtual void Landed(const FHitResult& Hit) override;

	const UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const;
	UCharacterEquipmentComponent* GetCharacterEquipmentComponent_Mutable() const;

	const UCharacterAttributeComponent* GetCharacterAttributeComponent() const;
	UCharacterAttributeComponent* GetCharacterAttributeComponent_Mutable() const;

	void StartFire();
	void StopFire();

	void StartAiming();
	void StopAiming();

	FRotator GetAimOffset();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStartAiming();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStopAiming();

	bool IsAiming() const;

	float GetAimingMovementSpeed() const;

	FOnAimingStateChanged OnAimingStateChanged;

	void Reload();

	void NextItem();
	void PreviousItem();

	bool IsSwimmingUnderWater() const;

	void EquipPrimaryItem();

	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();

	virtual FGenericTeamId GetGenericTeamId() const override;

	void Interact();

	FOnInteractableObjectFound OnInteractableObjectFound;

	UPROPERTY(VisibleDefaultsOnly, Category = "Character|Components")
	UWidgetComponent* HealthBarProgressComponent;

	void InitializeHealthProgress();

	bool PickupItem(TWeakObjectPtr<UInventoryItem> ItemToPickup);
	void UseInventory(APlayerController* PlayerController);

	void ConfirmWeaponSelection();

	//UFUNCTION(Server, Reliable)
	//	void Server_ActivatePlatformTrigger(class APlatformTrigger* PlatformTrigger, bool bIsActivated);

	//UFUNCTION(Client, Reliable)
	//	void Client_ActivatePlatformTrigger(class APlatformTrigger* PlatformTrigger, bool bIsActivated);

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Character|Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Movement")
	void OnSprintEnd();
	virtual void OnSprintEnd_Implementation();

	virtual void RecalculateBaseEyeHeight() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Movement")
	float SprintSpeed = 800.0f;

	virtual bool CanSprint();

	bool CanMantle() const;

	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime);

	UGCBaseCharacterMovementComponent* GCBaseCharacterMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|IK Settings")
	FName RightFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|IK Settings")
	FName LeftFootSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|IK Settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKTraceExtendDistance = 30.0f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Character|IK Settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKInterpSpeed = 20.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Movement")
	class ULedgeDetectorComponent* LedgeDetectorComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling")
	FMantlingSettings HighMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling")
	FMantlingSettings LowMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling", meta = (ClampMin =0.0f, UIMin =0.0f))
	float LowMantleMaxHeight = 125.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components")
	UCharacterAttributeComponent* CharacterAttributesComponent;

	virtual void OnDeath();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Animations")
	class UAnimMontage* OnDeathAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Attributes")
	class UCurveFloat* FallDamageCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components")
	UCharacterEquipmentComponent* CharacterEquipmentComponent;

	virtual void OnStartAimingInternal();
	virtual void OnStopAimingInternal();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Team")
	ETeams Team = ETeams::Enemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Team")
	float LineOfSightDistance = 500.0f;

	void TraceLineOfSight();

	UPROPERTY()
	TScriptInterface<IInteractable> LineOfSightObject;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	UCharacterInventoryComponent* CharacterInventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Significance")
	bool bIsSignificanceEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Significance")
	float VeryHighSignificanceDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Significance")
	float HighSignificanceDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Significance")
	float MediumSignificanceDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Significance")
	float LowSignificanceDistance =6000.0f;

private:
	void TryChangeSprintState(float DeltaTime);

	bool bIsSprintRequested = false;

	float GetIKOffsetForASocket(const FName& SocketName);

	float IKRightFootOffset = 0.0f;
	float IKLeftFootOffset = 0.0f;

	float IKTraceDistance = 0.0f;
	float IKScale = 0.0f;

	float CharacterScale = 1.0f;
	float CapsuleHalfHeightBase = 0.0f;
	float CapsuleHalfHeightCrouch = 0.0f;
	float PronedEyeHeight = 0.0f;

	void UpdateStamina(bool bIsOutOfStamina);

	const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;

	TInteractiveActorsArray AvailableInteractiveActors;

	FTimerHandle DeathMontageTimer;
	void EnableRagdoll();

	FVector CurrentFallApex;

	bool bIsAiming;
	float CurrentAimMovementSpeed;

	float SignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& ViewPoint);
	void PostSignificanceFunction(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal);
};
