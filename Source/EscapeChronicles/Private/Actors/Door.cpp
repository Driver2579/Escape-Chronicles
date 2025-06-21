// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Door.h"

#include "AbilitySystemComponent.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
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

	EnterBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnEnterBoxOverlapBeginOverlap);
	ExitBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnExitBoxOverlapBeginOverlap);

	EnterBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnDoorwayOverlapEndOverlap);
	ExitBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnDoorwayOverlapEndOverlap);
}

void ADoor::OnEnterBoxOverlapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || ConfirmedCharactersPool.Contains(Character) || EnteringCharactersPool.Contains(Character))
	{
		return;
	}

	if (ExitingCharactersPool.Contains(Character))
	{
		if (bExitRequiresKey && !HasCharacterAccessTag(Character))
		{
			if (!HasCharacterMatchingKey(Character))
			{
				SetLockDoorway(Character, true);

				return;
			}

			UseKey(Character);
		}

		ConfirmedCharactersPool.Add(Character);
	}
	else if (HasCharacterEnterAccess(Character))
	{
		EnteringCharactersPool.Add(Character);

		SetLockDoorway(Character, false);
	}
	else
	{
		SetLockDoorway(Character, true);
	}
}

void ADoor::OnExitBoxOverlapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || ConfirmedCharactersPool.Contains(Character) || ExitingCharactersPool.Contains(Character))
	{
		return;
	}

	if (EnteringCharactersPool.Contains(Character))
	{
		if (bEnterRequiresKey && !HasCharacterAccessTag(Character))
		{
			if (!HasCharacterMatchingKey(Character))
			{
				SetLockDoorway(Character, true);

				return;
			}

			UseKey(Character);
		}

		ConfirmedCharactersPool.Add(Character);
	}
	else if (HasCharacterExitAccess(Character))
	{
		ExitingCharactersPool.Add(Character);

		SetLockDoorway(Character, false);
	}
	else
	{
		SetLockDoorway(Character, true);
	}
}

void ADoor::Test(AActor* Actor, TArray<AEscapeChroniclesCharacter*>& StartPathPool,
	TArray<AEscapeChroniclesCharacter*>& EndPathPool, bool bRequiresKey)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(Actor);

	if (!IsValid(Character) || ConfirmedCharactersPool.Contains(Character) || EnteringCharactersPool.Contains(Character))
	{
		return;
	}

	if (EndPathPool.Contains(Character))
	{
		if (bRequiresKey && !HasCharacterAccessTag(Character))
		{
			if (!HasCharacterMatchingKey(Character))
			{
				SetLockDoorway(Character, true);

				return;
			}

			UseKey(Character);
		}

		ConfirmedCharactersPool.Add(Character);
	}
	else if (HasCharacterEnterAccess(Character))
	{
		StartPathPool.Add(Character);

		SetLockDoorway(Character, false);
	}
	else
	{
		SetLockDoorway(Character, true);
	}
}

void ADoor::OnDoorwayOverlapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || EnterBox->IsOverlappingActor(Character) || ExitBox->IsOverlappingActor(Character))
	{
		return;
	}

	EnteringCharactersPool.Remove(Character);
	ExitingCharactersPool.Remove(Character);
	ConfirmedCharactersPool.Remove(Character);

	// Close the door after passing through
	SetLockDoorway(Character, true);
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
