// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class EControlledCharacterType : uint8
{
	// Character is controlled by an AI
	Bot,

	// Character is controlled by a human player
	RealPlayer,

	NumberOfTypes UMETA(Hidden)
};