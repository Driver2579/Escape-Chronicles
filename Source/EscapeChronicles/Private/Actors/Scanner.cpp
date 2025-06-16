// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Scanner.h"

#include "AbilitySystemComponent.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Common/Enums/ItemClassification.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/AssetManager.h"
#include "Objects/InventoryItemFragments/ClassificationInventoryItemFragment.h"
#include "Objects/InventoryItemFragments/ContrabandBagInventoryItemFragment.h"
#include "Objects/InventoryItemFragments/DurabilityInventoryItemFragment.h"

AScanner::AScanner()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);

	TriggerComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	TriggerComponent->SetupAttachment(RootComponent);

	TriggerAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Trigger Audio"));
	TriggerAudioComponent->SetupAttachment(RootComponent);
	TriggerAudioComponent->SetAutoActivate(false);

	bReplicates = true;
}

void AScanner::BeginPlay()
{
	Super::BeginPlay();

	TriggerComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnTriggerComponentBeginOverlap);
}

void AScanner::OnTriggerComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// The next logic won't work on clients, so there is no point to waste resources on it
	if (!HasAuthority())
	{
		return;
	}

	const AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(OtherActor);

	/**
	 * Don't do anything if the overlapping actor isn't a character or if the overlapping component isn't the
	 * character's capsule.
	 */
	if (!IsValid(Character) || OtherComp != Character->GetCapsuleComponent())
	{
		return;
	}

	// Don't do anything if immunity tags are specified and the character has any of them
	if (!ImmunityTags.IsEmpty())
	{
		const UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

		const bool bCharacterHasImmunity = IsValid(AbilitySystemComponent) &&
			AbilitySystemComponent->HasAnyMatchingGameplayTags(ImmunityTags);

		if (bCharacterHasImmunity)
		{
			return;
		}
	}

	bool bHasContrabandItem = false;

	// Try to find a contraband item and a contraband bag in the character's inventory
	for (const FInventorySlotsTypedArray& Slots :
		Character->GetInventoryManagerComponent()->GetInventoryContent().GetItems())
	{
		for (const FInventorySlot& Slot : Slots.Array.GetItems())
		{
			// Skip empty slots
			if (!Slot.Instance)
			{
				continue;
			}

			const UContrabandBagInventoryItemFragment* ContrabandBagItemFragment = Slot.Instance->GetFragmentByClass<
				UContrabandBagInventoryItemFragment>();

			// Return if the character has a contraband bag. We can't do anything with him in that case.
			if (IsValid(ContrabandBagItemFragment))
			{
				const UDurabilityInventoryItemFragment* DurabilityItemFragment = Slot.Instance->GetFragmentByClass<
					UDurabilityInventoryItemFragment>();

				// Reduce the durability of the contraband bag item if it has a durability fragment
				if (IsValid(DurabilityItemFragment))
				{
					DurabilityItemFragment->ReduceDurability(Slot.Instance);
				}

				return;
			}

			// Check if the item is contraband if we didn't find any yet
			if (!bHasContrabandItem)
			{
				const UClassificationInventoryItemFragment* ClassificationItemFragment =
					Slot.Instance->GetFragmentByClass<UClassificationInventoryItemFragment>();

				bHasContrabandItem = IsValid(ClassificationItemFragment) &&
					ClassificationItemFragment->GetItemClassification() == EItemClassification::Contraband;
			}
		}
	}

	// Don't do anything if the character doesn't have any contraband items
	if (!bHasContrabandItem)
	{
		return;
	}

	/**
	 * Since the character doesn't have a tag immunity, has a contraband item, and doesn't have a contraband bag, we
	 * need to apply the gameplay effect to him. Load the gameplay effect class now to do that.
	 */
	UAssetManager::GetStreamableManager().RequestAsyncLoad(GameplayEffectClass.ToSoftObjectPath(), 
		FStreamableDelegateWithHandle::CreateUObject(this, &ThisClass::OnGameplayEffectClassLoaded,
			Character));

	// Play the scanner sound effect both on server and all clients
	NetMulticast_PlayTriggerAudioComponent();
}

// ReSharper disable once CppPassValueParameterByConstReference
void AScanner::OnGameplayEffectClassLoaded(TSharedPtr<FStreamableHandle> LoadObjectHandle,
	const AEscapeChroniclesCharacter* Character) const
{
#if DO_CHECK
	check(LoadObjectHandle.IsValid());
#endif

	// Ensure the character didn't become invalid while we were loading the gameplay effect class
	if (!ensureAlways(IsValid(Character)))
	{
		// Unload the gameplay effect class as we don't need it anymore
		LoadObjectHandle->ReleaseHandle();

		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();

	// Ensure the character has a valid ability system component
	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		// Unload the gameplay effect class as we don't need it anymore
		LoadObjectHandle->ReleaseHandle();

		return;
	}

#if DO_CHECK
	check(GameplayEffectClass.IsValid());
#endif

	// Apply the loaded gameplay effect to the character
	AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffectClass->GetDefaultObject<UGameplayEffect>(),
		UGameplayEffect::INVALID_LEVEL, AbilitySystemComponent->MakeEffectContext());

	// Release the handle to the gameplay effect class as we don't need it anymore
	LoadObjectHandle->ReleaseHandle();
}

void AScanner::NetMulticast_PlayTriggerAudioComponent_Implementation() const
{
	TriggerAudioComponent->Play();
}