// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/DestructibleComponent.h"

#include "DynamicMeshActor.h"
#include "Components/DynamicMeshComponent.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

UDestructibleComponent::UDestructibleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UDestructibleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Holes);
}

void UDestructibleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the owner of this component, which should be a DynamicMeshActor
	OwningDynamicMeshActor = GetOwner<ADynamicMeshActor>();

	const bool bOwnerValid = ensureAlwaysMsgf(OwningDynamicMeshActor.IsValid(),
		TEXT("Only a DynamicMeshActor can have a DestructibleComponent!"));

	if (!bOwnerValid)
	{
		return;
	}

	// Add the SaveableActorTag to the owner if it was requested and if an owner doesn't have this tag already
	if (bAutomaticallyMakeActorSaveable)
	{
		OwningDynamicMeshActor->Tags.AddUnique(SaveableActorTag);
	}

	if (OwningDynamicMeshActor->HasAuthority())
	{
		// Make the owner replicated if it was requested
		if (bAutomaticallyMakeActorReplicated)
		{
			OwningDynamicMeshActor->SetReplicates(true);
		}

		/**
		 * We should create an AIPerceptionStimuliSourceComponent if it was requested and if the owner doesn't already
		 * have one.
		 */
		const bool bCreateAIPerceptionStimuliSourceComponent = bAutomaticallyAddAIPerceptionStimuliSourceComponent &&
			!OwningDynamicMeshActor->FindComponentByClass<UAIPerceptionStimuliSourceComponent>();

		// Create an AIPerceptionStimuliSourceComponent if we should and register it for the sight sense
		if (bCreateAIPerceptionStimuliSourceComponent)
		{
			UAIPerceptionStimuliSourceComponent* AIPerceptionStimuliSourceComponent =
				NewObject<UAIPerceptionStimuliSourceComponent>(OwningDynamicMeshActor.Get());

#if DO_CHECK
			check(IsValid(AIPerceptionStimuliSourceComponent));
#endif

			AIPerceptionStimuliSourceComponent->RegisterComponent();

			AIPerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
			AIPerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
		}
	}

	// We have to use the complex collision to update collision based on the mesh changes
	OwningDynamicMeshActor->GetDynamicMeshComponent()->EnableComplexAsSimpleCollision();
}

UDynamicMesh* UDestructibleComponent::AllocateDestructToolMeshChecked(const FVector& HoleRelativeLocation,
	const float Radius) const
{
#if DO_CHECK
	check(OwningDynamicMeshActor.IsValid());
#endif

	// Allocate the DestructToolMesh that will be used to create holes in the mesh
	UDynamicMesh* DestructToolMesh = OwningDynamicMeshActor->AllocateComputeMesh();

	// Convert the location into a transform
	FTransform ToolTransform;
	ToolTransform.SetLocation(HoleRelativeLocation);

	/**
	 * Make sure that the transform is scaled correctly based on the actor's scale. We want to have holes independently
	 * of the actor's scale.
	 */
	const FVector ActorScale3D = OwningDynamicMeshActor->GetActorScale3D();
	ToolTransform.SetScale3D(FVector(1 / ActorScale3D.X, 1 / ActorScale3D.Y, 1 / ActorScale3D.Z));

	// Create a sphere mesh for the DestructToolMesh with the specified radius
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendSphereBox(DestructToolMesh, FGeometryScriptPrimitiveOptions(),
		ToolTransform, Radius);

	return DestructToolMesh;
}

void UDestructibleComponent::GetHoleWorldLocation(const FVector& HoleRelativeLocation, FVector& OutWorldLocation) const
{
	// Convert the relative location of the hole into a world location based on the mesh's world transform
	if (ensureAlways(OwningDynamicMeshActor.IsValid()))
	{
		OutWorldLocation = OwningDynamicMeshActor->GetDynamicMeshComponent()->GetComponentTransform().TransformPosition(
			HoleRelativeLocation);
	}
}

void UDestructibleComponent::AddHoleAtWorldLocation(const FVector& HoleWorldLocation, const float HoleRadius)
{
	/**
	 * Don't run the logic if the owner isn't a valid DynamicMeshActor or if it doesn't have authority. The holes will
	 * be automatically replicated on clients.
	 */
	if (!ensureAlways(OwningDynamicMeshActor.IsValid()) || !OwningDynamicMeshActor->HasAuthority())
	{
		return;
	}

	const UDynamicMeshComponent* DynamicMeshComponent = OwningDynamicMeshActor->GetDynamicMeshComponent();

	// Convert the world location into a relative location of the DynamicMeshComponent
	const FVector HoleRelativeLocation = DynamicMeshComponent->GetComponentTransform().InverseTransformPosition(
		HoleWorldLocation);

	// Add a hole of the specified radius at the calculated location of the mesh
	AddHoleAtRelativeLocation_Internal(HoleRelativeLocation, HoleRadius);

	// Remember that we used this location to create a hole
	Holes.Add(FDynamicMeshHoleData(HoleRelativeLocation, HoleRadius));
}

