// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/MeshComponents/DynamicMeshComponents/DestructibleMeshComponent.h"

#include "GeometryScript/MeshBooleanFunctions.h"

void UDestructibleMeshComponent::AddHoleAtWorldLocation(const FVector& HoleWorldLocation)
{
	// Convert the world location into a relative location of this component
	const FVector HoleRelativeLocation = GetComponentTransform().InverseTransformPosition(HoleWorldLocation);

	// Add a hole at the calculated location
	AddHoleAtRelativeLocation(HoleRelativeLocation);
}

void UDestructibleMeshComponent::AddHoleAtRelativeLocation(const FVector& HoleRelativeLocation)
{
	AddHoleAtRelativeLocation_Internal(HoleRelativeLocation);

	// Remember that we used this location to create a hole
	HolesLocations.Add(HoleRelativeLocation);
}

void UDestructibleMeshComponent::AddHoleAtRelativeLocation_Internal(const FVector& Location)
{
#if DO_ENSURE
	ensureAlways(DestructToolMesh);
#endif

	// Convert the location into a transform
	FTransform ToolTransform;
	ToolTransform.SetLocation(Location);

	// Apply the mesh boolean operation to subtract the DestructToolMesh from the current mesh
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean(GetDynamicMesh(),
		GetComponentTransform(), DestructToolMesh, ToolTransform,
		EGeometryScriptBooleanOperation::Subtract, FGeometryScriptMeshBooleanOptions());
}

void UDestructibleMeshComponent::ClearAllHoles()
{
	// Remove all holes at the locations that were used to create them
	for (const FVector& HoleLocation : HolesLocations)
	{
		RemoveHoleAtRelativeLocation(HoleLocation);
	}

	// Clear the list of holes locations because we filled all of them back
	HolesLocations.Empty();
}

void UDestructibleMeshComponent::RemoveHoleAtRelativeLocation(const FVector& HoleRelativeLocation)
{
#if DO_ENSURE
	ensureAlways(HolesLocations.Contains(HoleRelativeLocation));
	ensureAlways(DestructToolMesh);
#endif

	// Convert the location into a transform
	FTransform ToolTransform;
	ToolTransform.SetLocation(HoleRelativeLocation);

	/**
	 * Apply the mesh boolean operation to add the DestructToolMesh to the current mesh. We basically reverse the
	 * subtraction operation.
	 */
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean(GetDynamicMesh(),
		GetComponentTransform(), DestructToolMesh, ToolTransform,
		EGeometryScriptBooleanOperation::Union, FGeometryScriptMeshBooleanOptions());
}

void UDestructibleMeshComponent::OnPreLoadObject()
{
	// Clear the holes before loading the object to apply the loaded ones instead
	ClearAllHoles();
}

void UDestructibleMeshComponent::OnPostLoadObject()
{
	// Add holes at all the locations that were saved in the loaded HolesLocations array
	for (const FVector& Location : HolesLocations)
	{
		AddHoleAtRelativeLocation(Location);
	}
}