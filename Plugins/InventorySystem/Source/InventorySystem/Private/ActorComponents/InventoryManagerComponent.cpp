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

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, InventoryContent);
	DOREPLIFETIME(ThisClass, Fragments);
}

void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner()->HasAuthority())
	{
		for (UInventoryManagerFragment* Fragment : Fragments)
		{
			Fragment->OnManagerInitialized();
		}

		return;
	}

#if WITH_EDITORONLY_DATA && !NO_LOGGING
	if (bLogInventoryContent)
	{
		OnContentChanged.AddLambda([this]
		{
			LogInventoryContent();
		});
	}
#endif

	// === Preparing for construction ===

	const bool bMainSlotDefined  =
		!ensureAlwaysMsgf(SlotsNumberByTypes.Contains(InventorySystemGameplayTags::Inventory_Slot_Type_Main),
			TEXT("SlotsNumberByTypes must contain InventorySystemGameplayTags::Inventory_Slot_Type_Main!"));

	if (bMainSlotDefined)
	{
		// Add at least 1 slot to try to avoid most of the bugs
		SlotsNumberByTypes.Add(InventorySystemGameplayTags::Inventory_Slot_Type_Main, 1);
	}

	for (const auto& SlotsNumberByType : SlotsNumberByTypes)
	{
		const bool bSlotNumberValid =
			!ensureAlwaysMsgf(SlotsNumberByType.Value > 0, TEXT("SlotsNumberByType %s must be positive!"),
				*SlotsNumberByType.Key.ToString());

		if (bSlotNumberValid)
		{
			// Remove invalid data to try to avoid some bugs
			SlotsNumberByTypes.Remove(SlotsNumberByType.Key);
		}
	}

	// === Construct inventory ===

	InventoryContent.Construct(this, SlotsNumberByTypes);

	for (UInventoryManagerFragment* Fragment : Fragments)
	{
		Fragment->OnManagerInitialized();
	}
}

void UInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (!IsUsingRegisteredSubObjectList())
	{
		return;
	}
	
	ForEachInventoryItemInstance([this](UInventoryItemInstance* ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	});

	for (UInventoryManagerFragment* Fragment : Fragments)
	{
		AddReplicatedSubObject(Fragment);
	}
}

UInventoryItemInstance* UInventoryManagerComponent::GetItemInstance(const int32 SlotIndex,
	const FGameplayTag& SlotTypeTag) const
{
	const int32 ArrayIndex = InventoryContent.IndexOfByTag(SlotTypeTag);

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
	const FGameplayTag& SlotTypeTag)
{
#if DO_CHECK
	check(IsValid(ItemInstance));
#endif

#if DO_ENSURE
	ensureAlways(GetOwner()->HasAuthority());
#endif

	// Find which inventory array corresponds to the requested slot type
	const int32 SlotsArrayIndex = InventoryContent.IndexOfByTag(SlotTypeTag);

	const bool bSlotsTypeValid = !ensureAlwaysMsgf(SlotsArrayIndex != INDEX_NONE,
		TEXT("Failed to find a slots array by tag %s"), *SlotTypeTag.ToString());

	if (bSlotsTypeValid)
	{
		return false;
	}

	const FInventorySlotsArray& SlotsArray = InventoryContent[SlotsArrayIndex].Array;

	// Automatic search for empty slot
	if (SlotIndex == INDEX_NONE || !ensureAlways(SlotsArray.GetItems().IsValidIndex(SlotIndex)))
	{
		SlotIndex = SlotsArray.GetEmptySlotIndex();
	}

	if (SlotIndex == INDEX_NONE || !ensureAlways(SlotsArray.IsSlotEmpty(SlotIndex)))
	{
		return false;
	}

	/**
	 * TODO: Potential memory leak if you assign the component as the Outer due to UE bug UE-127172. Lyra circumvents
	 * this by setting the component's owner as the Outer instead. Consider implementing a subsystem to manage the item
	 * instance lifecycle explicitly. This would be more robust from an architectural standpoint but is also more
	 * complex. There may be no issue in practice, but this should be verified.
	 */
	UInventoryItemInstance* ItemInstanceDuplicate = ItemInstance->Duplicate(this);

#if DO_CHECK
	check(IsValid(ItemInstanceDuplicate))
#endif

	// Assign the duplicated item instance to the target slot
	InventoryContent.SetInstance(ItemInstanceDuplicate, SlotsArrayIndex, SlotIndex);

	/**
	 * Start replication item instance if bReplicateUsingRegisteredSubObjectList is enabled, but postpone replication
	 * if the component is not ready for replication yet.
	*/
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(ItemInstanceDuplicate);
	}

