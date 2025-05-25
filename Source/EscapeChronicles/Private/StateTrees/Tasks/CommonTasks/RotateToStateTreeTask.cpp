// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/RotateToStateTreeTask.h"

#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FRotateToStateTreeTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Interpolate the rotation of the actor towards the target rotation with the specified speed
	const FRotator NewRotation = FMath::RInterpTo(InstanceData.ActorToRotate->GetActorRotation(),
		InstanceData.TargetRotation, DeltaTime, InstanceData.InterpolationSpeed);

	InstanceData.ActorToRotate->SetActorRotation(NewRotation);

	// Return Succeeded once the new rotation is close enough to the target rotation
	if (NewRotation.Equals(InstanceData.TargetRotation))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// Otherwise, return Running to continue the rotation
	return EStateTreeRunStatus::Running;
}