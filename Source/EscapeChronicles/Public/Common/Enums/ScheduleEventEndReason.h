// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

UENUM()
enum class EScheduleEventEndReason : uint8
{
	// The event was ended normally
	Normal,

	// The event was ended due to the game being loaded
	Loading,

	// The event was ended due to the EndPlay being called on owning UScheduleEventManagerComponent
	EndPlay
};