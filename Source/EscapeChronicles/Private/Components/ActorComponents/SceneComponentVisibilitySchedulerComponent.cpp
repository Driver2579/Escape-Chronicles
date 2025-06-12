// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/SceneComponentVisibilitySchedulerComponent.h"

#include "GameState/EscapeChroniclesGameState.h"

USceneComponentVisibilitySchedulerComponent::USceneComponentVisibilitySchedulerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USceneComponentVisibilitySchedulerComponent::BeginPlay()
{
	Super::BeginPlay();

	AEscapeChroniclesGameState* GameState = GetWorld()->GetGameState<AEscapeChroniclesGameState>();

	if (!ensureAlways(IsValid(GameState)))
	{
		return;
	}

	// Cache the pointer to the target component based on the reference set in the editor
	TargetComponent = CastChecked<USceneComponent>(TargetComponentReference.GetComponent(GetOwner()),
		ECastCheckedType::NullAllowed);

#if DO_ENSURE
	ensureAlwaysMsgf(TargetComponent.IsValid(),
		TEXT("No target component is set for visibility scheduling, this component will work for nothing! "
			"Consider removing this component or setting the target component."));
#endif

	// Initialize the visibility of the component based on the current game time
	UpdateComponentVisibility(GameState->GetCurrentGameDateTime().Time, true);

	// Listen for game date time changes
	GameState->OnCurrentGameDateTimeUpdated.AddUObject(this, &ThisClass::OnCurrentGameDateTimeUpdated);
}

void USceneComponentVisibilitySchedulerComponent::OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldDateTime,
	const FGameplayDateTime& NewDateTime)
{
	// Update the visibility of the component based on the new game time
	UpdateComponentVisibility(NewDateTime.Time);
}

void USceneComponentVisibilitySchedulerComponent::UpdateComponentVisibility(
	const FGameplayTime& CurrentGameTime, const bool bForceUpdate)
{
	// Whether the component should be visible. Depends on the current game time.
	bool bNewVisibility;

	// Usual scenario. Example: 10:00-11:30.
	if (ComponentVisibleStartTime < ComponentVisibleEndTime)
	{
		bNewVisibility = CurrentGameTime >= ComponentVisibleStartTime && CurrentGameTime < ComponentVisibleEndTime;
	}
	// Special scenario. Example: 22:00-01:30.
	else
	{
		bNewVisibility = CurrentGameTime >= ComponentVisibleStartTime || CurrentGameTime < ComponentVisibleEndTime;
	}

	// No need to update visibility if it hasn't changed unless the force update was requested
	if (!bForceUpdate && bNewVisibility == bCurrentVisibility)
	{
		return;
	}

	// Update the visibility of the target component and propagate the visibility change to children if needed
	if (ensureAlways(TargetComponent.IsValid()))
	{
		TargetComponent->SetVisibility(bNewVisibility, bPropagateVisibilityToChildren);
	}

	// Remember the new visibility state
	bCurrentVisibility = bNewVisibility;
}