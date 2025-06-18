// Fill out your copyright notice in the Description page of Project Settings.

#include "Objects/InventoryItemFragments/HoldingViewInventoryItemFragment.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Engine/AssetManager.h"
#include "Objects/InventoryItemInstance.h"

void UHoldingViewInventoryItemFragment::StartHolding(UInventoryItemInstance* ItemInstance)
{
#if DO_CHECK
	check(IsValid(ItemInstance));
#endif

	if (!ensureAlways(!HoldingViewData.Contains(ItemInstance)))
	{
		return;
	}

	const TSharedPtr<FStreamableHandle> LoadedHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
		ItemInstanceViewClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::ItemInstanceViewClassLoaded, ItemInstance));

	FHoldingViewDataItem HoldingViewDataItem;
	HoldingViewDataItem.StreamableHandle = LoadedHandle;

	HoldingViewData.Add(ItemInstance, HoldingViewDataItem);
}

void UHoldingViewInventoryItemFragment::ItemInstanceViewClassLoaded(UInventoryItemInstance* ItemInstance)
{
#if DO_CHECK
	check(IsValid(ItemInstance));
#endif
	
	FHoldingViewDataItem& HoldingViewDataItem = HoldingViewData[ItemInstance];

	const UInventoryManagerComponent* Inventory = Cast<UInventoryManagerComponent>(ItemInstance->GetOuter());

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	const AActor* Owner = Inventory->GetOwner();

	if (!ensureAlways(IsValid(Owner)))
	{
		return;
	}

	TInlineComponentArray<USceneComponent*> OwnerSceneComponents;
	Owner->GetComponents(USceneComponent::StaticClass(), OwnerSceneComponents);

	for (USceneComponent* Component : OwnerSceneComponents)
	{
		if (IsValid(Component) && Component->ComponentHasTag(ComponentToAttachTag))
		{
			AActor* SpawnedActor = ItemInstance->GetWorld()->SpawnActor<AActor>(ItemInstanceViewClass.Get());
			HoldingViewDataItem.SpawnedActor = SpawnedActor;

			SpawnedActor->AttachToComponent(Component, FAttachmentTransformRules::KeepRelativeTransform,
				SocketToAttach);

			SpawnedActor->SetActorRelativeTransform(TransformToAttach);

			return;
		}
	}

#if DO_ENSURE
	ensureAlwaysMsgf(false, TEXT("No matching component. Perhaps you forgot to set the tag."));
#endif
}

void UHoldingViewInventoryItemFragment::StopHolding(UInventoryItemInstance* ItemInstance)
{
#if DO_CHECK
	check(IsValid(ItemInstance));
#endif

	FHoldingViewDataItem& HoldingViewDataItem = HoldingViewData[ItemInstance];

	if (ensureAlways(HoldingViewDataItem.SpawnedActor))
	{
		HoldingViewDataItem.SpawnedActor->Destroy();
	}

	if (ensureAlways(HoldingViewDataItem.StreamableHandle.IsValid()))
	{
		HoldingViewDataItem.StreamableHandle->CancelHandle();
		HoldingViewDataItem.StreamableHandle.Reset();
	}

	HoldingViewData.Remove(ItemInstance);
}
