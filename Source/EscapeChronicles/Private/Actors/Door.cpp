// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Door.h"

#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Objects/InventoryItemFragments/DoorKeyInventoryItemFragment.h"

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
	
	EnterBox->OnComponentBeginOverlap.AddDynamic(this, &ADoor::OnEnterBoxBeginOverlap);
	ExitBox->OnComponentBeginOverlap.AddDynamic(this, &ADoor::OnExitBoxBeginOverlap);
	
	EnterBox->OnComponentEndOverlap.AddDynamic(this, &ADoor::OnEnterBoxEndOverlap);
	ExitBox->OnComponentEndOverlap.AddDynamic(this, &ADoor::OnExitBoxEndOverlap);
}

void ADoor::OnEnterBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || bEnterRequiresKey && !HasCharacterMatchingKey(Character))
	{
		return;
	}
	
	Character->GetCapsuleComponent()->IgnoreComponentWhenMoving(DoorwayBox, true);
	DoorwayBox->IgnoreComponentWhenMoving(Character->GetMesh(), true);
}

void ADoor::OnExitBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || bExitRequiresKey && !HasCharacterMatchingKey(Character))
	{
		return;
	}
	
	Character->GetCapsuleComponent()->IgnoreComponentWhenMoving(DoorwayBox, true);
	DoorwayBox->IgnoreComponentWhenMoving(Character->GetMesh(), true);
}

void ADoor::OnEnterBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || ExitBox->IsOverlappingActor(Character))
	{
		return;
	}
	
	Character->GetCapsuleComponent()->IgnoreComponentWhenMoving(DoorwayBox, false);
	DoorwayBox->IgnoreComponentWhenMoving(Character->GetMesh(), false);
}

void ADoor::OnExitBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (!IsValid(Character) || EnterBox->IsOverlappingActor(Character))
	{
		return;
	}
	
	Character->GetCapsuleComponent()->IgnoreComponentWhenMoving(DoorwayBox, false);
	DoorwayBox->IgnoreComponentWhenMoving(Character->GetMesh(), false);
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

void ADoor::UseKey(const AEscapeChroniclesCharacter* Character) const
{
	const UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	bool bIsKeyUsed = false;
	
	Inventory->ForEachInventoryItemInstance([&](UInventoryItemInstance* ItemInstance)
	{
		const UDoorKeyInventoryItemFragment* DoorKeyFragment =
			ItemInstance->GetFragmentByClass<UDoorKeyInventoryItemFragment>();

		if (!IsValid(DoorKeyFragment))
		{
			return;
		}
		
		/*if (bIsKeyUsed)
		{
			bIsKeyUsed = true;
		}*/
	});
}
