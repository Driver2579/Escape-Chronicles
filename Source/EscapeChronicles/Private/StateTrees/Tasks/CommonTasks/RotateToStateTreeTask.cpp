// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Tasks/CommonTasks/RotateToStateTreeTask.h"

#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FRotateToStateTreeTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Interpolate the rotation of the actor towards the target rotation with the specified speed
	const FRotator NewRotation = FMath::RInterpTo(InstanceData.ActorToRotate->GetActorRotation(),
	InstanceData.TargetRotation, DeltaTime, InstanceData.InterpolationSpeed);

#if WITH_EDITORONLY_DATA
	if (InstanceData.bLogRotation)
	{
		UE_LOG(LogTemp, Display,
			TEXT("FRotateToStateTreeTask::Tick: Rotating Actor: %s OldRotation: %s NewRotation: %s "
				"TargetRotation: %s"),
			*InstanceData.ActorToRotate->GetName(),
			*InstanceData.ActorToRotate->GetActorRotation().ToString(),
			*NewRotation.ToString(),
			*InstanceData.TargetRotation.ToString());
	}
#endif

	InstanceData.ActorToRotate->SetActorRotation(NewRotation);

	// Return Succeeded once the new rotation is close enough to the target rotation
	if (NewRotation.Equals(InstanceData.TargetRotation, InstanceData.RotationTolerance))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// Otherwise, return Running to continue the rotation
	return EStateTreeRunStatus::Running;
}