// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM(BlueprintType)
enum class EDestructiveToolType : uint8
{
	// A tool that is usually used for breaking stone walls
	Pickaxe,

	// A tool that is usually used for cutting fences and prison bars
	WireCutters,

	NumberOfTypes UMETA(Hidden),
	None UMETA(Hidden)
};