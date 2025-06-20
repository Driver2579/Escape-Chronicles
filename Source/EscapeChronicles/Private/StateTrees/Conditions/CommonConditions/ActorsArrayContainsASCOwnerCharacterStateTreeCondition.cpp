// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/ActorsArrayContainsASCOwnerCharacterStateTreeCondition.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"

bool FActorsArrayContainsASCOwnerCharacterStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	// Return false if the given array is empty. If bInvert is true, then true will be returned.
	if (InstanceData.Array.IsEmpty())
	{
		return bInvert;
	}

#if DO_CHECK
	check(InstanceData.AbilitySystemComponent);
#endif

	const AActor* AvatarActor = InstanceData.AbilitySystemComponent->GetAvatarActor();

	// Return false if the AvatarActor is not a valid character. If bInvert is true, then true will be returned.
	if (!IsValid(AvatarActor) || !AvatarActor->IsA<AEscapeChroniclesCharacter>())
	{
		return bInvert;
	}

	// Check if the given array contains an AvatarActor of the given ASC and invert the result if needed
	return InstanceData.Array.Contains(AvatarActor) ^ bInvert;
}
