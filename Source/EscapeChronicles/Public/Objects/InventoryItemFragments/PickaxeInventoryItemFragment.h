// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "WeaponInventoryItemFragment.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Enums/DestructiveToolType.h"
#include "Components/ActorComponents/DestructibleComponent.h"
#include "Objects/InventoryItemInstance.h"
#include "PickaxeInventoryItemFragment.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UPickaxeInventoryItemFragment : public UWeaponInventoryItemFragment
{
	GENERATED_BODY()

public:
	virtual void EffectHit(UInventoryItemInstance* ItemInstance) override
	{
		const UInventoryManagerComponent* Inventory = Cast<UInventoryManagerComponent>(ItemInstance->GetOuter());

		if (!ensureAlways(IsValid(Inventory)))
		{
			return;
		}

		const AEscapeChroniclesCharacter* Character = Inventory->GetOwner<AEscapeChroniclesCharacter>();

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

		const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * DestructiveMaxDistance;

		FCollisionQueryParams TraceParams;
		TraceParams.bTraceComplex = true;
		TraceParams.AddIgnoredActor(Character);

		FHitResult HitResult;
		const bool bHit = Controller->GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd,
			ECC_Visibility, TraceParams);

		if (!bHit || !IsValid(HitResult.GetActor()))
		{
			return;
		}

		UDestructibleComponent* DestructibleComponent =
			HitResult.GetActor()->GetComponentByClass<UDestructibleComponent>();

		if (!IsValid(DestructibleComponent) || DestructibleComponent->GetDestructiveToolType() != DestructiveToolType)
		{
			return;
		}

		const float Radius = FMath::RandRange(DestructiveMinRadius, DestructiveMaxRadius);

		DestructibleComponent->AddHoleAtWorldLocation(HitResult.Location, Radius);
	}

private:
	UPROPERTY(EditDefaultsOnly)
	EDestructiveToolType DestructiveToolType = EDestructiveToolType::Pickaxe;

	UPROPERTY(EditDefaultsOnly)
	float DestructiveMaxDistance = 200;

	UPROPERTY(EditDefaultsOnly)
	float DestructiveMaxRadius = 100;

	UPROPERTY(EditDefaultsOnly)
	float DestructiveMinRadius = 100;
};