#if DO_CHECK
	check(IsValid(ItemInstanceDuplicate->GetDefinition()));
#endif

	const UInventoryItemDefinition* ItemDefinitionCDO = ItemInstanceDuplicate->GetDefinition()->GetDefaultObject<
		UInventoryItemDefinition>();

	// Notify all fragments that the item was added to the slot
	for (const UInventoryItemFragment* ItemFragment : ItemDefinitionCDO->GetFragments())
	{
		ItemFragment->OnItemAddedToSlot(ItemInstanceDuplicate, this, SlotTypeTag, SlotIndex);
	}

	OnContentChanged.Broadcast();

	return true;
}

bool UInventoryManagerComponent::DeleteItem(const int32 SlotIndex, const FGameplayTag& SlotTypeTag)
{
#if DO_ENSURE
	ensureAlways(GetOwner()->HasAuthority());
#endif

	// Find which inventory array corresponds to the requested slot type
	const int32 SlotsArrayIndex = InventoryContent.IndexOfByTag(SlotTypeTag);

	const bool bSlotsTypeValid = !ensureAlwaysMsgf(SlotsArrayIndex != INDEX_NONE,
		TEXT("Failed to find a slots array by tag %s"), *SlotTypeTag.ToString());

	if (bSlotsTypeValid)
	{
		return false;
	}

	const FInventorySlotsArray& SlotsArray = InventoryContent[SlotsArrayIndex].Array;

#if DO_CHECK
	checkf(SlotsArray.GetItems().IsValidIndex(SlotIndex), TEXT("Unavailable slot index"));
#endif

	if (SlotsArray.IsSlotEmpty(SlotIndex))
	{
		return false;
	}

	UInventoryItemInstance* ItemInstance = SlotsArray.GetItems()[SlotIndex].Instance;

	if (!ensureAlways(IsValid(ItemInstance)))
	{
		return false;
	}

	// Stop replication item instance if bReplicateUsingRegisteredSubObjectList is enabled
	if (IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}

	// Clear the slot by setting its instance to null
	InventoryContent.SetInstance(nullptr, SlotsArrayIndex, SlotIndex);

#if DO_CHECK
	check(IsValid(ItemInstance->GetDefinition()));
#endif

	const UInventoryItemDefinition* ItemDefinitionCDO = ItemInstance->GetDefinition()->GetDefaultObject<
		UInventoryItemDefinition>();

	// Notify all fragments that the item was removed from the slot
	for (const UInventoryItemFragment* ItemFragment : ItemDefinitionCDO->GetFragments())
	{
		ItemFragment->OnItemRemovedFromSlot(ItemInstance, this, SlotTypeTag, SlotIndex);
	}

	OnContentChanged.Broadcast();

	return true;
}

void UInventoryManagerComponent::ClearInventory()
{
	// Remove all items
	for (const FInventorySlotsTypedArray& Slots : GetInventoryContent().GetItems())
	{
		for (int32 i = 0; i < Slots.Array.GetItems().Num(); ++i)
		{
			DeleteItem(i, Slots.TypeTag);
		}
	}
}

void UInventoryManagerComponent::OnRep_InventoryContent()
{
	InventoryContent.UpdateOwningRefs(this);
	OnContentChanged.Broadcast();
}

bool UInventoryManagerComponent::GetItemInstanceContainerAndIndex(FGameplayTag& OutSlotsType, int32& OutSlotIndex,
	UInventoryItemInstance* ItemInstance) const
{
	for (const FInventorySlotsTypedArray& TypedArray : InventoryContent.GetItems())
	{
		for (int32 Index = 0; Index <= TypedArray.Array.GetItems().Num(); Index++)
		{
			if (ItemInstance == TypedArray.Array[Index].Instance)
			{
				OutSlotsType = TypedArray.TypeTag;
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

	// Find the location of the breaking item instance and remove it from the slot
	if (ensureAlways(GetItemInstanceContainerAndIndex(SlotsType, SlotIndex, ItemInstance)))
	{
		DeleteItem(SlotIndex, SlotsType);
	}
}

#if WITH_EDITORONLY_DATA && !NO_LOGGING
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

	UE_LOG(LogInventorySystem, Display, TEXT("%s"), *Output);
}
#endif