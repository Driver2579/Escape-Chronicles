// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/NavMoverComponents/EscapeChroniclesNavMoverComponent.h"

UEscapeChroniclesNavMoverComponent::UEscapeChroniclesNavMoverComponent()
{
	AvoidanceGroup.bGroup0 = true;

	GroupsToAvoid.Packed = 0xFFFFFFFF;
	GroupsToIgnore.Packed = 0;
}

float UEscapeChroniclesNavMoverComponent::GetRVOAvoidanceRadius()
{
	float CapsuleRadius;
	float CapsuleHalfHeight;
	GetSimpleCollisionCylinder(CapsuleRadius, CapsuleHalfHeight);

	return CapsuleRadius;
}

float UEscapeChroniclesNavMoverComponent::GetRVOAvoidanceHeight()
{
	float CapsuleRadius;
	float CapsuleHalfHeight;
	GetSimpleCollisionCylinder(CapsuleRadius, CapsuleHalfHeight);

	return CapsuleHalfHeight;
}