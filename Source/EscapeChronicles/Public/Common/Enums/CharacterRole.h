// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class ECharacterRole : uint8
{
	Prisoner,
	Guard,
	Medic,

	NumberOfTypes UMETA(Hidden),
	None UMETA(Hidden)
};