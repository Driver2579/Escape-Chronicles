// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class AEscapeChroniclesCharacter;

struct FCharacterStatusEvents
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCharacterStatusChagnedDelegate, AEscapeChroniclesCharacter* Character,
		const bool bNewStatus);

	// Called when the character is fainted or revived
	static FOnCharacterStatusChagnedDelegate OnFaintedStatusChanged;
};