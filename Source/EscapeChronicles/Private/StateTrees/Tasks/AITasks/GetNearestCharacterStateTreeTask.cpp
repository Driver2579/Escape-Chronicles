// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/AITasks/GetNearestCharacterStateTreeTask.h"

#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"

FGetNearestCharacterStateTreeTask::FGetNearestCharacterStateTreeTask()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FGetNearestCharacterStateTreeTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Pawn);
#endif

	// Return Failed if there are no characters to search the nearest one in
	if (InstanceData.Characters.IsEmpty())
	{
		return EStateTreeRunStatus::Failed;
	}

	// Cache the pawn location to not get it multiple times
	const FVector PawnLocation = InstanceData.Pawn->GetActorLocation();

	double ShortestDistance = TNumericLimits<double>::Max();

	for (AEscapeChroniclesCharacter* Character : InstanceData.Characters)
	{
		// Skip the actor if it's not valid
		if (!IsValid(Character))
		{
			continue;
		}

		// Find the distance between the controlled pawn and the character
		const double Distance = FVector::Dist(PawnLocation, Character->GetActorLocation());

		// Remember the character if it's closer to the pawn than the previous one
		if (Distance < ShortestDistance)
		{
			ShortestDistance = Distance;
			InstanceData.OutNearestCharacter = Character;
		}
	}

	// Return Succeeded or Failed based on whether we found the nearest character
	return InstanceData.OutNearestCharacter ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}