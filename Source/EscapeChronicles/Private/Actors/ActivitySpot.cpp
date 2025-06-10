// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ActivitySpot.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/AttributeSets/VitalAttributeSet.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "Engine/AssetManager.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AActivitySpot::AActivitySpot()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root"));

	PlayerOwnershipComponent = CreateDefaultSubobject<UPlayerOwnershipComponent>(TEXT("Player Ownership"));
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("Interactable"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->ComponentTags.Add(InteractableComponent->GetHintMeshTag());

#if WITH_EDITORONLY_DATA && WITH_EDITOR
	CharacterTransformOnOccupySpotComponent = CreateDefaultSubobject<UCapsuleComponent>(
		TEXT("Character Transform On Occupy Spot"));

	CharacterTransformOnOccupySpotComponent->SetupAttachment(MeshComponent);
#endif
}

#if WITH_EDITORONLY_DATA && WITH_EDITOR
void AActivitySpot::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const FTransform& CharacterTransformOnOccupySpot = CharacterTransformOnOccupySpotComponent->GetComponentTransform();

	CharacterLocationOnOccupySpot = CharacterTransformOnOccupySpot.GetLocation();
	CharacterRotationOnOccupySpot = CharacterTransformOnOccupySpot.GetRotation().Rotator();
}
#endif

void AActivitySpot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CachedOccupyingCharacter)
}

void AActivitySpot::BeginPlay()
{
	Super::BeginPlay();

	AEscapeChroniclesGameMode* GameMode = GetWorld()->GetAuthGameMode<AEscapeChroniclesGameMode>();

	if (IsValid(GameMode))
	{
		const FUniquePlayerID* OwningPlayerID = PlayerOwnershipComponent->GetOwningPlayer();

		/**
		 * Call the OnPlayerOrBotInitialized function for the owning player or bot immediately if this owner is already
		 * set and this player or bot is already fully initialized.
		 */
		if (OwningPlayerID)
		{
			bool bLoaded;

			if (GameMode->IsPlayerOrBotFullyInitialized(*OwningPlayerID, bLoaded))
			{
				AEscapeChroniclesGameState* GameState = GameMode->GetGameState<AEscapeChroniclesGameState>();

				for (APlayerState* PlayerState : GameState->PlayerArray)
				{
					AEscapeChroniclesPlayerState* CastedPlayerState = Cast<AEscapeChroniclesPlayerState>(PlayerState);

					if (IsValid(CastedPlayerState) && CastedPlayerState->GetUniquePlayerID() == *OwningPlayerID)
					{
						/**
						 * Call the OnPlayerOrBotInitialized function with found PlayerState and whether the player or
						 * bot was loaded at least once.
						 */
						OnPlayerOrBotInitialized(CastedPlayerState, bLoaded);

						// Break the loop since we found the player or bot that owns this spot
						break;
					}
				}
			}
		}

		/**
		 * Listen for the OnPlayerOrBotInitialized delegate regardless of whether we already called its callback or not
		 * because this function is virtual and may be used by child classes for whatever purpose.
		 */
		GameMode->OnPlayerOrBotInitialized.AddUObject(this, &ThisClass::OnPlayerOrBotInitialized);
	}

	InteractableComponent->OnInteract.AddUObject(this, &ThisClass::OnInteract);
}

UAbilitySystemComponent* AActivitySpot::GetAbilitySystemComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CachedOccupyingCharacter);
}

void AActivitySpot::OnPlayerOrBotInitialized(AEscapeChroniclesPlayerState* PlayerState, const bool bLoaded)
{
	/**
	 * Don't do anything if the player or bot shouldn't occupy this spot when he first joined the game or if the player
	 * or bot was loaded (which means that it isn't the first time he joined the game).
	 */
	if (!bOccupyWhenOwningPlayerFirstJoined || bLoaded)
	{
		return;
	}

#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	const FUniquePlayerID* OwningPlayerID = PlayerOwnershipComponent->GetOwningPlayer();

	/**
	 * Don't do anything if this spot doesn't have an owning player or if this function was called for a different
	 * player or bot that was initialized.
	 *
	 * P.S.: The PlayerOwnershipComponents for this player or bot are guaranteed to be initialized by the time this
	 * function is called (of course, if any PlayerOwnershipComponent should be owned by this player or bot).
	 */
	if (!OwningPlayerID || *OwningPlayerID != PlayerState->GetUniquePlayerID())
	{
		return;
	}

#if DO_CHECK
	// The PlayerState must have a valid character by now
	check(IsValid(PlayerState->GetPawn()));
	check(PlayerState->GetPawn()->IsA<AEscapeChroniclesCharacter>());
#endif

	/**
	 * Try to set the character of the initialized player or bot as an occupying character for this spot if we've passed
	 * all the checks above.
	 */
	SetOccupyingCharacter(CastChecked<AEscapeChroniclesCharacter>(PlayerState->GetPawn()));
}

