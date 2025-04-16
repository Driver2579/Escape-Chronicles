// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeChroniclesGameplayTags.h"

namespace EscapeChroniclesGameplayTags
{
	// === Input tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look, "InputTag.Look", "Look input");

	// === Extended Mover tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mover_IsWalkGroundSpeedModeActive, "Mover.IsWalkGroundSpeedModeActive",
		"Mover state flag indicating character is walking (could be set even if walking movement mode isn't active).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mover_IsJogGroundSpeedModeActive, "Mover.IsJogGroundSpeedModeActive",
		"Mover state flag indicating character is jogging (could be set even if walking movement mode isn't active).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mover_IsRunGroundSpeedModeActive, "Mover.IsRunGroundSpeedModeActive",
		"Mover state flag indicating character is running (could be set even if walking movement mode isn't active).");

	// === Movement tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Falling, "Status.Movement.Falling", "The character is falling");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_InAir, "Status.Movement.InAir", "The character is in the air");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_NavWalking, "Status.Movement.NavWalking",
		"The character is moving on a navmesh");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_OnGround, "Status.Movement.OnGround",
		"The character is on the ground");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Walking, "Status.Movement.Walking",
		"The character is walking (low speed, isn't necessarily on the ground)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Jogging, "Status.Movement.Jogging",
		"The character is jogging (medium speed, isn't necessarily on the ground)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Running, "Status.Movement.Running",
		"The character is running (high speed, isn't necessarily on the ground)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Crouching, "Status.Movement.Crouching",
		"The character is crouching");

	// === Roles tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_Prisoner, "Role.Prisoner", "The character is a prisoner");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_Guard, "Role.Guard", "The character is a guard");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_NPC, "Role.NPC", "The character is controlled by an AI");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_Player, "Role.Player", "The character is controlled by a real player");
}