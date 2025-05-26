// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "EscapeChroniclesCharacterMoverComponent.generated.h"

enum class EGroundSpeedMode : uint8;

UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesCharacterMoverComponent : public UCharacterMoverComponent
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Make movement impossible (sets movement mode to MOVE_None)
	void DisableMovement();

	//Set movement mode to the default based on the current physics volume
	void SetDefaultMovementMode();
	
	virtual bool CanCrouch() override { return !IsAirborne(); }

	/**
	 * Could be overriden by child classes to add more movement modifiers checks.
	 * @return Whether the MaxSpeed parameter in CommonLegacyMovementSettings is overriden or going to be modified in
	 * the next tick by any movement modifier other than GroundSpeedModeModifier by checking which movement modifiers
	 * are applied.
	 * @remark Used internally by the GroundSpeedModeModifier.
	 */
	virtual bool DoesMaxSpeedWantToBeOverriden() const;

	bool IsWalkGroundSpeedModeActive() const;
	bool IsJogGroundSpeedModeActive() const;
	bool IsRunGroundSpeedModeActive() const;

	void SetGroundSpeedMode(const EGroundSpeedMode NewGroundSpeedMode) const;
	void ResetGroundSpeedMode() const;
	
	float GetMaxSpeed() const;
	
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGroundSpeedModeChangedDelegate, EGroundSpeedMode OldGroundSpeedMode,
		EGroundSpeedMode NewGroundSpeedMode);

	FOnGroundSpeedModeChangedDelegate OnGroundSpeedModeChanged;

	const static FName NullModeName;
	
protected:
	virtual void OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep,
		const FMoverInputCmdContext& InputCmd) override;

	virtual void OnHandleImpact(const FMoverOnImpactParams& ImpactParams) override;

	/**
	 * Apply physics forces to the impacted component, if bEnablePhysicsInteraction is true.
	 * @param Impact				HitResult that resulted in the impact
	 * @param ImpactAcceleration	Acceleration of the character at the time of impact
	 * @param ImpactVelocity		Velocity of the character at the time of impact
	 */
	virtual void ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration,
		const FVector& ImpactVelocity);
	
private:
	// If enabled, the player will interact with physics objects when walking into them
	UPROPERTY(Category="Physics Interaction", EditAnywhere)
	uint8 bEnablePhysicsInteraction:1;

	// If enabled, the PushForceFactor is applied per kg mass of the affected object
	UPROPERTY(Category="Physics Interaction", EditAnywhere, meta=(editcondition = "bEnablePhysicsInteraction"))
	uint8 bPushForceScaledToMass:1;
	
	/**
	 * If enabled, the applied push force will try to get the physics object to the same velocity than the player, not
	 * faster. This will only scale the force down, it will never apply more force than defined by PushForceFactor.
	 */
	UPROPERTY(Category="Physics Interaction", EditAnywhere, meta=(editcondition = "bEnablePhysicsInteraction"))
	uint8 bScalePushForceToVelocity:1;

	// Initial impulse force to apply when the player bounces into a blocking physics object
	UPROPERTY(Category="Physics Interaction", EditAnywhere, meta=(editcondition = "bEnablePhysicsInteraction"))
	float InitialPushForceFactor;

	// Force to apply when the player collides with a blocking physics object
	UPROPERTY(Category="Physics Interaction", EditAnywhere, meta=(editcondition = "bEnablePhysicsInteraction"))
	float PushForceFactor;
	
	EGroundSpeedMode LastGroundSpeedMode;
};