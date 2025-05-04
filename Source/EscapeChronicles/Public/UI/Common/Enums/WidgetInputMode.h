// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Interpreting possible widget input modes as Enum
UENUM(BlueprintType)
enum class EWidgetInputMode : uint8 {
	GameOnly,
	UiOnly,
	GameAndUi,
};