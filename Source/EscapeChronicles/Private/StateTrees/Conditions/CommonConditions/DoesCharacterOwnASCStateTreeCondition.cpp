// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Conditions/CommonConditions/DoesCharacterOwnASCStateTreeCondition.h"

#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"
#include "Characters/EscapeChroniclesCharacter.h"

bool FDoesCharacterOwnASCStateTreeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Character);
	check(InstanceData.AbilitySystemComponent);
#endif

	// Check if the character is the AvatarActor of the Ability System Component and invert the result if needed
	return InstanceData.Character == InstanceData.AbilitySystemComponent->GetAvatarActor() ^ bInvert;
}