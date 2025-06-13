// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/GeometryFramework/Public/Components/DynamicMeshComponent.h"
#include "Interfaces/Saveable.h"
#include "DestructibleMeshComponent.generated.h"

class UDynamicMesh;

/**
 * A mesh that supports making holes in it by subtracting a tool mesh from it. This component also supports
 * saving/loading the holes when the game is saved/loaded.
 */
UCLASS(ClassGroup=(Rendering, Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UDestructibleMeshComponent : public UDynamicMeshComponent, public ISaveable
{
	GENERATED_BODY()

public:
	// TODO: We should use AddHoleAtWorldLocation function with an ImpactPoint from the hit result

	// Adds a hole at the given world location converting to a relative location of the mesh
	void AddHoleAtWorldLocation(const FVector& HoleWorldLocation);

	// Adds a hole at the given relative location of the mesh
	void AddHoleAtRelativeLocation(const FVector& HoleRelativeLocation);

	// Clears all holes in this mesh that were previously created
	void ClearAllHoles();

	virtual void OnPreLoadObject() override;
	virtual void OnPostLoadObject() override;

private:
	// A mesh will be used to create holes. Each hole will completely duplicate the form of this mesh.
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDynamicMesh> DestructToolMesh;

	/**
	 * An actual implementation of the AddHoleAtRelativeLocation function. Doesn't add the location to the
	 * HolesLocations array.
	 */
	void AddHoleAtRelativeLocation_Internal(const FVector& Location);

	/**
	 * Removes an existing hole at the given relative location.
	 * @reamrk You should call this function only for locations that were previously used to create holes.
	 * @remark This function doesn't remove the given location from the HolesLocations array.
	 */
	void RemoveHoleAtRelativeLocation(const FVector& HoleRelativeLocation);

	// A list of locations that were used to create holes in the mesh
	UPROPERTY(Transient, SaveGame)
	TArray<FVector> HolesLocations;
};