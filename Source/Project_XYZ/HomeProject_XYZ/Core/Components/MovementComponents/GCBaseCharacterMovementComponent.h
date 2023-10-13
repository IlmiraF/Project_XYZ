// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../LedgeDetectorComponent.h"
#include "../../../../XYZ_ProjectTypes.h"
#include "GCBaseCharacterMovementComponent.generated.h"

struct FMantlingMovementParameters
{
	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	FVector InitialAnimationLocation = FVector::ZeroVector;

	float Duration = 1.0f;
	float StartTime = 0.0f;

	UCurveVector* MantlingCurve;
};

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None = 0 UMETA(DisplayName = "None"),
	CMOVE_Mantling UMETA(DisplayName = "Mantling"),
	CMOVE_Ladder UMETA(DisplayName = "Ladder"),
	CMOVE_Zipline UMETA(DisplayName = "Zipline"),
	CMOVE_WallRun UMETA(DisplayName = "WallRun"),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDetachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff
};
/**
 * 
 */
UCLASS()
class PROJECT_XYZ_API UGCBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class FSavedMove_GC;
	
public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags);

	FORCEINLINE bool IsSprinting() { return bIsSprinting; }
	virtual float GetMaxSpeed() const override;

	void StartSprint();
	void StopSprint();

	bool IsOutOfStamina() const { return bIsOutOfStamina; }
	void SetIsOutOfStamina(bool bIsOutOfStamina_In) { bIsOutOfStamina = bIsOutOfStamina_In; }

	bool IsProning() const;
	void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	bool CanProneInCurrentState() const;
	void Prone();
	void UnProne();
	bool bWantsToProne = false;
	bool bIsFullHeight = false;
	bool bProneMaintainsBaseLocation = false;
	float PronedHalfHeight = 34.0f;

	void StartMantle(const FMantlingMovementParameters& MantlingParameters);
	void EndMantle();
	bool IsMantling() const;

	void AttachToLadder(const class ALadder* Ladder);
	void DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod = EDetachFromLadderMethod::Fall);
	bool IsOnLadder() const;

	const class ALadder* GetCurrentLadder();

	virtual void PhysicsRotation(float DeltaTime) override;

	float GetActorToCurrentLadderProjection(const FVector& Location) const;

	float GetLadderSpeedRatio() const;

	void AttachToZipline(const class AZipline* Zipline);
	void DetachFromZipline();
	bool IsOnZipline() const;

	//float GetActorToCurrentZiplineProjection(const FVector& Location) const;

	const class AZipline* GetCurrentZipline();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Sprint", meta = (ClimpMin = 0.0f, UIMin = 0.0f))
	float SprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Stamina", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OutOfStaminaSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Prone", meta = (ClimpMin = 0.0f, UIMin = 0.0f))
	float MaxProneSpeed = 100.0f;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SwimmingCapsuleRadius = 60.0f;

	UPROPERTY(Category = "Character Movement: Swimming", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SwimmingCapsuleHalfHeight = 60.0f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float ClimbingOnLadderMaxSpeed = 200.0f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float ClimbingOnLadderBrakingDecelaration = 2048.0f;

	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	void PhysMantling(float DeltaTime, int32 Iterations);
	void PhysLadder(float DeltaTime, int32 Iterations);
	void PhysZipline(float DeltaTime, int32 Iterations);

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float LadderToCharacterOffset = 60.0f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxLadderTopOffset = 90.0f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MinLadderBottomOffset = 90.0f;

	UPROPERTY(Category = "Character Movement: Ladder", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float JumpOffFromLadderSpeed = 500.0f;

	class AGCBaseCharacter* GetBaseCharacterOwner() const;

	UPROPERTY(Category = "Character Movement: Zipline", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float ZiplineMovementSpeed = 500.0f;

	UPROPERTY(Category = "Character Movement: Zipline", EditAnywhere, BlueprintReadOnly)
	UCurveVector* ZiplineCurve;

private:
	bool bIsSprinting = false;
	bool bIsOutOfStamina = false;

	FMantlingMovementParameters CurrentMantlingParameters;

	FTimerHandle MantlingTimer;
	FTimerHandle ZiplineTimer;

	const ALadder* CurrentLadder = nullptr;
	const AZipline* CurrentZipline = nullptr;

	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;
};

class FSavedMove_GC : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

public:
	virtual void Clear() override;

	virtual uint8 GetCompressedFlags() const override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const override;

	virtual void SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;

	virtual void PrepMoveFor(ACharacter* Character) override;
private:
	uint8 bSavedIsSprinting : 1;
	uint8 bSavedIsMantling : 1;

};

class FNetworkPredictionData_Client_Character_GC : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;

public:
	FNetworkPredictionData_Client_Character_GC(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};
