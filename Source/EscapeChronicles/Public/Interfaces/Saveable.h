// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Saveable.generated.h"

class UEscapeChroniclesSaveGame;

UINTERFACE()
class USaveable : public UInterface
{
	GENERATED_BODY()
};

class ISaveable
{
	GENERATED_BODY()

public:
	/**
	 * Indicates if the object can be saved or loaded now. Can be used to skip saving if the object is not in a valid
	 * state (for example, when it isn't fully initialized yet).
	 */
	virtual bool CanBeSavedOrLoaded() const { return true; }

	/**
	 * This is called right before saving an object implementing this interface. This is the place where you should
	 * update all of your properties that are marked with "SaveGame". 
	 */
	virtual void OnPreSaveObject() {}

	// This is called right after the game is saved
	virtual void OnGameSaved() {}

	// This is called right before loading an object implementing this interface
	virtual void OnPreLoadObject() {}

	/**
	 * This is called right after loading an object implementing this interface. This is the place where you should read
	 * all of your properties that are marked with "SaveGame".
	 */
	virtual void OnPostLoadObject() {}
};