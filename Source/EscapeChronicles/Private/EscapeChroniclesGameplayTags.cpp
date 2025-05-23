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

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Moving, "Status.Movement.Moving",
		"The character is moving (velocity not zero)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Falling, "Status.Movement.Falling", "The character is falling");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_InAir, "Status.Movement.InAir", "The character is in the air");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_NavWalking, "Status.Movement.NavWalking",
		"The character is moving on a navmesh");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_OnGround, "Status.Movement.OnGround",
		"The character is on the ground");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Crouching, "Status.Movement.Crouching",
		"The character is crouching");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Mode_Walking, "Status.Movement.Mode.Walking",
		"The character is in walking mode (low speed, isn't necessarily on the ground, isn't necessarily moving)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Mode_Jogging, "Status.Movement.Mode.Jogging",
		"The character is in jogging mode (medium speed, isn't necessarily on the ground, isn't necessarily moving)");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Movement_Mode_Running, "Status.Movement.Mode.Running",
		"The character is in running mode (high speed, isn't necessarily on the ground, isn't necessarily moving)");

	// === Roles tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_Prisoner, "Role.Prisoner", "The character is a prisoner");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_Guard, "Role.Guard", "The character is a guard");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_Medic, "Role.Medic", "The character is a medic");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_NPC, "Role.NPC", "The character is controlled by an AI");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Role_Player, "Role.Player", "The character is controlled by a real player");

	// === Schedule events tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ScheduleEvent_Bedtime, "ScheduleEvent.Bedtime", "Bedtime event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ScheduleEvent_RollCall, "ScheduleEvent.RollCall", "Roll Call event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ScheduleEvent_Meal, "ScheduleEvent.Meal", "Meal event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ScheduleEvent_Shower, "ScheduleEvent.Shower", "Shower event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ScheduleEvent_Break, "ScheduleEvent.Break", "Break event");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(ScheduleEvent_Alert, "ScheduleEvent.Alert", "Alert event");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_CheckedIn, "Status.CheckedIn",
		"The character has checked in during the scheduled event");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Wanted, "Status.Wanted",
		"The character is wanted during the alert. Keep in mind that this tag may still be present even if alert is "
			"paused.");

	// === Other Status tags ===

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_InForbiddenZone, "Status.InForbiddenZone",
		"The prisoner is in a forbidden zone");
}