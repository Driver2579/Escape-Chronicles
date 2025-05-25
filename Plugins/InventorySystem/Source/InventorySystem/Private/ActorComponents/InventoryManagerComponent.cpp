// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/InventoryManagerComponent.h"

#include "Net/UnrealNetwork.h"
#include "Objects/InventoryManagerFragments/InventoryManagerFragment.h"

UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bReplicateUsingRegisteredSubObjectList = true;
	SetIsReplicatedByDefault(true);
}

void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (bLogInventoryContentWhenChanges)
	{
		FOnInventoryContentChanged::FDelegate Delegate;
		Delegate.BindLambda([&] { LogInventoryContent(); });

		AddInventoryContentChangedHandler(Delegate);
	}

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// === Preparing for initialization ===
	
	if (!ensureAlwaysMsgf(SlotsNumberByTypes.Find(InventorySystemGameplayTags::InventoryTag_MainSlotType) != nullptr,
		TEXT("SlotsNumberByTypes must contain InventorySystemGameplayTags::InventoryTag_MainSlotType!")))
	{
		SlotsNumberByTypes.Add(InventorySystemGameplayTags::InventoryTag_MainSlotType, 1);
	}

	for (auto SlotsNumberByType : SlotsNumberByTypes)
	{
		if (!ensureAlwaysMsgf(SlotsNumberByType.Value > 0, TEXT("SlotsNumberByType %s must not be negative!"),
			*SlotsNumberByType.Key.ToString()))
		{
			SlotsNumberByTypes.Remove(SlotsNumberByType.Key);
		}
	}

	// === Initialization ===
	
	TypedInventorySlotsLists.Initialize(SlotsNumberByTypes);
}

UInventoryItemInstance* UInventoryManagerComponent::GetItemInstance(const int32 SlotIndex,
	const FGameplayTag SlotsType) const
{
	const int32 ArrayIndex = TypedInventorySlotsLists.IndexOfByTag(SlotsType);
	
	if (ArrayIndex == INDEX_NONE)
	{
		return nullptr;
	}

	return TypedInventorySlotsLists.GetInstance(ArrayIndex, SlotIndex);
}

void UInventoryManagerComponent::ForEachInventoryItemInstance(
	const TFunctionRef<void(UInventoryItemInstance*)>& Action) const
{
	for (const FInventorySlotsTypedArray& TypedArray : TypedInventorySlotsLists.GetArrays())
	{
		for (const FInventorySlot& Slot : TypedArray.Array.GetSlots())
		{
			UInventoryItemInstance* Instance = Slot.Instance;
			if (IsValid(Instance))
			{
				Action(Instance);
			}
		}
	}
}

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, TypedInventorySlotsLists);
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

bool UInventoryManagerComponent::AddItem(const UInventoryItemInstance* ItemInstance, int32 SlotIndex,
	const FGameplayTag SlotsType)
{
	check(GetOwner()->HasAuthority());

	if (!ensureAlways(IsValid(ItemInstance)))
	{
		return false;
	}

	const int32 SlotsArrayIndex = TypedInventorySlotsLists.IndexOfByTag(SlotsType);
	
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
	UInventoryItemInstance* ItemInstanceDuplicate = ItemInstance->Duplicate(this);

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

bool UInventoryManagerComponent::DeleteItem(const int32 SlotIndex, const FGameplayTag SlotsType)
{
	check(GetOwner()->HasAuthority());

	const int32 SlotsArrayIndex = TypedInventorySlotsLists.IndexOfByTag(SlotsType);
	
	if (!ensureAlwaysMsgf(SlotsArrayIndex != INDEX_NONE, TEXT("Array not found by tag")))
	{
		return false;
	}
	
	const FInventorySlotsArray& SlotsArray = TypedInventorySlotsLists[SlotsArrayIndex].Array;
	
	if (!ensureAlwaysMsgf(SlotsArray.IsValidSlotIndex(SlotIndex), TEXT("Unavailable slot index"))
		|| SlotsArray.IsEmptySlot(SlotIndex))
	{
		return false;
	}

	UInventoryItemInstance* ItemInstance = SlotsArray.GetSlots()[SlotIndex].Instance;

	if (!ensureAlways(IsValid(ItemInstance)))
	{
		return false;
	}
	
	if (IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
	
	TypedInventorySlotsLists.SetInstance(nullptr, SlotsArrayIndex, SlotIndex);
	
	OnInventoryContentChanged.Broadcast();
	return true;
}

bool UInventoryManagerComponent::GetItemInstanceContainerAndIndex(FGameplayTag& OutSlotsType, int32& OutSlotIndex,
	UInventoryItemInstance* ItemInstance) const
{
	for (const FInventorySlotsTypedArray& TypedArray : TypedInventorySlotsLists.GetArrays())
	{
		for (int32 Index = 0; Index <= TypedArray.Array.GetSlots().Num(); Index++)
		{
			if (ItemInstance == TypedArray.Array[Index].Instance)
			{
				OutSlotsType = TypedArray.Type;
				OutSlotIndex = Index;

				return true;
			}
		}
	}

	return false;
}

void UInventoryManagerComponent::BreakItemInstance(UInventoryItemInstance* ItemInstance)
{
	FGameplayTag SlotsType;
	int32 SlotIndex;
	
	if (ensureAlways(GetItemInstanceContainerAndIndex(SlotsType, SlotIndex, ItemInstance)))
	{
		DeleteItem(SlotIndex, SlotsType);
	}
}

void UInventoryManagerComponent::AddInventoryContentChangedHandler(const FOnInventoryContentChanged::FDelegate& Callback)
{
	OnInventoryContentChanged.Add(Callback);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UInventoryManagerComponent::OnRep_TypedInventorySlotsLists()
{
	if (bLogInventoryContentWhenChanges)
	{
		OnInventoryContentChanged.Broadcast();
	}
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
