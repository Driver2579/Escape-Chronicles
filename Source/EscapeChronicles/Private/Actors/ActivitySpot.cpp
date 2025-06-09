// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ActivitySpot.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/AttributeSets/VitalAttributeSet.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"
#include "Engine/AssetManager.h"
#include "GameModes/EscapeChroniclesGameMode.h"
#include "GameState/EscapeChroniclesGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

AActivitySpot::AActivitySpot()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	PlayerOwnershipComponent = CreateDefaultSubobject<UPlayerOwnershipComponent>(TEXT("PlayerOwnershipComponent"));
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->ComponentTags.Add(InteractableComponent->GetHintMeshTag());
}

void AActivitySpot::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
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

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}
	
	if (!IsValid(CachedOccupyingCharacter))
	{
		SetOccupyingCharacter(Character);
	}
	else if (CachedOccupyingCharacter == Character)
	{
		SetOccupyingCharacter(nullptr);
	}
}

bool AActivitySpot::SetOccupyingCharacter(AEscapeChroniclesCharacter* Character)
{
	check(HasAuthority());

	if (CachedOccupyingCharacter == Character)
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent = Character == nullptr ?
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CachedOccupyingCharacter) :
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);

	if (!IsValid(AbilitySystemComponent))
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
		// End track health changes
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalAttributeSet->GetHealthAttribute())
			.Remove(UnoccupyIfAttributeHasDecreasedDelegateHandle);
		
		UnoccupySpot(CachedOccupyingCharacter);
	}
	else
	{
		// Block the occupying if the character has a blocking tag
		if (AbilitySystemComponent->HasAnyMatchingGameplayTags(OccupyingBlockedTags))
		{
			return false;
		}
		
		OccupySpot(Character);

		// Start track health changes
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalAttributeSet->GetHealthAttribute())
			.AddUObject(this, &ThisClass::OnOccupyingCharacterHealthChanged);
	}

	CachedOccupyingCharacter = Character;

	OnOccupyingCharacterChanged.Broadcast(Character);

	return true;
}

void AActivitySpot::OnOccupyingCharacterHealthChanged(const FOnAttributeChangeData& AttributeChangeData)
{
	// We need to unoccupy the character if his health is decreasing
	if (AttributeChangeData.OldValue > AttributeChangeData.NewValue)
	{
		SetOccupyingCharacter(nullptr);	
	}
}

void AActivitySpot::OnRep_CachedOccupyingCharacter(AEscapeChroniclesCharacter* OldValue)
{
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
	const UCapsuleComponent* CharacterCapsule = Character->GetCapsuleComponent();
	const UEscapeChroniclesCharacterMoverComponent* CharacterMover = Character->GetCharacterMoverComponent();

#if DO_CHECK
	check(IsValid(CharacterMesh));
	check(IsValid(CharacterCapsule))
	check(IsValid(CharacterMover));
#endif

	// === Attach only the mesh to the desired location ===

	CachedMeshAttachParent = CharacterMesh->GetAttachParent();
	CachedMeshTransform = CharacterMesh->GetRelativeTransform();
	CharacterMesh->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);

	// === Load and apply animation and effect ===

	OccupyingEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		OccupyingEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&ThisClass::OnOccupyingEffectLoaded));

#if DO_CHECK
	check(OccupyingAnimMontages.Num() > 0);
#endif

	SelectedOccupyingAnimMontage = FMath::Rand() % OccupyingAnimMontages.Num();

	OccupyingAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		OccupyingAnimMontages[SelectedOccupyingAnimMontage].ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&ThisClass::OnOccupyingAnimMontageLoaded));
}

void AActivitySpot::UnoccupySpot(AEscapeChroniclesCharacter* Character)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	const UCapsuleComponent* CharacterCapsule = Character->GetCapsuleComponent();
	const UEscapeChroniclesCharacterMoverComponent* CharacterMover = Character->GetCharacterMoverComponent();

#if DO_CHECK
	check(IsValid(CharacterMesh));
	check(IsValid(CharacterCapsule))
	check(IsValid(CharacterMover));
#endif

	CancelOccupyingAnimationAndEffect(Character);

	// === Return the mesh to the state it was in before occupying spot ===

	CharacterMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	CharacterMesh->AttachToComponent(CachedMeshAttachParent.Get(), FAttachmentTransformRules::KeepWorldTransform);
	CharacterMesh->SetRelativeTransform(CachedMeshTransform);
}

void AActivitySpot::OnOccupyingAnimMontageLoaded()
{
	if (!ensureAlways(IsValid(CachedOccupyingCharacter)))
	{
		return;
	}

	const USkeletalMeshComponent* CharacterMesh = CachedOccupyingCharacter->GetMesh();

	if (!ensureAlways(IsValid(CharacterMesh)))
	{
		return;
	}

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();

	if (ensureAlways(IsValid(AnimInstance)))
	{
		AnimInstance->Montage_Play(OccupyingAnimMontages[SelectedOccupyingAnimMontage].Get());
	}
}

void AActivitySpot::OnOccupyingEffectLoaded()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		OccupyingEffectClass.Get(), EffectLevel, AbilitySystemComponent->MakeEffectContext());

	OccupyingEffectSpecHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}

void AActivitySpot::CancelOccupyingAnimationAndEffect(AEscapeChroniclesCharacter* Character)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	const USkeletalMeshComponent* CharacterMesh = Character->GetMesh();

	if (!ensureAlways(IsValid(CharacterMesh)))
	{
		return;
	}

	// === Stop animation montage ===
	
	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();

	if (!ensureAlways(IsValid(AnimInstance)))
	{
		return;
	}

	AnimInstance->Montage_Stop(0.0, OccupyingAnimMontages[SelectedOccupyingAnimMontage].Get());

	// === Stop effect ===

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return;
	}

	AbilitySystemComponent->RemoveActiveGameplayEffect(OccupyingEffectSpecHandle);
	
	// === Releasing the memory ===

	if (OccupyingAnimMontageHandle.IsValid())
	{
		OccupyingAnimMontageHandle->CancelHandle();
		OccupyingAnimMontageHandle.Reset();
	}

	if (OccupyingEffectHandle.IsValid())
	{
		OccupyingEffectHandle->CancelHandle();
		OccupyingEffectHandle.Reset();
	}
}
