// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/PropertyFunctions/CommonPropertyFunctions/TryGetActivitySpotStateTreePropertyFunction.h"

#include "StateTreeExecutionContext.h"
#include "Actors/ActivitySpot.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void FTryGetActivitySpotStateTreePropertyFunction::Execute(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Character);
	check(InstanceData.Character->IsA<AEscapeChroniclesCharacter>());
#endif

	AEscapeChroniclesCharacter* CastedCharacter = CastChecked<AEscapeChroniclesCharacter>(InstanceData.Character);

	const AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		CastedCharacter->GetPlayerState(), ECastCheckedType::NullAllowed);

	// Return the character itself if it doesn't have a valid PlayerState yet
	if (!IsValid(PlayerState))
	{
		InstanceData.OutCharacterOrActivitySpot = CastedCharacter;

		return;
	}

	// TODO: Get the activity spot from the PlayerState
	AActivitySpot* CurrentActivitySpot = /*PlayerState->GetCurrentActivitySpot()*/ nullptr;

	// Return the current activity spot if it is valid
	if (IsValid(CurrentActivitySpot))
	{
		InstanceData.OutCharacterOrActivitySpot = CurrentActivitySpot;
	}
	// Otherwise, return the character itself
	else
	{
		InstanceData.OutCharacterOrActivitySpot = CastedCharacter;
	}
}