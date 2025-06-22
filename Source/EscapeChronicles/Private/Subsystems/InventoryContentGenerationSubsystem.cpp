// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/InventoryContentGenerationSubsystem.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/DataAssets/InventoryContentGeneratingData.h"
#include "Engine/AssetManager.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

bool UInventoryContentGenerationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return this->GetClass()->IsInBlueprint() && Super::ShouldCreateSubsystem(Outer);
}

void UInventoryContentGenerationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	AEscapeChroniclesGameMode* GameMode = Cast<AEscapeChroniclesGameMode>(GetWorld()->GetAuthGameMode());

	if (!IsValid(GameMode))
	{
		return;
	}

	GameMode->OnPlayerOrBotInitialized.AddWeakLambda(this,
		[this](AEscapeChroniclesPlayerState* PlayerState, const bool bLoaded)
		{
			if (!DataTableOwnerships.Contains(PlayerState->GetUniquePlayerID())) AssignGeneratingData(PlayerState);
		});
}

int32 UInventoryContentGenerationSubsystem::GetCurrentDataTableOwnersNumber(const TSoftObjectPtr<UDataTable>& DataTable) const
{
	int32 Result = 0;

	for (const auto& Pair : DataTableOwnerships)
	{
		if (Pair.Value == DataTable) Result++;
	}

	return Result;
}

void UInventoryContentGenerationSubsystem::AssignGeneratingData(const AEscapeChroniclesPlayerState* PlayerState)
{
	const AEscapeChroniclesCharacter* Character = PlayerState->GetPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	for (const auto& Entry : GenerationEntries)
	{
		const bool bSuitableEntry = Character->IsA(Entry.Key) &&
			Entry.Value.AvailableNumber > GetCurrentDataTableOwnersNumber(Entry.Value.InventoryContentGeneratingData);

		if (!bSuitableEntry) continue;

		DataTableOwnerships.Add(PlayerState->GetUniquePlayerID(), Entry.Value.InventoryContentGeneratingData);

		GenerateInventoryContent(PlayerState);

		return;
	}
}

void UInventoryContentGenerationSubsystem::GenerateInventoryContent(const AEscapeChroniclesPlayerState* PlayerState)
{
	const AEscapeChroniclesCharacter* Character = PlayerState->GetPawn<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	UInventoryManagerComponent* Inventory = Character->GetInventoryManagerComponent();

	if (!ensureAlways(IsValid(Inventory)))
	{
		return;
	}

	const FUniquePlayerID& UniquePlayerID = PlayerState->GetUniquePlayerID();

#if DO_CHECK
	check(DataTableOwnerships.Contains(UniquePlayerID));
#endif

	const TSoftObjectPtr<UDataTable> DataTable= DataTableOwnerships[UniquePlayerID];

	UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(DataTable.ToSoftObjectPath(),
		FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnDataTableLoaded, DataTable, Inventory));
}

void UInventoryContentGenerationSubsystem::OnDataTableLoaded(TSharedPtr<FStreamableHandle> Handle,
	const TSoftObjectPtr<UDataTable> DataTable, UInventoryManagerComponent* Inventory)
{
	// Retrieve all loot table rows
	TArray<FInventoryContentGeneratingData*> GeneratingContent;
	DataTable->GetAllRows("", GeneratingContent);

	UE_LOG(LogTemp, Error, TEXT("InventoryContentGenerationSubsystem::OnDataTableLoaded"));
	/*// Process each loot table entry
	for (const FInventoryContentGeneratingData* Row : GeneratingContent)
	{
		// Skip if probability check fails
		if (FMath::Rand() < Row->Probability)
		{
			continue;
		}

		// Determine quantity to spawn
		const int32 Number = FMath::RandRange(Row->MinNumber, Row->MaxNumber);
		
		for (int32 Index = 0; Index < Number; Index++)
		{
			UInventoryItemInstance* ItemInstance = NewObject<UInventoryItemInstance>();
			ItemInstance->Initialize(Row->ItemDefinition);

			// Apply stat overrides
			for (const FInstanceStatsItem& Stat : Row->InstanceStatsOverride.GetAllStats())
			{
				ItemInstance->GetInstanceStats_Mutable().SetStat(Stat);
			}

			Inventory->AddItem(ItemInstance);
		}
	}*/

	if (Handle.IsValid())
	{
		Handle->CancelHandle();
		Handle.Reset();
	}
}