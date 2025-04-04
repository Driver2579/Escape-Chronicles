// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/InventoryManagerComponent.h"

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

void UInventoryManagerComponent::AddItem(UInventoryItemInstance* Item, FGameplayTag Type, int32 SlotIndex)
{
	if (GetOwner()->HasAuthority())
	{
		UInventoryItemInstance* NewItemInstance = NewObject<UInventoryItemInstance>(this);
		
		AddReplicatedSubObject(NewItemInstance);
		
		FInventorySlotsTypedArray* InventorySlotsTypedArray = TypedInventorySlotsLists.TypedLists.FindByPredicate(
			[Type](const FInventorySlotsTypedArray& List)
			{
				return List.Type == Type;
			});

		if (!InventorySlotsTypedArray)
		{
			UE_LOG(LogTemp, Log, TEXT("Not Found"));
			return;
		}

		if (SlotIndex > InventorySlotsTypedArray->List.Slots.Num() - 1 || SlotIndex < 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Unavailable slot number"));
			return;
		}
		
		if (InventorySlotsTypedArray->List.Slots[SlotIndex].Instance != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Slot is not empty"));
			return;
		}
		
		InventorySlotsTypedArray->List.Slots[SlotIndex].Instance = NewItemInstance;

		InventorySlotsTypedArray->List.MarkItemDirty(InventorySlotsTypedArray->List.Slots[SlotIndex]);
		TypedInventorySlotsLists.MarkItemDirty(*InventorySlotsTypedArray);
	}
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
		for (const auto& SlotTypeAndQuantity : SlotTypesAndQuantities)
		{
			FInventorySlotsTypedArray TypedList;

			TypedList.Type = SlotTypeAndQuantity.Key;

			// Создаем список слотов для данного типа
			FInventorySlotsArray InventorySlotsList;
			for (int32 i = 0; i < SlotTypeAndQuantity.Value; ++i)
			{
				// Добавляем пустой слот в список
				FInventorySlot Slot = FInventorySlot();
				InventorySlotsList.Slots.Add(Slot);
			}

			// Присваиваем созданный список слотов в FInventorySlotsTypedList
			TypedList.List = InventorySlotsList;

			// Добавляем типизированный список слотов в контейнер
			TypedInventorySlotsLists.TypedLists.Add(TypedList);
		}

		TypedInventorySlotsLists.MarkArrayDirty();
	}

	if (GetOwner()->HasAuthority())
	{
		TObjectPtr<UInventoryItemInstance> NewItemInstance = NewObject<UInventoryItemInstance>(this);
		NewItemInstance->SetItemDefinition(UInventoryItemDefinition::StaticClass());
		
		FGameplayTag WeaponTag = FGameplayTag::RequestGameplayTag(FName("Inventory.SlotTypes.Main"));

		AddItem(NewItemInstance, WeaponTag, 1);
	}

	bInventoryInitialized = true;
	OnInventoryInitialized.Broadcast(this);
}


void UInventoryManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Tick 1"));
	}
	else if (GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("Tick 2"));
	}
}

