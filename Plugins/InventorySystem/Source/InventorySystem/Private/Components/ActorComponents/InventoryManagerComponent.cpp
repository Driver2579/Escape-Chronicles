// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/InventoryManagerComponent.h"

#include "InventorySystemGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemDefinition.h"
#include "Objects/InventoryItemInstance.h"

UInventoryManagerComponent::UInventoryManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bReplicateUsingRegisteredSubObjectList = true;
	SetIsReplicatedByDefault(true);
}

void UInventoryManagerComponent::CallOrRegisterOnInventoryInitialized(
	const FOnInventoryInitializedDelegate::FDelegate& Callback)
{
	if (bInventoryInitialized)
	{
		Callback.Execute(this);
	}
	else
	{
		OnInventoryInitialized.Add(Callback);
	}
}

/*
void UInventoryManagerComponent::AddItem(UInventoryItemInstance* Item, int32 Index)
{
	AddItem(Item, Index, InventorySystemGameplayTags::InventoryTag_MainSlotType);
}
*/

void UInventoryManagerComponent::AddItem(UInventoryItemInstance* Item, int32 Index, FGameplayTag Type)
{
	check(GetOwner()->HasAuthority());
  
	// TODO: UE-127172
	UInventoryItemInstance* ItemInstanceDuplicate = Item->Duplicate(this);
	
	TypedInventorySlotsLists.SetInstanceIntoSlot(ItemInstanceDuplicate, Index, Type);
	
	AddReplicatedSubObject(ItemInstanceDuplicate);
}


// Called when the game starts
void UInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bInventoryInitialized && IsReadyForReplication())
	{
		InitializeInventory();
	}
}

void UInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (!bInventoryInitialized)
	{
		InitializeInventory();
	}
}

void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, TypedInventorySlotsLists);
}

void UInventoryManagerComponent::InitializeInventory()
{
	if (GetOwner()->HasAuthority())
	{
		TypedInventorySlotsLists.Init(SlotTypesAndQuantities);
	}

	if (GetOwner()->HasAuthority())
	{
		TObjectPtr<UInventoryItemInstance> NewItemInstance = NewObject<UInventoryItemInstance>(this);
		NewItemInstance->SetItemDefinition(UInventoryItemDefinition::StaticClass());
		
		AddItem(NewItemInstance, 1);
	}

	bInventoryInitialized = true;
	OnInventoryInitialized.Broadcast(this);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UInventoryManagerComponent::OnRep_TypedInventorySlotsLists()
{
	UE_LOG(LogTemp, Log, TEXT("================= UInventoryManagerComponent::OnRep_TypedInventorySlotsLists"));
	
	UE_LOG(LogTemp, Log, TEXT("Owner: %s"), *GetOwner()->GetName());

	for (const FInventorySlotsTypedArray& TypedInventorySlots : TypedInventorySlotsLists.GetTypedLists())
	{
		UE_LOG(LogTemp, Log, TEXT("--Start: %s"), *TypedInventorySlots.GetType().ToString());

		for (const FInventorySlot& Slot : TypedInventorySlots.GetList().GetSlots())
		{
			UInventoryItemInstance* Item = Slot.GetInstance();

			if (!IsValid(Item))
			{
				UE_LOG(LogTemp, Log, TEXT("-"));
				continue;
			}
			
			UE_LOG(LogTemp, Log, TEXT("I:%s (%p)"), *Item->GetName(), Item);
		}
		
		UE_LOG(LogTemp, Log, TEXT("--End: %s"), *TypedInventorySlots.GetType().ToString());
	}
	
	UE_LOG(LogTemp, Log, TEXT("================="));
}
