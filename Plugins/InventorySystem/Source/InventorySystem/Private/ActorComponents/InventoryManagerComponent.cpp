// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/InventoryManagerComponent.h"

#include "InventorySystem.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryManagerFragment.h"

UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bReplicateUsingRegisteredSubObjectList = true;

	SetIsReplicatedByDefault(true);
}

void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (bLogInventoryContent)
	{
		FOnInventoryContentChanged::FDelegate Delegate;
		Delegate.BindLambda([this] { LogInventoryContent(); });

		AddInventoryContentChangedHandler(Delegate);
	}

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// === Preparing for construction ===
	
	if (!ensureAlwaysMsgf(SlotsNumberByTypes.Find(InventorySystemGameplayTags::InventoryTag_MainSlotType),
		TEXT("SlotsNumberByTypes must contain InventorySystemGameplayTags::InventoryTag_MainSlotType!")))
	{
		// Add at least 1 slot to try to avoid most of the bugs
		SlotsNumberByTypes.Add(InventorySystemGameplayTags::InventoryTag_MainSlotType, 1);
	}

	for (auto SlotsNumberByType : SlotsNumberByTypes)
	{
		if (!ensureAlwaysMsgf(SlotsNumberByType.Value > 0, TEXT("SlotsNumberByType %s must not be negative!"),
			*SlotsNumberByType.Key.ToString()))
		{
			// Remove invalid data to try to avoid some bugs
			SlotsNumberByTypes.Remove(SlotsNumberByType.Key);
		}
	}

	// === Construct inventory ===

	InventoryContent.Construct(SlotsNumberByTypes);
}

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, InventoryContent);
	DOREPLIFETIME(ThisClass, Fragments);
}

void UInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (!IsUsingRegisteredSubObjectList())
	{
		return;
	}
	
	ForEachInventoryItemInstance([&](UInventoryItemInstance* ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	});

	for (UInventoryManagerFragment* Fragment : Fragments)
	{
		AddReplicatedSubObject(Fragment);
	}
}

UInventoryItemInstance* UInventoryManagerComponent::GetItemInstance(const int32 SlotIndex,
	const FGameplayTag SlotsType) const
{
	const int32 ArrayIndex = InventoryContent.IndexOfByTag(SlotsType);

	if (ArrayIndex == INDEX_NONE)
	{
		return nullptr;
	}

	return InventoryContent.GetInstance(ArrayIndex, SlotIndex);
}

void UInventoryManagerComponent::ForEachInventoryItemInstance(
	const TFunctionRef<void(UInventoryItemInstance*)>& Action) const
{
	for (const FInventorySlotsTypedArray& TypedArray : InventoryContent.GetItems())
	{
		for (const FInventorySlot& Slot : TypedArray.Array.GetItems())
		{
			UInventoryItemInstance* Instance = Slot.Instance;
			
			if (IsValid(Instance))
			{
				Action(Instance);
			}
		}
	}
}

bool UInventoryManagerComponent::AddItem(const UInventoryItemInstance* ItemInstance, int32 SlotIndex,
	const FGameplayTag SlotsType)
{
	if (!ensureAlways(GetOwner()->HasAuthority()) || !ensureAlways(IsValid(ItemInstance)))
	{
		return false;
	}

	const int32 SlotsArrayIndex = InventoryContent.IndexOfByTag(SlotsType);

	if (!ensureAlwaysMsgf(SlotsArrayIndex != INDEX_NONE, TEXT("Array not found by tag")))
	{
		return false;
	}

	const FInventorySlotsArray& SlotsArray = InventoryContent[SlotsArrayIndex].Array;

	// Automatic search for empty slot
	if (SlotIndex == -1 || !ensureAlways(SlotsArray.GetItems().IsValidIndex(SlotIndex)))
	{
		SlotIndex = SlotsArray.GetEmptySlotIndex();
	}

	if (SlotIndex == -1 || !ensureAlwaysMsgf(SlotsArray.IsSlotEmpty(SlotIndex), TEXT("Slot is not empty")))
	{
		return false;
	}

	/**
	 * TODO: There are suspicions of memory leaks if you install the Outer component due to bug UE-127172 (In Lyra,
	 * this is circumvented by setting the Outer component owner. You can also create a subsystem that will control the
	 * life cycle of the object, which is more difficult but better from an architectural point of view). There may not
	 * be any problems, then leave it as is, but it should be checked
	 */
	UInventoryItemInstance* ItemInstanceDuplicate = ItemInstance->Duplicate(this);

	if (!ensureAlways(IsValid(ItemInstanceDuplicate)))
	{
		return false;
	}
	
	InventoryContent.SetInstance(ItemInstanceDuplicate, SlotsArrayIndex, SlotIndex);
	
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstanceDuplicate)
	{
		AddReplicatedSubObject(ItemInstanceDuplicate);
	}

	OnInventoryContentChanged.Broadcast();
	return true;
}

bool UInventoryManagerComponent::DeleteItem(const int32 SlotIndex, const FGameplayTag SlotsType)
{
	if (!ensureAlways(GetOwner()->HasAuthority()))
	{
		return false;
	}

	const int32 SlotsArrayIndex = InventoryContent.IndexOfByTag(SlotsType);

	if (!ensureAlwaysMsgf(SlotsArrayIndex != INDEX_NONE, TEXT("Array not found by tag")))
	{
		return false;
	}

	const FInventorySlotsArray& SlotsArray = InventoryContent[SlotsArrayIndex].Array;

	if (!ensureAlwaysMsgf(SlotsArray.GetItems().IsValidIndex(SlotIndex), TEXT("Unavailable slot index"))
		|| SlotsArray.IsSlotEmpty(SlotIndex))
	{
		return false;
	}

	UInventoryItemInstance* ItemInstance = SlotsArray.GetItems()[SlotIndex].Instance;

	if (!ensureAlways(IsValid(ItemInstance)))
	{
		return false;
	}

	if (IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}

	InventoryContent.SetInstance(nullptr, SlotsArrayIndex, SlotIndex);

	OnInventoryContentChanged.Broadcast();
	return true;
}

void UInventoryManagerComponent::AddInventoryContentChangedHandler(const FOnInventoryContentChanged::FDelegate& Callback)
{
	OnInventoryContentChanged.Add(Callback);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UInventoryManagerComponent::OnRep_InventoryContent()
{
	if (bLogInventoryContent)
	{
		OnInventoryContentChanged.Broadcast();
	}
}

void UInventoryManagerComponent::LogInventoryContent() const
{
	const FString Separator = TEXT("========================================\n");
    
	// Header design
	FString Output = "UInventoryManagerComponent:\n";
	Output += FString::Printf(TEXT("Owner: %s | Component instance: %p\n"), *GetOwner()->GetName(), this);
	Output += Separator;

	// TypedArray design
	for (const FInventorySlotsTypedArray& TypedInventorySlots : InventoryContent.GetItems())
	{
		const TArray<FInventorySlot>& Slots = TypedInventorySlots.Array.GetItems();

		// Type design
		Output += FString::Printf(TEXT("Slot Type: %s (%d slots)\n"), *TypedInventorySlots.TypeTag.ToString(),
			Slots.Num());

		// Slots design
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

	UE_LOG(LogInventorySystem, Log, TEXT("%s"), *Output);
}