void AActivitySpot::OnInteract(UInteractionManagerComponent* InteractionManagerComponent)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(
		InteractionManagerComponent->GetOwner());

	if (!ensureAlways(IsValid(Character)) || CachedOccupyingCharacter != nullptr)
	{
		return;
	}

	AEscapeChroniclesPlayerState* PlayerState = Character->GetPlayerState<AEscapeChroniclesPlayerState>();

	if (ensureAlways(IsValid(PlayerState)) && SetOccupyingCharacter(Character))
	{
		PlayerState->SetOccupyingActivitySpot(this);
	}
}

bool AActivitySpot::SetOccupyingCharacter(AEscapeChroniclesCharacter* Character)
{
	/**
	 * - Server-only.
	 * - Must not be trying to reassign the same character.
	 * - Allows unoccupying (if Character == nullptr).
	 * - Prevents assigning if already occupied.
	 */
	const bool bSuccessfulInitialCheck = ensureAlways(HasAuthority()) && CachedOccupyingCharacter != Character &&
		CachedOccupyingCharacter == nullptr || Character == nullptr;

	if (!bSuccessfulInitialCheck)
	{
		return false;
	}

	/**
	 * Get the appropriate ability system component:
	 * - When unoccupying (Character == nullptr), use the currently occupying character.
	 * - When occupying, use the character that is trying to occupy the spot.
	 */
	UAbilitySystemComponent* AbilitySystemComponent = Character == nullptr ?
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CachedOccupyingCharacter) :
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return false;
	}

	const UVitalAttributeSet* VitalAttributeSet = AbilitySystemComponent->GetSet<UVitalAttributeSet>();

	if (!ensureAlways(IsValid(VitalAttributeSet)))
	{
		return false;
	}

	if (Character == nullptr)
	{
		// Cleanup health monitoring
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalAttributeSet->GetHealthAttribute())
			.Remove(UnoccupyIfAttributeHasDecreasedDelegateHandle);

		UnoccupySpot(CachedOccupyingCharacter);

		InteractableComponent->SetCanInteract(true);
	}
	else
	{
		// Check for blocking tags
		if (AbilitySystemComponent->HasAnyMatchingGameplayTags(OccupyingBlockedTags))
		{
			return false;
		}

		OccupySpot(Character);

		InteractableComponent->SetCanInteract(false);

		// Setup health monitoring
		UnoccupyIfAttributeHasDecreasedDelegateHandle =
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalAttributeSet->GetHealthAttribute())
				.AddUObject(this, &ThisClass::OnOccupyingCharacterHealthChanged);
	}

	CachedOccupyingCharacter = Character;

	OnOccupyingCharacterChanged.Broadcast(Character);

	return true;
}

void AActivitySpot::OnOccupyingCharacterHealthChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	// Unoccupy if health decreased
	if (AttributeChangeData.OldValue > AttributeChangeData.NewValue)
	{
		SetOccupyingCharacter(nullptr);
	}
}

void AActivitySpot::OnRep_CachedOccupyingCharacter(AEscapeChroniclesCharacter* OldValue)
{
	// Handle occupation change on clients
	if (CachedOccupyingCharacter == nullptr)
	{
		UnoccupySpot(OldValue);
	}
	else
	{
		OccupySpot(CachedOccupyingCharacter);
	}
}

void AActivitySpot::OccupySpot(AEscapeChroniclesCharacter* Character)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();

	if (!ensureAlways(IsValid(CharacterMesh)))
	{
		return;
	}

	CacheMeshData(CharacterMesh);

	CharacterMesh->SetUsingAbsoluteRotation(false);

	// Move the character
	Character->SetActorLocation(CharacterLocationOnOccupySpot);
	Character->SetActorRotation(CharacterRotationOnOccupySpot);

	// === Start async loading of assets ===

	LoadOccupyingAnimMontage();

	if (HasAuthority())
	{
		LoadOccupyingEffect();
	}
}

