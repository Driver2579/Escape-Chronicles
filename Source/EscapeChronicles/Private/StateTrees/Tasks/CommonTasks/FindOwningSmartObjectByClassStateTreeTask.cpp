// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/FindOwningSmartObjectByClassStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void FFindOwningSmartObjectByClassStateTreeTask::FilterFoundActors(FStateTreeExecutionContext& Context,
	TArray<AActor*>& InOutFoundActors) const
{
	FFindSmartObjectByClassStateTreeTask::FilterFoundActors(Context, InOutFoundActors);

	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	const AEscapeChroniclesCharacter* UserCharacter = Cast<AEscapeChroniclesCharacter>(InstanceData.UserActor);

	// The UserActor must be a valid character
	if (!ensureAlways(IsValid(UserCharacter)))
	{
		InOutFoundActors.Empty();

		return;
	}

	const AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		UserCharacter->GetPlayerState(), ECastCheckedType::NullAllowed);

	// The character definitely doesn't own anything if his PlayerState is invalid yet
	if (!IsValid(PlayerState))
	{
		InOutFoundActors.Empty();

		return;
	}

	const FUniquePlayerID& UniquePlayerID = PlayerState->GetUniquePlayerID();

	// The character definitely doesn't own anything if his UniquePlayerID is invalid yet
	if (!UniquePlayerID.IsValid())
	{
		InOutFoundActors.Empty();

		return;
	}

	for (int32 i = InOutFoundActors.Num() - 1; i >= 0; --i)
	{
		const AActor* FoundActor = InOutFoundActors[i];

#if DO_CHECK
		check(IsValid(FoundActor));
#endif

		const UPlayerOwnershipComponent* FoundActorPlayerOwnershipComponent = FoundActor->FindComponentByClass<
			UPlayerOwnershipComponent>();

		/**
		 * The character definitely doesn't own this actor if it doesn't have a PlayerOwnershipComponent. Remove the
		 * found actor from the array in this case.
		 */
		if (!IsValid(FoundActorPlayerOwnershipComponent))
		{
			InOutFoundActors.RemoveAt(i, EAllowShrinking::No);

			continue;
		}

		const FUniquePlayerID* FoundActorOwnerID = FoundActorPlayerOwnershipComponent->GetOwningPlayer();

		/**
		 * Remove the found actor from the array if it doesn't have an owning player or if UniquePlayerID of the owning
		 * player isn't the same as the UniquePlayerID of the character.
		 */
		if (!FoundActorOwnerID || *FoundActorOwnerID != UniquePlayerID)
		{
			InOutFoundActors.RemoveAt(i, EAllowShrinking::No);
		}
	}
}