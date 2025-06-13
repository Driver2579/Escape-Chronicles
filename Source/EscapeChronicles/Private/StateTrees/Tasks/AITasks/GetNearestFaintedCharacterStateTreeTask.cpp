// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/GetNearestFaintedCharacterStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

FGetNearestFaintedCharacterStateTreeTask::FGetNearestFaintedCharacterStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetNearestFaintedCharacterStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Pawn);

	check(IsValid(Context.GetWorld()));
	check(IsValid(Context.GetWorld()->GetGameState()));
#endif

	// Cache the pawn location to not get it multiple times
	const FVector PawnLocation = InstanceData.Pawn->GetActorLocation();

	double ShortestDistance = TNumericLimits<double>::Max();

	for (const APlayerState* PlayerState : Context.GetWorld()->GetGameState()->PlayerArray)
	{
#if DO_CHECK
		check(IsValid(PlayerState));
#endif

		AEscapeChroniclesCharacter* Character = PlayerState->GetPawn<AEscapeChroniclesCharacter>();

		// Skip the PlayerState if it doesn't have a valid character or if this character isn't fainted
		if (!IsValid(Character) || !Character->IsFainted())
		{
			continue;
		}

		// Find the distance between the controlled pawn and the character
		const double Distance = FVector::Dist(PawnLocation, Character->GetActorLocation());

		// Remember the character if it's closer to the pawn than the previous one
		if (Distance < ShortestDistance)
		{
			ShortestDistance = Distance;
			InstanceData.OutNearestFaintedCharacter = Character;
		}
	}

	// Return Succeeded or Failed based on whether we found the nearest fainted character
	return InstanceData.OutNearestFaintedCharacter ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}