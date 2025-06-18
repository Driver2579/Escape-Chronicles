// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/DestructibleComponent.h"

struct FEnvironmentInteractionEvents
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDestructibleDamagedDelegate, AEscapeChroniclesCharacter* Character,
		UDestructibleComponent* DestructibleComponent);

	static FOnDestructibleDamagedDelegate OnDestructibleDamaged;
};
