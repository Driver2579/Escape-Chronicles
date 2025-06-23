// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "WeaponInventoryItemFragment.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Enums/DestructiveToolType.h"
#include "Common/Structs/EnvironmentInteractionEvents.h"
#include "Components/ActorComponents/DestructibleComponent.h"
#include "Objects/InventoryItemInstance.h"
#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"
#include "PickaxeInventoryItemFragment.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UPickaxeInventoryItemFragment : public UWeaponInventoryItemFragment
{
	GENERATED_BODY()

public:
	bool IsUseDurability() const { return bUseDurability; }

	virtual bool IsValidConfiguration(UInventoryItemDefinition* ItemDefinition) override
	{
		// Non-durability keys are always valid
		if (!bUseDurability)
		{
			return true;
		}

		// Durability keys must include durability fragment
		return ensureAlwaysMsgf(ItemDefinition->GetFragments().FindItemByClass<UDurabilityInventoryItemFragment>(),
			TEXT("If bUseDurability is true, the definition must include UDurabilityInventoryItemFragment"));
	}

	virtual void EffectHit(UInventoryItemInstance* ItemInstance) const override
	{
#if DO_CHECK
		check(ItemInstance)
#endif

		const UInventoryManagerComponent* Inventory = Cast<UInventoryManagerComponent>(ItemInstance->GetOuter());

		if (!ensureAlways(IsValid(Inventory)))
		{
			return;
		}

		AEscapeChroniclesCharacter* Character = Inventory->GetOwner<AEscapeChroniclesCharacter>();

		if (!ensureAlways(IsValid(Character)))
		{
			return;
		}

		const AController* Controller = Character->GetController();

		if (!ensureAlways(IsValid(Controller)))
		{
			return;
		}

		FVector ViewLocation;
		FRotator ViewRotation;
		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

		const FVector TraceStart = Character->GetActorLocation();
		const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * DestructiveMaxDistance;

		FCollisionQueryParams TraceParams;
		TraceParams.bTraceComplex = true;
		TraceParams.AddIgnoredActor(Character);

		FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SphereTraceRadius);

		TArray<FHitResult> HitResults;
		const bool bHit = Controller->GetWorld()->SweepMultiByChannel(HitResults, TraceStart, TraceEnd,
			FQuat::Identity, ECC_Visibility, CollisionShape, TraceParams);

		if (!bHit)
		{
			return;
		}

		for (const FHitResult& Hit : HitResults)
		{
			if (!IsValid(Hit.GetActor()))
			{
				continue;
			}

			UDestructibleComponent* DestructibleComponent = Hit.GetActor()->GetComponentByClass<UDestructibleComponent>();

			if (!IsValid(DestructibleComponent) || DestructibleComponent->GetDestructiveToolType() != DestructiveToolType)
			{
				continue;
			}

			const float Radius = FMath::RandRange(DestructiveMinRadius, DestructiveMaxRadius);

			DestructibleComponent->AddHoleAtWorldLocation(Hit.Location, Radius);

			FEnvironmentInteractionEvents::OnDestructibleDamaged.Broadcast(Character, DestructibleComponent);
		}

		if (!bUseDurability)
		{
			return;
		}

		const UDurabilityInventoryItemFragment* DurabilityInventoryItemFragment =
			ItemInstance->GetFragmentByClass<UDurabilityInventoryItemFragment>();

		if (ensureAlways(IsValid(DurabilityInventoryItemFragment)))
		{
			DurabilityInventoryItemFragment->ReduceDurability(ItemInstance, 1);
		}
	}

private:
	UPROPERTY(EditDefaultsOnly)
	EDestructiveToolType DestructiveToolType = EDestructiveToolType::Pickaxe;

	UPROPERTY(EditDefaultsOnly)
	float DestructiveMaxDistance = 200;

	UPROPERTY(EditDefaultsOnly)
	float SphereTraceRadius = 35;

	UPROPERTY(EditDefaultsOnly)
	float DestructiveMaxRadius = 100;

	UPROPERTY(EditDefaultsOnly)
	float DestructiveMinRadius = 100;

	UPROPERTY(EditDefaultsOnly)
	bool bUseDurability;
};
