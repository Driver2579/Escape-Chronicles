// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChronicles/Public/Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"

#include "EscapeChroniclesGameplayTags.h"
#include "Common/Enums/Mover/GroundSpeedMode.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "Mover/MovementModifiers/GroundSpeedModeModifier.h"
#include "Mover/MovementSettings/GroundSpeedModeSettings.h"

#if WITH_EDITOR
void UEscapeChroniclesCharacterMoverComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UCommonLegacyMovementSettings* CommonLegacyMovementSettings = FindSharedSettings_Mutable<
		UCommonLegacyMovementSettings>();

	const UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings<UGroundSpeedModeSettings>();

	/**
	 * Override MaxSpeed in CommonLegacyMovementSettings if GroundSpeedModeSettings exist to avoid confusion since
	 * MaxSpeed is always overriden by GroundSpeedModeModifier in runtime if it exists.
	 */
	if (IsValid(CommonLegacyMovementSettings) && IsValid(GroundSpeedModeSettings))
	{
		CommonLegacyMovementSettings->MaxSpeed = GroundSpeedModeSettings->GetDefaultMaxSpeed();
	}
}
#endif

void UEscapeChroniclesCharacterMoverComponent::BeginPlay()
{
	Super::BeginPlay();

	const UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings<UGroundSpeedModeSettings>();

	// Add GroundSpeedModeModifier only if GroundSpeedModeSettings exist
	if (IsValid(GroundSpeedModeSettings))
	{
		QueueMovementModifier(MakeShared<FGroundSpeedModeModifier>());
	}
}

bool UEscapeChroniclesCharacterMoverComponent::DoesMaxSpeedWantToBeOverriden() const
{
	// We use cons_cast here because CanCrouch should be const, BUT EPIC GAMES FOR SOME FUCKING REASON MADE IT NON-CONST
	return IsCrouching() || (bWantsToCrouch && const_cast<ThisClass*>(this)->CanCrouch());
}

bool UEscapeChroniclesCharacterMoverComponent::IsWalkGroundSpeedModeActive() const
{
	return HasGameplayTag(EscapeChroniclesGameplayTags::Mover_IsWalkGroundSpeedModeActive, true);
}

bool UEscapeChroniclesCharacterMoverComponent::IsJogGroundSpeedModeActive() const
{
	return HasGameplayTag(EscapeChroniclesGameplayTags::Mover_IsJogGroundSpeedModeActive, true);
}

bool UEscapeChroniclesCharacterMoverComponent::IsRunGroundSpeedModeActive() const
{
	return HasGameplayTag(EscapeChroniclesGameplayTags::Mover_IsRunGroundSpeedModeActive, true);
}

void UEscapeChroniclesCharacterMoverComponent::SetGroundSpeedMode(const EGroundSpeedMode NewGroundSpeedMode) const
{
#if DO_ENSURE
	ensureAlwaysMsgf(NewGroundSpeedMode < EGroundSpeedMode::NumberOfModes,
		TEXT("Invalid NewGroundSpeedMode was passed!"));
#endif

	UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings_Mutable<UGroundSpeedModeSettings>();

	if (ensureAlways(IsValid(GroundSpeedModeSettings)))
	{
		GroundSpeedModeSettings->GroundSpeedMode = NewGroundSpeedMode;
	}
}

void UEscapeChroniclesCharacterMoverComponent::ResetGroundSpeedMode() const
{
	UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings_Mutable<UGroundSpeedModeSettings>();

	if (ensureAlways(IsValid(GroundSpeedModeSettings)))
	{
		GroundSpeedModeSettings->GroundSpeedMode = GroundSpeedModeSettings->GetDefaultGroundSpeedMode();
	}
}

void UEscapeChroniclesCharacterMoverComponent::OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep,
	const FMoverInputCmdContext& InputCmd)
{
	Super::OnMoverPreSimulationTick(TimeStep, InputCmd);

	const UGroundSpeedModeSettings* GroundSpeedModeSettings = FindSharedSettings<UGroundSpeedModeSettings>();

	if (IsValid(GroundSpeedModeSettings) && GroundSpeedModeSettings->GroundSpeedMode != LastGroundSpeedMode)
	{
		OnGroundSpeedModeChanged.Broadcast(LastGroundSpeedMode, GroundSpeedModeSettings->GroundSpeedMode);

		LastGroundSpeedMode = GroundSpeedModeSettings->GroundSpeedMode;
	}
}