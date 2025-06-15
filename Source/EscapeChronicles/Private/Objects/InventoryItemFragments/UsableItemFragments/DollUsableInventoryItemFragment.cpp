// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemFragments/UsableItemFragments/DollUsableInventoryItemFragment.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Actors/ActivitySpot.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"

bool UDollUsableInventoryItemFragment::OnTriedToUseItem(UInventoryManagerComponent* InventoryManager,
	UInventoryItemInstance* ItemInstance, const FGameplayTag& SlotTypeTag, const int32 SlotIndex) const
{
#if DO_CHECK
	check(IsValid(InventoryManager));
#endif

	const AEscapeChroniclesCharacter* OwningCharacter = InventoryManager->GetOwner<AEscapeChroniclesCharacter>();

	if (!IsValid(OwningCharacter))
	{
#if !NO_LOGGING
		UE_LOG(LogTemp, Warning,
			TEXT("UDollUsableInventoryItemFragment::OnTriedToUseItem: This item can only be used when it is "
				"owned by the character!"));
#endif

		return false;
	}

	const UInteractableComponent* SelectedInteractableComponent =
		OwningCharacter->GetInteractionManagerComponent()->GetSelectedInteractableComponent();

	// Check if we have any selected interactable component
	if (!IsValid(SelectedInteractableComponent))
	{
		return false;
	}

	AActivitySpot* SelectedActivitySpot = SelectedInteractableComponent->GetOwner<AActivitySpot>();

	// Check if the selected actor is a valid activity spot
	if (!IsValid(SelectedActivitySpot))
	{
		return false;
	}

	// Check if the selected activity spot is an allowed activity spot class if any are specified
	if (!AllowedActivitySpotClasses.IsEmpty())
	{
		bool bSelectedActivitySpotIsAllowed = false;

		for (const TSoftClassPtr<AActivitySpot>& AllowedActivitySpotClass : AllowedActivitySpotClasses)
		{
			/**
			 * If the activity spot class is invalid, then it's for sure isn't the one the selected activity spot is
			 * because it isn't even loaded, which means it for sure isn't even currently in the world. Otherwise, if it
			 * is valid, then check if the selected activity spot is a child of the allowed activity spot class.
			 */
			bSelectedActivitySpotIsAllowed = AllowedActivitySpotClass.IsValid() &&
				SelectedActivitySpot->GetClass()->IsChildOf(AllowedActivitySpotClass.Get());

			// If the selected activity spot is allowed, then we can stop searching
			if (bSelectedActivitySpotIsAllowed)
			{
				break;
			}
		}

		// Return false if the selected activity spot is not allowed
		if (!bSelectedActivitySpotIsAllowed)
		{
			return false;
		}
	}

#if DO_CHECK
	check(IsValid(ItemInstance));
#endif

	// Spawn the doll with the specified transform if it was requested and return the result
	if (bUseCustomSpawnTransform)
	{
		return SelectedActivitySpot->SpawnOccupyingDoll(DollActorClass, &DollActorSpawnTransform);
	}

	// Otherwise, spawn the doll with a default transform and return the result
	return SelectedActivitySpot->SpawnOccupyingDoll(DollActorClass);
}
