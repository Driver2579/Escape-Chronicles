// Fill out your copyright notice in the Description page of Project Settings.

#include "Mover/MovementModifiers/GroundSpeedModeModifier.h"

#include "EscapeChroniclesGameplayTags.h"
#include "MoverComponent.h"
#include "Common/Enums/Mover/GroundSpeedMode.h"
#include "Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "Mover/MovementSettings/GroundSpeedModeSettings.h"

FGroundSpeedModeModifier::FGroundSpeedModeModifier()
	: LastGroundSpeedMode(EGroundSpeedMode::None)
{
	DurationMs = -1;
}

void FGroundSpeedModeModifier::OnPreMovement(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep)
{
	FMovementModifierBase::OnPreMovement(MoverComp, TimeStep);

	const UEscapeChroniclesCharacterMoverComponent* CharacterMoverComponent =
		Cast<UEscapeChroniclesCharacterMoverComponent>(MoverComp);

	if (!ensureAlways(IsValid(CharacterMoverComponent)) || CharacterMoverComponent->DoesMaxSpeedWantToBeOverriden())
	{
		return;
	}

	UCommonLegacyMovementSettings* MovementSettings = MoverComp->FindSharedSettings_Mutable<
		UCommonLegacyMovementSettings>();

	const UGroundSpeedModeSettings* GroundSpeedModeSettings = MoverComp->FindSharedSettings<UGroundSpeedModeSettings>();

	if (ensureAlways(IsValid(MovementSettings)) && ensureAlways(IsValid(GroundSpeedModeSettings)))
	{
		MovementSettings->MaxSpeed = GroundSpeedModeSettings->GetSelectedMaxSpeed();
		LastGroundSpeedMode = GroundSpeedModeSettings->GroundSpeedMode;
	}
}

bool FGroundSpeedModeModifier::HasGameplayTag(FGameplayTag TagToFind, bool bExactMatch) const
{
	if (LastGroundSpeedMode == EGroundSpeedMode::Walking)
	{
		return bExactMatch ?
			TagToFind.MatchesTagExact(EscapeChroniclesGameplayTags::Mover_IsWalkGroundSpeedModeActive) :
			TagToFind.MatchesTag(EscapeChroniclesGameplayTags::Mover_IsWalkGroundSpeedModeActive);
	}

	if (LastGroundSpeedMode == EGroundSpeedMode::Jogging)
	{
		return bExactMatch ?
			TagToFind.MatchesTagExact(EscapeChroniclesGameplayTags::Mover_IsJogGroundSpeedModeActive) :
			TagToFind.MatchesTag(EscapeChroniclesGameplayTags::Mover_IsJogGroundSpeedModeActive);
	}

	if (LastGroundSpeedMode == EGroundSpeedMode::Running)
	{
		return bExactMatch ?
			TagToFind.MatchesTagExact(EscapeChroniclesGameplayTags::Mover_IsRunGroundSpeedModeActive) :
			TagToFind.MatchesTag(EscapeChroniclesGameplayTags::Mover_IsRunGroundSpeedModeActive);
	}

	return false;
}