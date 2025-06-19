// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Saveable.h"
#include "Common/Enums/DestructiveToolType.h"
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
 * @remark All public functions of this component can be called only on the server. The client calls will be ignored.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UDestructibleComponent : public UActorComponent, public ISaveable
{
	GENERATED_BODY()

public:
	UDestructibleComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<FDynamicMeshHoleData>& GetHoles() const { return Holes; }

	EDestructiveToolType GetDestructiveToolType() const { return DestructiveToolType; }

	/**
	 * Converts the given relative location of the hole in the mesh to a world location based on the mesh's world
	 * transform.
	 */
	void GetHoleWorldLocation(const FVector& HoleRelativeLocation, FVector& OutWorldLocation) const;

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

	// Whether the component should automatically make the actor replicated at the beginning of the game
	UPROPERTY(EditAnywhere, Category="Replication")
	bool bAutomaticallyMakeActorReplicated = true;

	/**
	 * Whether the component should automatically add a UAIPerceptionStimuliSourceComponent to the actor at the
	 * beginning of the game with a registered sight sense.
	 * @remark The component will be added only if the actor has authority.
	 */
	UPROPERTY(EditAnywhere, Category="AI")
	bool bAutomaticallyAddAIPerceptionStimuliSourceComponent = true;

	UPROPERTY(Transient)
	TObjectPtr<UDynamicMesh> OriginalDynamicMeshCopy;

	/**
	 * A type of tool that will be used to create holes in the mesh. This should be checked before creating holes. Other
	 * types of tools than the selected one should not be used to create holes in an actor that uses this component.
	 */
	UPROPERTY(EditAnywhere)
	EDestructiveToolType DestructiveToolType = EDestructiveToolType::Pickaxe;

	/**
	 * Allocates the tool mesh that will be used to create holes in the mesh. The mesh is a sphere with the specified
	 * radius and the specified relative location.
	 * @remark The OwningDynamicMeshActor must be valid when calling this function.
	 */
	UDynamicMesh* AllocateDestructToolMeshChecked(const FVector& HoleRelativeLocation, const float Radius) const;

	/**
	 * An actual implementation of the AddHoleAtRelativeLocation function. Doesn't add the data to the Holes array,
	 * doesn't check if the OwningDynamicMeshActor is valid, and doesn't check if the owner is an authority.
	 */
	void AddHoleAtRelativeLocation_Internal(const FVector& HoleRelativeLocation, const float HoleRadius) const;

	// TODO: Replace the next logic with a FastArraySerializer

	// A list of holes in the mesh
	UPROPERTY(Transient, SaveGame, ReplicatedUsing="OnRep_Holes")
	TArray<FDynamicMeshHoleData> Holes;

	UFUNCTION()
	void OnRep_Holes(const TArray<FDynamicMeshHoleData>& OldHoles);
};