// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/Structs/FunctionLibriries/AIPerceptionFunctionLibrary.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

bool FAIPerceptionFunctionLibrary::IsBoxVisible(const UAIPerceptionComponent* AIPerceptionComponent,
	const FVector& BoxLocation, const FVector& BoxExtent)
{
#if DO_CHECK
	check(IsValid(AIPerceptionComponent));
#endif

	const UAISenseConfig_Sight* SightSenseConfig = AIPerceptionComponent->GetSenseConfig<UAISenseConfig_Sight>();

	// If the perception component doesn't even have a sense config, then it for sure can't see anything
	if (!IsValid(SightSenseConfig))
	{
		return false;
	}

	FVector AgentLocation;
	FVector AgentDirection;
	AIPerceptionComponent->GetLocationAndDirection(AgentLocation, AgentDirection);

	// Calculate the cosine of half the viewing angle (for comparison via Dot Product)
	const float CosHalfFOV = FMath::Cos(
		FMath::DegreesToRadians(SightSenseConfig->PeripheralVisionAngleDegrees * 0.5f));

	// Get all 8 corners of the box
	TArray<FVector> BoxPoints;
	BoxPoints.Reserve(8);

	// Calculate locations of the points on the diagonal of the box
	const FVector Min = BoxLocation - BoxExtent;
	const FVector Max = BoxLocation + BoxExtent;

	// Combine all possible angles along the axes
	BoxPoints.Add(FVector(Min.X, Min.Y, Min.Z));
	BoxPoints.Add(FVector(Min.X, Min.Y, Max.Z));
	BoxPoints.Add(FVector(Min.X, Max.Y, Min.Z));
	BoxPoints.Add(FVector(Min.X, Max.Y, Max.Z));
	BoxPoints.Add(FVector(Max.X, Min.Y, Min.Z));
	BoxPoints.Add(FVector(Max.X, Min.Y, Max.Z));
	BoxPoints.Add(FVector(Max.X, Max.Y, Min.Z));
	BoxPoints.Add(FVector(Max.X, Max.Y, Max.Z));

	// Check each point whether it's within the FOV
	for (const FVector& Point : BoxPoints)
	{
		const FVector ToPoint = Point - AgentLocation;

		// Check if the point isn't too close to the agent
		if (ToPoint.IsNearlyZero())
		{
			return true;
		}

		const FVector ToPointNormalized = ToPoint.GetSafeNormal();
		const float Dot = FVector::DotProduct(AgentDirection, ToPointNormalized);

		// Check if at least one point is within the FOV
		if (Dot >= CosHalfFOV)
		{
			return true;
		}
	}

	// Return false if no point is within the FOV
	return false;
}