// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Door.h"

#include "AbilitySystemComponent.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemFragments/DoorKeyInventoryItemFragment.h"
#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"

ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	EnterBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Enter Box Component"));
	EnterBoxComponent->SetupAttachment(RootComponent);

	ExitBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Exit Box Component"));
	ExitBoxComponent->SetupAttachment(RootComponent);

	DoorwayBlockingComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Doorway Blocking Component"));
	DoorwayBlockingComponent->SetupAttachment(RootComponent);

	DoorwayBoxZoneComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Doorway Box Zone Component"));
	DoorwayBoxZoneComponent->SetupAttachment(RootComponent);

	bReplicates = true;
}

void ADoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bEnterRequiresKey);
	DOREPLIFETIME(ThisClass, bExitRequiresKey);
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();

	DoorwayBoxZoneComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnDoorwayBoxZoneComponentBeginOverlap);
	DoorwayBoxZoneComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnDoorwayBoxZoneComponentEndOverlap);

	EnterBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEnterOrExitBoxComponentEndOverlap);
	ExitBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEnterOrExitBoxComponentEndOverlap);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void ADoor::OnDoorwayBoxZoneComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (IsValid(Character) && OtherComp == Character->GetCapsuleComponent())
	{
		TryAddCharacterToPool(Character);
	}
}

void ADoor::OnDoorwayBoxZoneComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (IsValid(Character) && OtherComp == Character->GetCapsuleComponent())
	{
		// Close the door after passing through
		SetLockDoorway(Character, true);
	}
}

void ADoor::OnEnterOrExitBoxComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}

	const UCapsuleComponent* CharactersCapsule = Character->GetCapsuleComponent();

#if DO_ENSURE
	ensureAlways(CharactersCapsule);
#endif
	
	// Remove the character's pass when he has completely passed through the door
	if (!DoorwayBoxZoneComponent->IsOverlappingComponent(CharactersCapsule))
	{
		ConfirmedCharactersPool.Remove(Character);
	}
}

void ADoor::SetLockDoorway(AEscapeChroniclesCharacter* Character, const bool IsLock) const
{
	UCapsuleComponent* CharacterCapsule = Character->GetCapsuleComponent();

	if (ensureAlways(IsValid(CharacterCapsule)))
	{
		CharacterCapsule->IgnoreComponentWhenMoving(DoorwayBlockingComponent, !IsLock);
		DoorwayBlockingComponent->IgnoreActorWhenMoving(Character, !IsLock);
	}
}

bool ADoor::DoesCharacterHaveAccessTag(const AEscapeChroniclesCharacter* Character) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return false;
	}

	return AbilitySystemComponent->HasAnyMatchingGameplayTags(CharacterAccessTags);
}

bool ADoor::DoesCharacterHaveMatchingKey(const AEscapeChroniclesCharacter* Character) const
{
	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory))) return false;

	bool bResult = false;
	
	Inventory->ForEachInventoryItemInstance([this, &bResult](UInventoryItemInstance* ItemInstance)
	{
		const UDoorKeyInventoryItemFragment* DoorKeyFragment =
			ItemInstance->GetFragmentByClass<UDoorKeyInventoryItemFragment>();

		if (IsValid(DoorKeyFragment) && DoorKeyFragment->GetCompatibleAccessTags().HasTag(KeyAccessTag))
		{
			bResult = true;
		}
		
		return true;
	});
	
	return bResult;
}

void ADoor::UseKey(const AEscapeChroniclesCharacter* Character) const
{
	if (!HasAuthority())
	{
		return;
	}
	
	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	Inventory->ForEachInventoryItemInstance([this](UInventoryItemInstance* ItemInstance)
	{
		const UDoorKeyInventoryItemFragment* DoorKeyFragment =
			ItemInstance->GetFragmentByClass<UDoorKeyInventoryItemFragment>();

		// Skip non-key items and keys without a matching access tag
		if (!IsValid(DoorKeyFragment) || !DoorKeyFragment->GetCompatibleAccessTags().HasTag(KeyAccessTag)) return true;

		const UDurabilityInventoryItemFragment* DurabilityKeyFragment =
			ItemInstance->GetFragmentByClass<UDurabilityInventoryItemFragment>();

		// Use durability if the key requires it
		if (DoorKeyFragment->IsUsingDurability() && ensureAlways(IsValid(DurabilityKeyFragment)))
		{
			DurabilityKeyFragment->ReduceDurability(ItemInstance, 1);
		}

		// Stop iteration
		return false;
	});
}

void ADoor::UpdateConfirmedCharactersPool()
{
	// TODO: Update the implementation of this method for the new door logic

	TArray<AActor*> OverlappingActors;
	DoorwayBoxZoneComponent->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(Actor);

		if (IsValid(Character))
		{
			TryAddCharacterToPool(Character);
		}
	}
}

void ADoor::TryAddCharacterToPool(AEscapeChroniclesCharacter* Character)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	// Open the door for the character if we have already determined that he has an access
	if (ConfirmedCharactersPool.Contains(Character))
	{
		SetLockDoorway(Character, false);

		return;
	}

	// Determine whether character needs a key in the current conditions
	bool bRequiresKey = true;

	const UCapsuleComponent* CharacterCapsule = Character->GetCapsuleComponent();

	if (ensureAlways(IsValid(CharacterCapsule)))
	{
		/**
		 * Determines whether the given character currently requires a key to pass through the doorway, based on their
		 * position (enter/exit side) and the door's access settings.
		 */
		if (EnterBoxComponent->IsOverlappingComponent(CharacterCapsule))
		{
			bRequiresKey = bEnterRequiresKey;
		}
		else if (ExitBoxComponent->IsOverlappingComponent(CharacterCapsule))
		{
			bRequiresKey = bExitRequiresKey;
		}
		else
		{
			ensureAlwaysMsgf(false, TEXT("During this check, the character must overlap either EnterBoxComponent or ExitBoxComponent."));
			return;
		}
	}
	
	// Unlock the door if the key is not required or the character has an access tag
	if (!bRequiresKey || DoesCharacterHaveAccessTag(Character))
	{
		SetLockDoorway(Character, false);

		ConfirmedCharactersPool.Add(Character);
		
		return;
	}

	// If character has the required key, use it
	if (DoesCharacterHaveMatchingKey(Character))
	{
		UseKey(Character);

		SetLockDoorway(Character, false);

		ConfirmedCharactersPool.Add(Character);
	}
}
