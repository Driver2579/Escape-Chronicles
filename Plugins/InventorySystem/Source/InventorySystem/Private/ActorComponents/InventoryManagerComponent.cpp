// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/InventoryManagerComponent.h"

#include "Net/UnrealNetwork.h"

UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bReplicateUsingRegisteredSubObjectList = true;
	SetIsReplicatedByDefault(true);
}

void UInventoryManagerComponent::AddOnInventoryContentChanged(const FOnInventoryContentChanged::FDelegate& Callback)
{
	OnInventoryContentChanged.Add(Callback);
}

void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (bLogInventoryContentWhenChanges)
	{
		FOnInventoryContentChanged::FDelegate Delegate;
		Delegate.BindLambda([&]
		{
			LogInventoryContent();
		});

		AddOnInventoryContentChanged(Delegate);
	}

	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (!ensureAlwaysMsgf(SlotTypesAndNumber.Find(InventorySystemGameplayTags::InventoryTag_MainSlotType) != nullptr,
		TEXT("SlotTypesAndNumber must contain InventorySystemGameplayTags::InventoryTag_MainSlotType!")))
	{
		SlotTypesAndNumber.Add(InventorySystemGameplayTags::InventoryTag_MainSlotType, 1);
	}

	for (auto SlotTypeAndNumber : SlotTypesAndNumber)
	{
		if (!ensureAlwaysMsgf(SlotTypeAndNumber.Value > 0, TEXT("SlotTypeAndNumber must not be negative (%s)!"),
			*SlotTypeAndNumber.Key.ToString()))
		{
			SlotTypesAndNumber.Remove(SlotTypeAndNumber.Key);
		}
	}

	TypedInventorySlotsLists.Initialize(SlotTypesAndNumber);
}

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, TypedInventorySlotsLists);
}

void UInventoryManagerComponent::ForEachInventoryItemInstance(
	const TFunctionRef<void(UInventoryItemInstance*)>& Action) const
{
	for (const auto& TypedArray : TypedInventorySlotsLists.GetArrays())
	{
		for (const auto& Slot : TypedArray.Array.GetSlots())
		{
			UInventoryItemInstance* Instance = Slot.Instance;
			if (IsValid(Instance))
			{
				Action(Instance);
			}
		}
	}
}

void UInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		ForEachInventoryItemInstance([&](UInventoryItemInstance* Instance)
		{
			AddReplicatedSubObject(Instance);
		});
	}
}

bool UInventoryManagerComponent::AddItem(const UInventoryItemInstance* Item, size_t SlotIndex,
	const FGameplayTag SlotsType)
{
	check(GetOwner()->HasAuthority());

	if (!ensureAlways(IsValid(Item)))
	{
		return false;
	}

	const size_t SlotsArrayIndex = TypedInventorySlotsLists.IndexOfByTag(SlotsType);
	
	if (!ensureAlwaysMsgf(SlotsArrayIndex != INDEX_NONE, TEXT("Array not found by tag")))
	{
		return false;
	}
	
	const FInventorySlotsArray& SlotsArray = TypedInventorySlotsLists[SlotsArrayIndex].Array;

	// Automatic search for empty slot
	if (SlotIndex == -1)
	{
		SlotIndex = SlotsArray.GetEmptySlotIndex();
	}

	if (SlotIndex == -1
		|| !ensureAlwaysMsgf(SlotsArray.IsValidSlotIndex(SlotIndex), TEXT("Unavailable slot index"))
		|| !ensureAlwaysMsgf(SlotsArray.IsEmptySlot(SlotIndex), TEXT("Slot is not empty")))
	{
		return false;
	}

	// TODO: There are suspicions of memory leaks if you install the Outer component due to bug UE-127172
	// (In Lyra, this is circumvented by setting the Outer component owner. You can also create a subsystem that will
	// control the life cycle of the object, which is more difficult but better from an architectural point of view)
	UInventoryItemInstance* ItemInstanceDuplicate = Item->Duplicate(this);

	if (!ensureAlways(IsValid(ItemInstanceDuplicate)))
	{
		return false;
	}
	
	TypedInventorySlotsLists.SetInstance(ItemInstanceDuplicate, SlotsArrayIndex, SlotIndex);
	
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstanceDuplicate)
	{
		AddReplicatedSubObject(ItemInstanceDuplicate);
	}

	OnInventoryContentChanged.Broadcast();
	return true;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UInventoryManagerComponent::OnRep_TypedInventorySlotsLists()
{
	OnInventoryContentChanged.Broadcast();
}

void UInventoryManagerComponent::LogInventoryContent() const
{
	const FString Separator = TEXT("========================================\n");
    
	// Header
	FString Output = "UInventoryManagerComponent::LogInventoryContent:\n";
	Output += FString::Printf(TEXT("Owner: %s | Component instance: %p\n"), *GetOwner()->GetName(), this);
	Output += Separator;

	// TypedArray
	for (const FInventorySlotsTypedArray& TypedInventorySlots : TypedInventorySlotsLists.GetArrays())
	{
		const TArray<FInventorySlot>& Slots = TypedInventorySlots.Array.GetSlots();

		// Type
		Output += FString::Printf(TEXT("Slot Type: %s (%d slots)\n"), *TypedInventorySlots.Type.ToString(),
			Slots.Num());

		// Slots
		for (int32 i = 0; i < Slots.Num(); ++i)
		{
			UInventoryItemInstance* Item = Slots[i].Instance;

			if (!IsValid(Item))
			{
				Output += FString::Printf(TEXT("%02d: [EMPTY]\n"), i);
				continue;
			}
            
			Output += FString::Printf(TEXT("%02d: %s (Class: %s | Instance: %p)\n"), i, *Item->GetName(),
				*Item->GetClass()->GetName(), Item);
		}

		Output += Separator;
	}

	UE_LOG(LogTemp, Log, TEXT("%s"), *Output);
}
