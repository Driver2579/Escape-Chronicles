// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/Structs/FunctionLibriries/SharedRelationshipAttributeSetFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/SharedRelationshipAttributeSet.h"
#include "Characters/EscapeChroniclesCharacter.h"

bool FSharedRelationshipAttributeSetFunctionLibrary::IsCharacterSuspicious(const AEscapeChroniclesCharacter* Character,
	const float MinSuspiciousValue)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	const UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

	// Check if the character has a valid AbilitySystemComponent
	if (!IsValid(AbilitySystemComponent))
	{
		return false;
	}

	// Find the SharedRelationshipAttributeSet from the AbilitySystemComponent
	const USharedRelationshipAttributeSet* SharedRelationshipAttributeSet =
		CastChecked<USharedRelationshipAttributeSet>(
			AbilitySystemComponent->GetAttributeSet(USharedRelationshipAttributeSet::StaticClass()),
			ECastCheckedType::NullAllowed);

	// Check if we found a valid SharedRelationshipAttributeSet
	if (!IsValid(SharedRelationshipAttributeSet))
	{
		return false;
	}

	const float Suspicion = SharedRelationshipAttributeSet->GetSuspicion();

	/**
	 * Return true if character's suspicion value is greater or equal to the minimum required suspicion value or if it's
	 * already at the maximum suspicion value.
	 */
	return Suspicion >= MinSuspiciousValue || Suspicion == SharedRelationshipAttributeSet->GetMaxSuspicion();
}

void FSharedRelationshipAttributeSetFunctionLibrary::FindSuspiciousCharactersInActors(const TArray<AActor*>& Actors,
	TArray<AEscapeChroniclesCharacter*>& OutSuspiciousCharacters, const float MinSuspiciousValue)
{
	OutSuspiciousCharacters.Empty();

	for (AActor* Actor : Actors)
	{
		AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(Actor);

		// Add an actor to the output array if it's a valid suspicious character
		if (IsValid(Character) && IsCharacterSuspicious(Character, MinSuspiciousValue))
		{
			OutSuspiciousCharacters.Add(Character);
		}
	}
}

AEscapeChroniclesCharacter* FSharedRelationshipAttributeSetFunctionLibrary::FindSuspiciousCharacterInActor(
	const TArray<AActor*>& Actors, const float MinSuspiciousValue)
{
	for (AActor* Actor : Actors)
	{
		AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(Actor);

		// Return an actor if it's a valid suspicious character
		if (IsValid(Character) && IsCharacterSuspicious(Character, MinSuspiciousValue))
		{
			return Character;
		}
	}

	// Return nullptr if no suspicious character was found
	return nullptr;
}