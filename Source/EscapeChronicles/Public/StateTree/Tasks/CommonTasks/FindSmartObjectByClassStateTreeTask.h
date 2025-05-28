// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "SmartObjectRequestTypes.h"
#include "FindSmartObjectByClassStateTreeTask.generated.h"

UENUM(BlueprintType)
enum class EFindSmartObjectMethod : uint8
{
	// Returns the first found smart object
	First,

	// Returns the nearest smart object to the UserActor (without checking the path)
	Nearest,

	/**
	 * Returns the nearest smart object to the UserActor that is within an acceptable radius. If no such smart object is
	 * found, returns the first one.
	 */
	NearestInAcceptableRadius
};

USTRUCT()
struct FFindSmartObjectByClassStateTreeTaskInstanceData
{
	GENERATED_BODY()

	// Class of actors to find a smart object in
	UPROPERTY(EditAnywhere, Category="Parameter")
	TSubclassOf<AActor> ActorClass;

	// Filter to use when searching for smart objects
	UPROPERTY(EditAnywhere, Category="Parameter")
	FSmartObjectRequestFilter SmartObjectRequestFilter;

	UPROPERTY(EditAnywhere, Category="Parameter")
	EFindSmartObjectMethod FindSmartObjectMethod = EFindSmartObjectMethod::First;

	UPROPERTY(EditAnywhere, Category="Parameter",
		meta=(ClampMin=0.1, EditCondition="FindSmartObjectMethod == EFindSmartObjectMethod::NearestInAcceptableRadius"))
	float AcceptableRadius = 200;

	// An actor that is going to use the smart object
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<AActor> UserActor;

	// A found smart object
	UPROPERTY(EditAnywhere, Category="Output")
	FSmartObjectRequestResult OutSmartObjectRequestResult;
};

// Finds all actors of a specified class and returns the first or nearest unclaimed smart object in found actors
USTRUCT(Category="Smart Objects", meta=(DisplayName="Find Smart Object by Class"))
struct FFindSmartObjectByClassStateTreeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FFindSmartObjectByClassStateTreeTask();

	using FInstanceDataType = FFindSmartObjectByClassStateTreeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};