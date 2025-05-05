// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MovementModifier.h"
#include "GroundSpeedModeModifier.generated.h"

enum class EGroundSpeedMode : uint8;

/**
 * Every tick (OnPreMovement) updates the MaxSpeed of the CommonLegacyMovementSettings based on the
 * GroundSpeedModeSettings.
 */
USTRUCT()
struct ESCAPECHRONICLES_API FGroundSpeedModeModifier : public FMovementModifierBase
{
	GENERATED_BODY()

	FGroundSpeedModeModifier();

	virtual void OnPreMovement(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep) override;

	virtual FMovementModifierBase* Clone() const override
	{
		return new FGroundSpeedModeModifier(*this);
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual bool HasGameplayTag(FGameplayTag TagToFind, bool bExactMatch) const override;

private:
	EGroundSpeedMode LastGroundSpeedMode;
};