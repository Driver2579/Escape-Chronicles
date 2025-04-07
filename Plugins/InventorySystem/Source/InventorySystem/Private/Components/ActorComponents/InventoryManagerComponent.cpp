// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/InventoryManagerComponent.h"

#include "InventorySystemGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemInstance.h"

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
		Delegate.BindLambda([&](UInventoryManagerComponent* Inventory)
		{
			LogInventoryContent();
		});
    
		AddOnInventoryContentChanged(Delegate);
	}
	
	if (GetOwner()->HasAuthority())
	{
		TypedInventorySlotsLists.Initialize(SlotTypesAndQuantities);
	}
}

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, TypedInventorySlotsLists);
}

void UInventoryManagerComponent::ForEachInventoryItemInstance(
	const TFunctionRef<void(UInventoryItemInstance*)>& Action) const
{
	for (const auto& TypedArray : TypedInventorySlotsLists.GetArrays())
	{
		for (const auto& Slot : TypedArray.GetArray().GetSlots())
		{
			UInventoryItemInstance* Instance = Slot.GetInstance();
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

bool UInventoryManagerComponent::AddItem(UInventoryItemInstance* Item, int32 Index, const FGameplayTag Type)
{
	check(GetOwner()->HasAuthority());

	if (!ensureAlways(IsValid(Item)))
	{
		return false;
	}
	
	FInventorySlotsTypedArray* SlotsTypedArray = TypedInventorySlotsLists.FindArrayByTag(Type);
	
	if (!ensureAlwaysMsgf(SlotsTypedArray, TEXT("Array not found by tag")))
	{
		return false;
	}

	const FInventorySlotsArray& SlotsArray = SlotsTypedArray->GetArray();
	
	if (Index == -1)
	{
		Index = SlotsArray.GetEmptySlotIndex();
	}

	if (Index == -1 || !ensureAlwaysMsgf(SlotsArray.IsValidSlotIndex(Index), TEXT("Unavailable slot index")) ||
		!ensureAlwaysMsgf(SlotsArray.IsEmptySlot(Index), TEXT("Slot is not empty")))
	{
		return false;
	}

	// NOTE: There are suspicions of memory leaks if you install the Outer component due to bug UE-127172
	// (In Lyra, this is circumvented by setting the Outer component owner. You can also create a subsystem that will
	// control the life cycle of the object, which is more difficult but better from an architectural point of view)
	UInventoryItemInstance* ItemInstanceDuplicate = Item->Duplicate(this);

	if (!ensureAlways(IsValid(ItemInstanceDuplicate)))
	{
		return false;
	}
	
	TypedInventorySlotsLists.SetInstanceIntoSlot(ItemInstanceDuplicate, Index, SlotsTypedArray);
	
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstanceDuplicate)
	{
		AddReplicatedSubObject(ItemInstanceDuplicate);
	}

	OnInventoryContentChanged.Broadcast(this);

	return true;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UInventoryManagerComponent::OnRep_TypedInventorySlotsLists()
{
	OnInventoryContentChanged.Broadcast(this);
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
		const auto& Slots = TypedInventorySlots.GetArray().GetSlots();

		// Type
		Output += FString::Printf(TEXT("Slot Type: %s (%d slots)\n"), *TypedInventorySlots.GetType().ToString(),
			Slots.Num());

		// Slots
		for (int32 i = 0; i < Slots.Num(); ++i)
		{
			UInventoryItemInstance* Item = Slots[i].GetInstance();

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
