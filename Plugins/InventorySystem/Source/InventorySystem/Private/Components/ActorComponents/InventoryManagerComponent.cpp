// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/InventoryManagerComponent.h"

#include "InventorySystemGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemInstance.h"

UInventoryManagerComponent::UInventoryManagerComponent(): TypedInventorySlotsLists(TypedInventorySlotsLists)
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
		TypedInventorySlotsLists = FInventorySlotsTypedArrayContainer(SlotTypesAndQuantities);
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

void UInventoryManagerComponent::AddItem(UInventoryItemInstance* Item, const int32 Index, const FGameplayTag Type)
{
	check(GetOwner()->HasAuthority());

	if (!ensureAlways(IsValid(Item)))
	{
		return;
	}
	
	// NOTE: There are suspicions of memory leaks if you install the Outer component due to bug UE-127172
	// (In Lyra, this is circumvented by setting the Outer component owner. You can also create a subsystem that will
	// control the life cycle of the object, which is more difficult but better from an architectural point of view)
	UInventoryItemInstance* ItemInstanceDuplicate = Item->Duplicate(this);
	
	TypedInventorySlotsLists.SetInstanceIntoSlot(ItemInstanceDuplicate, Index, Type);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstanceDuplicate)
	{
		AddReplicatedSubObject(ItemInstanceDuplicate);
	}

	OnInventoryContentChanged.Broadcast(this);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UInventoryManagerComponent::OnRep_TypedInventorySlotsLists()
{
	OnInventoryContentChanged.Broadcast(this);
}

void UInventoryManagerComponent::LogInventoryContent() const
{
	UE_LOG(LogTemp, Log, TEXT("================= UInventoryManagerComponent::OnRep_TypedInventorySlotsLists"));
	UE_LOG(LogTemp, Log, TEXT("Owner: %s"), *GetOwner()->GetName());

	for (const FInventorySlotsTypedArray& TypedInventorySlots : TypedInventorySlotsLists.GetArrays())
	{
		UE_LOG(LogTemp, Log, TEXT("--Start: %s"), *TypedInventorySlots.GetType().ToString());

		for (const FInventorySlot& Slot : TypedInventorySlots.GetArray().GetSlots())
		{
			UInventoryItemInstance* Item = Slot.GetInstance();

			if (!IsValid(Item))
			{
				UE_LOG(LogTemp, Log, TEXT("-"));
				continue;
			}
			
			UE_LOG(LogTemp, Log, TEXT("+ %s (%p)"), *Item->GetName(), Item);
		}
		
		UE_LOG(LogTemp, Log, TEXT("--End: %s"), *TypedInventorySlots.GetType().ToString());
	}
	
	UE_LOG(LogTemp, Log, TEXT("================="));
}
