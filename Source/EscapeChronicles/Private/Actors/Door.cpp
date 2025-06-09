// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Door.h"

#include "AbilitySystemComponent.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Objects/InventoryItemFragments/DoorKeyInventoryItemFragment.h"
#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"

ADoor::ADoor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	EnterBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EnterBox"));
	EnterBox->SetupAttachment(RootComponent);
	
	ExitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitBox"));
	ExitBox->SetupAttachment(RootComponent);
	
	DoorwayBoxBlock = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorwayBoxBlock"));
	DoorwayBoxBlock->SetupAttachment(RootComponent);
	
	DoorwayBoxOverlap = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorwayBoxOverlap"));
	DoorwayBoxOverlap->SetupAttachment(RootComponent);
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
	
	DoorwayBoxOverlap->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnDoorwayBoxOverlapBeginOverlap);
	DoorwayBoxOverlap->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnDoorwayBoxOverlapEndOverlap);

	EnterBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEnterOrExitBoxOverlapEndOverlap);
	ExitBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEnterOrExitBoxOverlapEndOverlap);
}

void ADoor::OnDoorwayBoxOverlapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}

	// Open the doors for character if it already has access to it
	if (ConfirmedCharactersPool.Contains(Character))
	{
		SetLockDoorway(Character, false);

		return;
	}

	// Determine whether character needs a key in the current conditions
	const bool bRequiresKey = IsRequiresKey(Character);

	// Unlock the door if the key is not required or the character has an access tag
	if (!bRequiresKey || HasCharacterAccessTag(Character))
	{
		SetLockDoorway(Character, false);

		ConfirmedCharactersPool.Add(Character);
		
		return;
	}

	// If character has the required key, use it
	if (HasCharacterMatchingKey(Character))
	{
		UseKey(Character);

		SetLockDoorway(Character, false);

		ConfirmedCharactersPool.Add(Character);
	}
}

void ADoor::OnDoorwayBoxOverlapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}

	// Close the door after passing through
	SetLockDoorway(Character, true);
}

void ADoor::OnEnterOrExitBoxOverlapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}

	// Remove the character's pass when he has completely passed through the door
	if (!DoorwayBoxOverlap->IsOverlappingActor(Character))
	{
		ConfirmedCharactersPool.Remove(Character);
	}
}

bool ADoor::IsRequiresKey(const AEscapeChroniclesCharacter* Character) const
{
	if (EnterBox->IsOverlappingActor(Character))
	{
		return bEnterRequiresKey;
	}
	if (ExitBox->IsOverlappingActor(Character))
	{
		return bExitRequiresKey;
	}

	return ensureAlwaysMsgf(true, TEXT("During this check, the character must be in one of the direction boxes."));
}

void ADoor::SetLockDoorway(const AEscapeChroniclesCharacter* Character, const bool IsLock) const
{
	UCapsuleComponent* CharacterCapsule = Character->GetCapsuleComponent();

	if (ensureAlways(IsValid(CharacterCapsule)))
	{
		CharacterCapsule->IgnoreComponentWhenMoving(DoorwayBoxBlock, !IsLock);
	}
	
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	
	if (ensureAlways(IsValid(CharacterMesh)))
	{
		DoorwayBoxBlock->IgnoreComponentWhenMoving(CharacterMesh, !IsLock);
	}
}

bool ADoor::HasCharacterAccessTag(const AEscapeChroniclesCharacter* Character) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return false;
	}

	return AbilitySystemComponent->HasAnyMatchingGameplayTags(CharacterAccessTags);
}

bool ADoor::HasCharacterMatchingKey(const AEscapeChroniclesCharacter* Character) const
{
	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return false;
	}

	bool bResult = false;
	
	Inventory->ForEachInventoryItemInstance([&](UInventoryItemInstance* ItemInstance)
	{
		const UDoorKeyInventoryItemFragment* DoorKeyFragment =
			ItemInstance->GetFragmentByClass<UDoorKeyInventoryItemFragment>();

		if (!IsValid(DoorKeyFragment))
		{
			return;
		}
		
		if (DoorKeyFragment->GetCompatibleAccessTags().HasTag(KeyAccessTag))
		{
			bResult = true;
		}
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

	// Cache data to reduce durability from a suitable key if necessary
	UInventoryItemInstance* CachedItemInstance = nullptr;
	const UDurabilityInventoryItemFragment* CachedDurabilityFragment = nullptr;

	bool bHasUnbreakableKey = false;
	
	Inventory->ForEachInventoryItemInstance([&](UInventoryItemInstance* ItemInstance)
	{
		// Do not continue the search if there is already unbreakable key.
		if (bHasUnbreakableKey)
		{
			return;
		}
		
		const UDoorKeyInventoryItemFragment* DoorKeyFragment =
			ItemInstance->GetFragmentByClass<UDoorKeyInventoryItemFragment>();

		if (!IsValid(DoorKeyFragment) || !DoorKeyFragment->GetCompatibleAccessTags().HasTag(KeyAccessTag))
		{
			return; 
		}
		
		bHasUnbreakableKey = !DoorKeyFragment->IsUseDurability();

		// If the key is not unbreakable, it must have a durability fragment
		if (IsValid(CachedDurabilityFragment) || bHasUnbreakableKey)
		{
			return; 
		}
		
		const UDurabilityInventoryItemFragment* DurabilityKeyFragment =
			ItemInstance->GetFragmentByClass<UDurabilityInventoryItemFragment>();
		
		if (IsValid(DurabilityKeyFragment))
		{
			CachedItemInstance = ItemInstance;
			CachedDurabilityFragment = DurabilityKeyFragment;
		}
	});

	if (!bHasUnbreakableKey)
	{
		CachedDurabilityFragment->ReduceDurability(CachedItemInstance, 1);
	}
}
