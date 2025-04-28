// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MoverTypes.h"
#include "EscapeChroniclesCharacterExtendedDefaultInputs.generated.h"

enum class EGroundSpeedMode : uint8;

// Data block containing extended ability inputs used by EscapeChroniclesCharacter
USTRUCT()
struct ESCAPECHRONICLES_API FEscapeChroniclesCharacterExtendedDefaultInputs : public FMoverDataStructBase
{
	GENERATED_USTRUCT_BODY()

	FEscapeChroniclesCharacterExtendedDefaultInputs();

	bool bWantsToBeCrouched = false;

	// "None" means the default ground speed mode should be used
	EGroundSpeedMode DesiredGroundSpeedModeOverride;

	/**
	 * Must be overridden by child classes.
	 * @return Newly allocated copy of this FEscapeChroniclesExtendedDefaultInputs. 
	 */
	virtual FMoverDataStructBase* Clone() const override
	{
		FEscapeChroniclesCharacterExtendedDefaultInputs* CopyPtr = new FEscapeChroniclesCharacterExtendedDefaultInputs(
			*this);

		return CopyPtr;
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		Super::NetSerialize(Ar, Map, bOutSuccess);

		Ar.SerializeBits(&bWantsToBeCrouched, 1);
		Ar << DesiredGroundSpeedModeOverride;

		bOutSuccess = true;
		return true;
	}

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	virtual void ToString(FAnsiStringBuilderBase& Out) const override
	{
		Super::ToString(Out);

		Out.Appendf("bWantsToBeCrouched: %i\n", bWantsToBeCrouched);

		Out.Appendf("DesiredGroundSpeedModeOverride: %s\n",
			*UEnum::GetValueAsString(DesiredGroundSpeedModeOverride));
	}
};