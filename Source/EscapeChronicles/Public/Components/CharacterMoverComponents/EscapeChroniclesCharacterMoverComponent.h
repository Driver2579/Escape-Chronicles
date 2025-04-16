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

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGroundSpeedModeChangedDelegate, EGroundSpeedMode OldGroundSpeedMode,
		EGroundSpeedMode NewGroundSpeedMode);

	FOnGroundSpeedModeChangedDelegate OnGroundSpeedModeChanged;

protected:
	virtual void OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep,
		const FMoverInputCmdContext& InputCmd) override;

private:
	EGroundSpeedMode LastGroundSpeedMode;
};