// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UAIPerceptionComponent;

struct FAIPerceptionFunctionLibrary
{
	/**
	 * Checks if the given box is visible to the AI perception component.
	 * @param AIPerceptionComponent AI perception component of the agent that is checking for the visibility. It should
	 * have a sight sense configured, or this function will always return false.
	 * @param BoxLocation Location of the box in world space. This is the center of the box.
	 * @param BoxExtent Extent of the box in world space. This is half the size of the box along each axis.
	 * @return True if at least one corner of the box is visible to the AI perception component. False otherwise.
	 * @remark This function only checks the FOV of the sight sense and, of course, the location and the direction of
	 * the agent. But it doesn't check for distance or other parameters. Consider using this function when you already
	 * know the agent sees an actor, but you want to check if a specific point of this actor is visible.
	 */
	static bool IsBoxVisible(const UAIPerceptionComponent* AIPerceptionComponent, const FVector& BoxLocation,
		const FVector& BoxExtent);
};