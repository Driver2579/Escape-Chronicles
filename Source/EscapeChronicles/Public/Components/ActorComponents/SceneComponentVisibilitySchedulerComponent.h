// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Structs/GameplayDateTime.h"
#include "SceneComponentVisibilitySchedulerComponent.generated.h"

// Changes the visibility of the selected scene component based on the current game time
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API USceneComponentVisibilitySchedulerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USceneComponentVisibilitySchedulerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnCurrentGameDateTimeUpdated(const FGameplayDateTime& OldDateTime,
		const FGameplayDateTime& NewDateTime);

private:
	// The time when the component should start being visible
	UPROPERTY(EditAnywhere)
	FGameplayTime ComponentVisibleStartTime;

	// The time when the component should end being visible
	UPROPERTY(EditAnywhere)
	FGameplayTime ComponentVisibleEndTime;

	// Component to change the visibility based on the current game time on
	UPROPERTY(EditAnywhere, DisplayName="Target Component", meta=(UseComponentPicker, AllowedClasses="SceneComponent"))
	FComponentReference TargetComponentReference;

	// An actual pointer to the target component. It is set in BeginPlay() based on TargetComponentReference.
	TWeakObjectPtr<USceneComponent> TargetComponent;

	// If true, the visibility change will be propagated to all children of the target component
	UPROPERTY(EditAnywhere)
	bool bPropagateVisibilityToChildren = false;

	/**
	 * Updates the visibility of the target component based on the current game time.
	 * @param CurrentGameTime Current game time to check the visibility against.
	 * @param bForceUpdate If true, forces the visibility update even if the current visibility is the same as the new
	 * visibility. Must be used for initial visibility update.
	 */
	void UpdateComponentVisibility(const FGameplayTime& CurrentGameTime, const bool bForceUpdate = false);

	// Current visibility of the component. Used to avoid unnecessary visibility changes.
	bool bCurrentVisibility = false;
};