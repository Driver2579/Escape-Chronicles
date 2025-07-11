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

#if DO_ENSURE
	ensureAlwaysMsgf(!TargetComponentsReferences.IsEmpty(),
		TEXT("No target components are set for visibility scheduling, this component will work for nothing! "
			"Consider removing this component or setting the target components."));
#endif

	// Reserve space for the target components based on the references set in the editor
	TargetComponents.Reserve(TargetComponentsReferences.Num());

	// Cache the pointers to the target components based on the references set in the editor
	for (const FComponentReference& TargetComponentReference : TargetComponentsReferences)
	{
		USceneComponent* TargetComponent = CastChecked<USceneComponent>(
			TargetComponentReference.GetComponent(GetOwner()), ECastCheckedType::NullAllowed);

		if (ensureAlways(IsValid(TargetComponent)))
		{
			TargetComponents.Add(TargetComponent);
		}
	}

	// Initialize the visibility of the components based on the current game time
	UpdateComponentsVisibility(GameState->GetCurrentGameDateTime().Time, true);

	// Listen for game date time changes
	GameState->OnCurrentGameDateTimeUpdated.AddUObject(this, &ThisClass::OnCurrentGameDateTimeUpdated);
}

void USceneComponentVisibilitySchedulerComponent::OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldDateTime,
	const FGameplayDateTime& NewDateTime)
{
	// Update the visibility of the components based on the new game time
	UpdateComponentsVisibility(NewDateTime.Time);
}

void USceneComponentVisibilitySchedulerComponent::UpdateComponentsVisibility(
	const FGameplayTime& CurrentGameTime, const bool bForceUpdate)
{
	// Whether the components should be visible. Depends on the current game time.
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

	// Update the visibility of the target components and propagate the visibility changes to children if needed
	for (TWeakObjectPtr TargetComponent : TargetComponents)
	{
		if (ensureAlways(TargetComponent.IsValid()))
		{
			TargetComponent->SetVisibility(bNewVisibility, bPropagateVisibilityToChildren);
		}
	}

	// Remember the new visibility state
	bCurrentVisibility = bNewVisibility;
}