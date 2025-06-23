// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/NavMoverComponent.h"
#include "AI/RVOAvoidanceInterface.h"
#include "AI/Navigation/NavigationAvoidanceTypes.h"
#include "EscapeChroniclesNavMoverComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UEscapeChroniclesNavMoverComponent : public UNavMoverComponent, public IRVOAvoidanceInterface
{
	GENERATED_BODY()

public:
	UEscapeChroniclesNavMoverComponent();

	// ~IRVOAvoidanceInterface
	virtual int32 GetRVOAvoidanceUID() override { return AvoidanceUID; }
	virtual void SetRVOAvoidanceUID(int32 UID) override { AvoidanceUID = UID; }

	virtual float GetRVOAvoidanceWeight() override { return AvoidanceWeight; }
	virtual void SetRVOAvoidanceWeight(float Weight) override { AvoidanceWeight = Weight; }

	virtual FVector GetRVOAvoidanceOrigin() override { return GetFeetLocation(); }

	virtual float GetRVOAvoidanceRadius() override;
	virtual float GetRVOAvoidanceHeight() override;

	virtual float GetRVOAvoidanceConsiderationRadius() override { return AvoidanceConsiderationRadius; }
	virtual FVector GetVelocityForRVOConsideration() override { return GetVelocityForNavMovement(); }

	virtual int32 GetAvoidanceGroupMask() override { return AvoidanceGroup.Packed; }
	virtual void SetAvoidanceGroupMask(int32 GroupFlags) override { AvoidanceGroup.SetFlagsDirectly(GroupFlags); }

	virtual int32 GetGroupsToAvoidMask() override { return GroupsToAvoid.Packed; }
	virtual void SetGroupsToAvoidMask(int32 GroupFlags) override { GroupsToAvoid.SetFlagsDirectly(GroupFlags); }

	virtual int32 GetGroupsToIgnoreMask() override { return GroupsToIgnore.Packed; }
	virtual void SetGroupsToIgnoreMask(int32 GroupFlags) override { GroupsToIgnore.SetFlagsDirectly(GroupFlags); }
	// ~End IRVOAvoidanceInterface

private:
	UPROPERTY(EditAnywhere, Category="Avoidance", meta=(ForceUnits="cm"))
	float AvoidanceConsiderationRadius = 500;

	UPROPERTY(VisibleAnywhere, Category="Avoidance", AdvancedDisplay)
	int32 AvoidanceUID = 0;

	// Moving actor's group mask
	UPROPERTY(EditAnywhere, Category="Avoidance", AdvancedDisplay)
	FNavAvoidanceMask AvoidanceGroup;

	// Will avoid other agents if they are in one of specified groups
	UPROPERTY(EditAnywhere, Category="Avoidance", AdvancedDisplay)
	FNavAvoidanceMask GroupsToAvoid;

	// Will NOT avoid other agents if they are in one of specified groups, higher priority than GroupsToAvoid
	UPROPERTY(EditAnywhere, Category="Avoidance", AdvancedDisplay)
	FNavAvoidanceMask GroupsToIgnore;

	UPROPERTY(EditAnywhere, Category="Avoidance")
	float AvoidanceWeight = 0.5;
};