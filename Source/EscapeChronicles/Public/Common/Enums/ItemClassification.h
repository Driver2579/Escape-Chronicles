// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class EItemClassification : uint8
{
	// A usual standard type of item
	Normal,

	// An item that is considered to be of high value and has a risk of being lost when the character is fainted
	Contraband,

	NumberOfTypes UMETA(Hidden),
	None UMETA(Hidden)
};