void AActivitySpot::CacheMeshData(const USkeletalMeshComponent* SkeletalMesh)
{
#if DO_CHECK
	check(IsValid(SkeletalMesh));
#endif

	// Remember original mesh state for later restoration
	CachedMeshAttachParent = SkeletalMesh->GetAttachParent();
	CachedMeshTransform = SkeletalMesh->GetRelativeTransform();
}

void AActivitySpot::LoadOccupyingAnimMontage()
{
	if (!ensureAlways(OccupyingAnimMontages.Num() > 0))
	{
		return;
	}

	// Randomly select and load animation

	SelectedOccupyingAnimMontage = FMath::Rand() % OccupyingAnimMontages.Num();

	if (!ensureAlways(!OccupyingAnimMontages[SelectedOccupyingAnimMontage].IsNull()))
	{
		return;
	}

	OccupyingAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		OccupyingAnimMontages[SelectedOccupyingAnimMontage].ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnOccupyingAnimMontageLoaded));
}

void AActivitySpot::LoadOccupyingEffect()
{
	if (!ensureAlways(!OccupyingEffectClass.IsNull()))
	{
		return;
	}

	// Load gameplay effect asynchronously
	OccupyingEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		OccupyingEffectClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnOccupyingEffectLoaded));
}

void AActivitySpot::OnOccupyingAnimMontageLoaded()
{
	if (!ensureAlways(IsValid(CachedOccupyingCharacter)))
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = CachedOccupyingCharacter->GetMesh();

	if (!ensureAlways(IsValid(CharacterMesh)))
	{
		return;
	}

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();

	if (ensureAlways(IsValid(AnimInstance)))
	{
		AnimInstance->Montage_Play(OccupyingAnimMontages[SelectedOccupyingAnimMontage].Get());

		CharacterMesh->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			AttachSocketName);
	}
}

void AActivitySpot::OnOccupyingEffectLoaded()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return;
	}

	const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		OccupyingEffectClass.Get(), EffectLevel, AbilitySystemComponent->MakeEffectContext());

	OccupyingEffectSpecHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}

void AActivitySpot::UnoccupySpot(AEscapeChroniclesCharacter* Character)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();

	if (!ensureAlways(IsValid(CharacterMesh)))
	{
		return;
	}

	// === Restore character to original state ===

	CancelOccupyingAnimation(CharacterMesh);

	if (HasAuthority())
	{
		CancelOccupyingEffect(Character);
	}

	ApplyCachedMeshData(CharacterMesh);
}

void AActivitySpot::CancelOccupyingAnimation(const USkeletalMeshComponent* CharacterMesh)
{
#if DO_CHECK
	check(IsValid(CharacterMesh));
#endif

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();

	if (!ensureAlways(IsValid(AnimInstance)))
	{
		return;
	}

	AnimInstance->Montage_Stop(0.0, OccupyingAnimMontages[SelectedOccupyingAnimMontage].Get());

	// Cleanup loading handle
	if (OccupyingAnimMontageHandle.IsValid())
	{
		OccupyingAnimMontageHandle->CancelHandle();
		OccupyingAnimMontageHandle.Reset();
	}
}

void AActivitySpot::CancelOccupyingEffect(const AEscapeChroniclesCharacter* Character)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return;
	}

	// Remove gameplay effect
	AbilitySystemComponent->RemoveActiveGameplayEffect(OccupyingEffectSpecHandle);

	// Cleanup loading handle
	if (OccupyingEffectHandle.IsValid())
	{
		OccupyingEffectHandle->CancelHandle();
		OccupyingEffectHandle.Reset();
	}
}

void AActivitySpot::ApplyCachedMeshData(USkeletalMeshComponent* SkeletalMesh) const
{
#if DO_CHECK
	check(IsValid(SkeletalMesh));
	check(CachedMeshAttachParent.IsValid());
#endif

	// Restore original mesh transform
	SkeletalMesh->AttachToComponent(CachedMeshAttachParent.Get(), FAttachmentTransformRules::KeepWorldTransform);
	SkeletalMesh->SetRelativeTransform(CachedMeshTransform);
	SkeletalMesh->SetUsingAbsoluteRotation(true);
}
