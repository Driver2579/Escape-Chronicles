// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/GetAllCharactersWithTagsStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

FGetAllCharactersWithTagsStateTreeTask::FGetAllCharactersWithTagsStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetAllCharactersWithTagsStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const UWorld* World = Context.GetWorld();

	if (!ensureAlways(IsValid(World)))
	{
		return EStateTreeRunStatus::Failed;
	}

	AGameStateBase* GameState = World->GetGameState();

	if (!ensureAlways(IsValid(GameState)))
	{
		return EStateTreeRunStatus::Failed;
	}

	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_ENSURE
	ensureAlways(InstanceData.Tag.IsValid());
#endif

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AEscapeChroniclesPlayerState* CastedPlayerState = Cast<AEscapeChroniclesPlayerState>(PlayerState);

		// Check if PlayerState's ASC has the specified tag if the PlayerState is valid
		const bool bPlayerHasTag = IsValid(CastedPlayerState) &&
			CastedPlayerState->GetAbilitySystemComponent()->HasMatchingGameplayTag(InstanceData.Tag);

		if (!bPlayerHasTag)
		{
			continue;
		}

		APawn* Pawn = CastedPlayerState->GetPawn();

		// If the pawn is a valid character, then add it to the output array
		if (IsValid(Pawn) && Pawn->IsA<AEscapeChroniclesCharacter>())
		{
			InstanceData.OutCharacters.Add(Pawn);
		}
	}

	return !InstanceData.OutCharacters.IsEmpty() ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}