// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChroniclesGameplayTags.h"

namespace EscapeChroniclesGameplayTags
{
	// === Input tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look, "InputTag.Look", "Look input");

	// === Movement tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Falling, "Status.Movement.Falling", "The character is falling");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_InAir, "Status.Movement.InAir", "The character is in the air");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_NavWalking, "Status.Movement.NavWalking",
		"The character is moving on a navmesh");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_OnGround, "Status.Movement.OnGround",
		"The character is on the ground");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Walking, "Status.Movement.Walking",
		"The character is walking (low speed)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Jogging, "Status.Movement.Jogging",
		"The character is jogging (medium speed)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Running, "Status.Movement.Running",
		"The character is running (high speed)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Crouching, "Status.Movement.Crouching",
		"The character is crouching");
}