void UDestructibleComponent::AddHoleAtRelativeLocation(const FVector& HoleRelativeLocation, const float HoleRadius)
{
	/**
	 * Don't run the logic if the owner isn't a valid DynamicMeshActor or if it doesn't have authority. The holes will
	 * be automatically replicated on clients.
	 */
	if (!ensureAlways(OwningDynamicMeshActor.IsValid()) || !OwningDynamicMeshActor->HasAuthority())
	{
		return;
	}

	// Add a hole of the specified radius at the given relative location of the mesh
	AddHoleAtRelativeLocation_Internal(HoleRelativeLocation, HoleRadius);

	// Remember that we used this location to create a hole
	Holes.Add(FDynamicMeshHoleData(HoleRelativeLocation, HoleRadius));
}

void UDestructibleComponent::AddHoleAtRelativeLocation_Internal(const FVector& HoleRelativeLocation,
	const float HoleRadius) const
{
#if DO_CHECK
	check(OwningDynamicMeshActor.IsValid());
#endif

	UDynamicMesh* DestructToolMesh = AllocateDestructToolMeshChecked(HoleRelativeLocation, HoleRadius);

#if DO_ENSURE
	ensureAlways(IsValid(DestructToolMesh));
#endif

	UDynamicMeshComponent* DynamicMeshComponent = OwningDynamicMeshActor->GetDynamicMeshComponent();

	// Apply the mesh boolean operation to subtract the DestructToolMesh from the current mesh
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean(DynamicMeshComponent->GetDynamicMesh(),
		FTransform::Identity, DestructToolMesh, FTransform::Identity,
		EGeometryScriptBooleanOperation::Subtract, FGeometryScriptMeshBooleanOptions());

	// Recalculate the dynamic mesh specifically at the hole location and radius
	OwningDynamicMeshActor->ReleaseComputeMesh(DestructToolMesh);
}

void UDestructibleComponent::ClearAllHoles()
{
	/**
	 * Don't run the logic if the owner isn't a valid DynamicMeshActor or if it doesn't have authority. The holes will
	 * be automatically replicated on clients.
	 */
	if (!ensureAlways(OwningDynamicMeshActor.IsValid()) || !OwningDynamicMeshActor->HasAuthority())
	{
		return;
	}

	// Remove all holes at the locations that were used to create them
	for (const FDynamicMeshHoleData& Hole : Holes)
	{
		RemoveHoleAtRelativeLocation_Internal(Hole);
	}

	// Clear the list of holes locations because we filled all of them back
	Holes.Empty();
}

void UDestructibleComponent::RemoveHoleAtRelativeLocation_Internal(const FDynamicMeshHoleData& Hole) const
{
#if DO_CHECK
	check(OwningDynamicMeshActor.IsValid());
#endif

#if DO_ENSURE
	ensureAlways(Holes.Contains(Hole));
#endif

	UDynamicMesh* DestructToolMesh = AllocateDestructToolMeshChecked(Hole.RelativeLocation, Hole.Radius);

#if DO_ENSURE
	ensureAlways(IsValid(DestructToolMesh));
#endif

	UDynamicMeshComponent* DynamicMeshComponent = OwningDynamicMeshActor->GetDynamicMeshComponent();

	/**
	 * Apply the mesh boolean operation to add the DestructToolMesh to the current mesh. We basically reverse the
	 * subtraction operation.
	 */
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean(DynamicMeshComponent->GetDynamicMesh(),
		FTransform::Identity, DestructToolMesh, FTransform::Identity,
		EGeometryScriptBooleanOperation::Union, FGeometryScriptMeshBooleanOptions());

	// Recalculate the dynamic mesh specifically at the hole location and radius
	OwningDynamicMeshActor->ReleaseComputeMesh(DestructToolMesh);
}

void UDestructibleComponent::OnPreLoadObject()
{
	// Clear the holes before loading the object to apply the loaded ones instead
	ClearAllHoles();
}

void UDestructibleComponent::OnPostLoadObject()
{
	if (!ensureAlways(OwningDynamicMeshActor.IsValid()))
	{
		return;
	}

	// Add all loaded holes to the mesh
	for (const FDynamicMeshHoleData& Hole : Holes)
	{
		AddHoleAtRelativeLocation_Internal(Hole.RelativeLocation, Hole.Radius);
	}
}

void UDestructibleComponent::OnRep_Holes(const TArray<FDynamicMeshHoleData>& OldHoles)
{
	if (!ensureAlways(OwningDynamicMeshActor.IsValid()))
	{
		return;
	}

	// Remove all holes that were present in the old Holes list but not in the new one
	for (const FDynamicMeshHoleData& OldHole : OldHoles)
	{
		if (!Holes.Contains(OldHole))
		{
			RemoveHoleAtRelativeLocation_Internal(OldHole);
		}
	}

	// Add all new holes that were not present in the old Holes list
	for (const FDynamicMeshHoleData& NewHole : Holes)
	{
		if (!OldHoles.Contains(NewHole))
		{
			AddHoleAtRelativeLocation_Internal(NewHole.RelativeLocation, NewHole.Radius);
		}
	}
}