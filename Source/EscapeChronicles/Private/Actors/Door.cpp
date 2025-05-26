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

	if (ConfirmedCharactersPool.Contains(Character))
	{
		SetLockDoorway(Character, true);

		return;
	}
	
	const bool bRequiresKey = IsRequiresKey(Character);
	
	if (!bRequiresKey || HasCharacterAccess(Character))
	{
		SetLockDoorway(Character, true);

		ConfirmedCharactersPool.Add(Character);
		
		return;
	}
	
	if (HasCharacterMatchingKey(Character))
	{
		UseKey(Character);

		SetLockDoorway(Character, true);

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

	SetLockDoorway(Character, false);
}

void ADoor::OnEnterOrExitBoxOverlapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character))
	{
		return;
	}

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

bool ADoor::IsOppositeSideRequiresKey(const AEscapeChroniclesCharacter* Character) const
{
	if (EnterBox->IsOverlappingActor(Character))
	{
		return bExitRequiresKey;
	}
	if (ExitBox->IsOverlappingActor(Character))
	{
		return bEnterRequiresKey;
	}

	return ensureAlwaysMsgf(true, TEXT("During this check, the character must be in one of the direction boxes."));
}

void ADoor::SetLockDoorway(const AEscapeChroniclesCharacter* Character, const bool IsLock) const
{
	Character->GetCapsuleComponent()->IgnoreComponentWhenMoving(DoorwayBoxBlock, IsLock);

	if (ensureAlways(IsValid(Character->GetMesh())))
	{
		DoorwayBoxBlock->IgnoreComponentWhenMoving(Character->GetMesh(), IsLock);
	}
}

bool ADoor::HasCharacterAccess(const AEscapeChroniclesCharacter* Character) const
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

		if (!IsValid(DoorKeyFragment) || !DoorKeyFragment->GetCompatibleAccessTags().HasTag(KeyAccessTag))
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

	if (HasAuthority() && !bHasUnbreakableKey)
	{
		CachedDurabilityFragment->ReduceDurability(CachedItemInstance, 1);
	}
}
