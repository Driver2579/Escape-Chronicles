// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Saveable.h"
#include "DestructibleComponent.generated.h"

class ADynamicMeshActor;
class UDynamicMeshComponent;
class UDynamicMesh;

// A structure that holds the data about a hole in the mesh. It supports being saved and loaded.
USTRUCT()
struct FDynamicMeshHoleData
{
	GENERATED_BODY()

	// The relative location of the hole in the mesh
	UPROPERTY(Transient, SaveGame)
	FVector RelativeLocation;

	// The radius of the hole in the mesh
	UPROPERTY(Transient, SaveGame)
	float Radius;

	bool operator==(const FDynamicMeshHoleData& Other) const
	{
		return Radius == Other.Radius && RelativeLocation == Other.RelativeLocation;
	}
};

/**
 * A component that supports making holes in the DynamicMeshActor this component is added to by subtracting a sphere
 * with a specified radius from it in a specified place. This component also supports saving/loading the holes when the
 * game is saved/loaded.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UDestructibleComponent : public UActorComponent, public ISaveable
{
	GENERATED_BODY()

public:
	UDestructibleComponent();

	// Adds a hole of the given radius at the given world location converting to a relative location of the mesh
	void AddHoleAtWorldLocation(const FVector& HoleWorldLocation, const float HoleRadius);

	// Adds a hole of the given radius at the given relative location of the mesh
	void AddHoleAtRelativeLocation(const FVector& HoleRelativeLocation, const float HoleRadius);

	// Clears all holes in this mesh that were previously created
	void ClearAllHoles();

	virtual void OnPreLoadObject() override;
	virtual void OnPostLoadObject() override;

protected:
	virtual void BeginPlay() override;

private:
	// A pointer to the dynamic mesh actor that owns this component
	TWeakObjectPtr<ADynamicMeshActor> OwningDynamicMeshActor;

	// Whether the component should automatically make the actor saveable at the beginning of the game
	UPROPERTY(EditAnywhere, Category="Save Game")
	bool bAutomaticallyMakeActorSaveable = true;

	/**
	 * The tag that will be added to the actor when this component calls BeginPlay. This tag should be the same as the
	 * one in the SaveGameSubsystem to mark this actor as saveable.
	 */
	UPROPERTY(EditAnywhere, Category="Save Game", meta=(EditCondition="bAutomaticallyMakeActorSaveable"))
	FName SaveableActorTag = TEXT("Saveable");

	/**
	 * Allocates the tool mesh that will be used to create holes in the mesh. The mesh is a sphere with the specified
	 * radius and the specified relative location.
	 * @remark The OwningDynamicMeshActor must be valid when calling this function.
	 */
	UDynamicMesh* AllocateDestructToolMeshChecked(const FVector& HoleRelativeLocation, const float Radius) const;

	/**
	 * An actual implementation of the AddHoleAtRelativeLocation function. Doesn't add the data to the Holes array and
	 * doesn't check if the OwningDynamicMeshActor is valid.
	 */
	void AddHoleAtRelativeLocation_Internal(const FVector& HoleRelativeLocation, const float HoleRadius) const;

	/**
	 * Removes an existing hole at the given relative location.
	 * @remark You should call this function only for locations that were previously used to create holes.
	 * @remark This function doesn't remove the given location from the HolesLocations array.
	 * @remark The OwningDynamicMeshActor must be valid when calling this function.
	 */
	void RemoveHoleAtRelativeLocation_Internal(const FDynamicMeshHoleData& Hole) const;

	// A list of holes in the mesh
	UPROPERTY(Transient, SaveGame)
	TArray<FDynamicMeshHoleData> Holes;
};