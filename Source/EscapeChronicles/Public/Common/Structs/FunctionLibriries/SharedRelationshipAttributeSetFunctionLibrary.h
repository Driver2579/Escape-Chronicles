// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class USharedRelationshipAttributeSet;
class AEscapeChroniclesCharacter;

struct FSharedRelationshipAttributeSetFunctionLibrary
{
	/**
	 * Checks if the given character is suspicious based on its suspicion attribute value.
	 * @param Character The character to check for suspicion.
	 * @param MinSuspiciousValue A minimum suspicious value to consider a character as suspicious. If it's greater than
	 * the maximum suspicion value, then the maximum value will be used.
	 */
	static bool IsCharacterSuspicious(const AEscapeChroniclesCharacter* Character, const float MinSuspiciousValue);

	/**
	 * Finds suspicious characters in the given array of actors based on their suspicion attribute value.
	 * @param Actors An array of actors to find suspicious characters in.
	 * @param OutSuspiciousCharacters An output array that will contain the found suspicious characters.
	 * @param MinSuspiciousValue A minimum suspicious value to consider a character as suspicious. If it's greater than
	 * the maximum suspicion value, then the maximum value will be used.
	 */
	static void FindSuspiciousCharactersInActors(const TArray<AActor*>& Actors,
		TArray<AEscapeChroniclesCharacter*>& OutSuspiciousCharacters, const float MinSuspiciousValue);

	/**
	 * Finds the first suspicious character in the given array of actors based on their suspicion attribute value.
	 * @param Actors An array of actors to find suspicious character in.
	 * @param MinSuspiciousValue A minimum suspicious value to consider a character as suspicious. If it's greater than
	 * the maximum suspicion value, then the maximum value will be used.
	 */
	static AEscapeChroniclesCharacter* FindSuspiciousCharacterInActor(const TArray<AActor*>& Actors,
		const float MinSuspiciousValue);
};