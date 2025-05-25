// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Door.h"

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
	
	DoorwayBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorwayBox"));
	DoorwayBox->SetupAttachment(RootComponent);
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();
	
	EnterBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnEnterBoxBeginOverlap);
	EnterBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEnterBoxEndOverlap);
	
	ExitBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnExitBoxBeginOverlap);
	ExitBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnExitBoxEndOverlap);
}

void ADoor::OnEnterBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || bEnterRequiresKey && !HasCharacterMatchingKey(Character))
	{
		return;
	}
	
	SetLockDoorway(Character, true);

	// If a character crosses two zones, he goes through a door
	if (bExitRequiresKey && ExitBox->IsOverlappingActor(Character) && !ConfirmedCharactersPool.Find(Character))
	{
		UseKey(Character);
		ConfirmedCharactersPool.Add(Character, true);
	}
}

void ADoor::OnEnterBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}
	
	const bool* PoolValue = ConfirmedCharactersPool.Find(Character);
	if (PoolValue && *PoolValue == false)
	{
		ConfirmedCharactersPool.Remove(Character);
	}
	
	if (!EnterBox->IsOverlappingActor(Character))
	{
		SetLockDoorway(Character, false);
	}
}

void ADoor::OnExitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || bExitRequiresKey && !HasCharacterMatchingKey(Character))
	{
		return;
	}
	
	SetLockDoorway(Character, true);
	
	// If a character crosses two zones, he goes through a door
	if (bEnterRequiresKey && EnterBox->IsOverlappingActor(Character) && !ConfirmedCharactersPool.Find(Character))
	{
		UseKey(Character);
		ConfirmedCharactersPool.Add(Character, false);
	}
}

void ADoor::OnExitBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}

	const bool* PoolValue = ConfirmedCharactersPool.Find(Character);
	
	if (PoolValue && *PoolValue == true)
	{
		ConfirmedCharactersPool.Remove(Character);
	}
	
	if (!EnterBox->IsOverlappingActor(Character))
	{
		SetLockDoorway(Character, false);
	}
}

void ADoor::SetLockDoorway(const AEscapeChroniclesCharacter* Character, const bool IsLock) const
{
	Character->GetCapsuleComponent()->IgnoreComponentWhenMoving(DoorwayBox, IsLock);

	if (ensureAlways(IsValid(Character->GetMesh())))
	{
		DoorwayBox->IgnoreComponentWhenMoving(Character->GetMesh(), IsLock);
	}
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
		
		if (DoorKeyFragment->GetCompatibleAccessTags().HasTag(AccessTag))
		{
			bResult = true;
		}
	});
	
	return bResult;
}

void ADoor::UseKey(AEscapeChroniclesCharacter* Character)
{
	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	UInventoryItemInstance* CachedItemInstance = nullptr;
	const UDurabilityInventoryItemFragment* CachedDurabilityFragment = nullptr;
	
	bool bHasUnbreakableKey = false;
	
	Inventory->ForEachInventoryItemInstance([&](UInventoryItemInstance* ItemInstance)
	{
		if (bHasUnbreakableKey)
		{
			return;
		}
		
		const UDoorKeyInventoryItemFragment* DoorKeyFragment =
			ItemInstance->GetFragmentByClass<UDoorKeyInventoryItemFragment>();

		if (!IsValid(DoorKeyFragment) || !DoorKeyFragment->GetCompatibleAccessTags().HasTag(AccessTag))
		{
			return; 
		}
		
		bHasUnbreakableKey = !DoorKeyFragment->IsUseDurability();

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
