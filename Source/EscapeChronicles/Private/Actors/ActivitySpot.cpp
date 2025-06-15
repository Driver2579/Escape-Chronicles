// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ActivitySpot.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/AttributeSets/VitalAttributeSet.h"
#include "ActorComponents/InventoryManagerComponent.h"
#include "Actors/DollOccupyingActivitySpot.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"
#include "Objects/InventoryItemInstance.h"
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
	MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

#if WITH_EDITORONLY_DATA
	CharacterTransformOnOccupySpotComponent = CreateEditorOnlyDefaultSubobject<UCapsuleComponent>(
		TEXT("Character Transform On Occupy Spot"));

	CharacterTransformOnOccupySpotComponent->SetupAttachment(MeshComponent);
#endif
}

#if WITH_EDITORONLY_DATA && WITH_EDITOR
void AActivitySpot::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CharacterTransformOnOccupySpot = CharacterTransformOnOccupySpotComponent->GetComponentTransform();
}
#endif

void AActivitySpot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CachedOccupyingCharacter);
	DOREPLIFETIME(ThisClass, CurrentOccupyingDollClass);
}

void AActivitySpot::BeginPlay()
{
	Super::BeginPlay();

	InteractableComponent->OnInteract.AddUObject(this, &ThisClass::OnInteract);
}

UAbilitySystemComponent* AActivitySpot::GetAbilitySystemComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CachedOccupyingCharacter);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void AActivitySpot::OnInteract(UInteractionManagerComponent* InteractionManagerComponent)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(
		InteractionManagerComponent->GetOwner());

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	/**
	 * If the spot is already occupied and if it's occupied by a doll, then try to move this doll into the character's
	 * inventory first. If it will be successfully moved, then the character would be able to occupy the spot after the
	 * second interaction.
	 */
	if (IsOccupied())
	{
		MoveOccupyingDollToCharacterInventory(Character);
	}
	// Otherwise, if the spot isn't occupying by anyone and anything, then try to occupy it with the character
	else
	{
		SetOccupyingCharacter(Character);
	}
}

void AActivitySpot::MoveOccupyingDollToCharacterInventory(const AEscapeChroniclesCharacter* Character)
{
	// Return if the spot isn't occupied by a doll
	if (CurrentOccupyingDollClass.IsNull())
	{
		return;
	}

#if DO_CHECK
	check(IsValid(Character));
#endif

	// If the class for the doll is still loading, then load it synchronously because we need it right now
	if (!CurrentOccupyingDollClass.IsValid())
	{
		CurrentOccupyingDollClass.LoadSynchronous();
	}

#if DO_CHECK
	// Since we finished loading the doll class, it's spawned actor must be valid at this stage
	check(SpawnedOccupyingDoll.IsValid());
#endif

	const UInventoryItemInstance* ItemInstance = SpawnedOccupyingDoll->GetItemInstance();

	/**
	 * We can't move the doll to the inventory if it doesn't have an item instance set. If an ItemInstance is valid,
	 * then try to add it to the first empty slot in the character's inventory. If it succeeds, then destroy the doll
	 * from this spot.
	 */
	if (ensureAlways(IsValid(ItemInstance)) && Character->GetInventoryManagerComponent()->AddItem(ItemInstance))
	{
		DestroyOccupyingDoll();
	}
}

bool AActivitySpot::SetOccupyingCharacter(AEscapeChroniclesCharacter* Character)
{
	/**
	 * Initial Validation:
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

	// === Actor State Preparation ===

	AEscapeChroniclesPlayerState* PlayerState;
	UAbilitySystemComponent* AbilitySystemComponent;

	// Branch for unoccupying (Character == nullptr)
	if (Character == nullptr)
	{
		// Return true if there is no cached character to unoccupy because it means the spot is already unoccupied
		if (!CachedOccupyingCharacter)
		{
			return true;
		}

		PlayerState = CachedOccupyingCharacter->GetPlayerState<AEscapeChroniclesPlayerState>();
		AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CachedOccupyingCharacter);
	}
	// Branch for new occupation
	else
	{
		PlayerState = Character->GetPlayerState<AEscapeChroniclesPlayerState>();
		AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Character);
	}

	if (!ensureAlways(IsValid(PlayerState)) || !ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return false;
	}

	const UVitalAttributeSet* VitalAttributeSet = AbilitySystemComponent->GetSet<UVitalAttributeSet>();

	if (!ensureAlways(VitalAttributeSet))
	{
		return false;
	}

	// ===== Occupation State Change =====

	if (Character == nullptr)
	{
		// Cleanup health monitoring
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalAttributeSet->GetHealthAttribute())
			.Remove(UnoccupyIfAttributeHasDecreasedDelegateHandle);

		UnoccupySpotWithCharacter(CachedOccupyingCharacter);
		PlayerState->SetOccupyingActivitySpot(nullptr);

		InteractableComponent->SetCanInteract(true);
	}
	// Check for blocking tags
	else if (!AbilitySystemComponent->HasAnyMatchingGameplayTags(OccupyingBlockedTags))
	{
		OccupySpotWithCharacter(Character);
		PlayerState->SetOccupyingActivitySpot(this);

		InteractableComponent->SetCanInteract(false);

		// Setup health monitoring
		UnoccupyIfAttributeHasDecreasedDelegateHandle =
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalAttributeSet->GetHealthAttribute())
				.AddUObject(this, &ThisClass::OnOccupyingCharacterHealthChanged);
	}
	else
	{
		return false;
	}

	CachedOccupyingCharacter = Character;

	OnOccupyingStateChanged.Broadcast(Character);

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
		UnoccupySpotWithCharacter(OldValue);
	}
	else
	{
		OccupySpotWithCharacter(CachedOccupyingCharacter);
	}
}

void AActivitySpot::OccupySpotWithCharacter(AEscapeChroniclesCharacter* Character)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();

	if (!ensureAlways(IsValid(CharacterMesh)))
	{
		return;
	}

	UEscapeChroniclesCharacterMoverComponent* CharacterMoverComponent = Character->GetCharacterMoverComponent();

	if (!ensureAlways(IsValid(CharacterMoverComponent)))
	{
		return;
	}

	// Destroy a currently occupying actor if any
	DestroyOccupyingDoll();

	/**
	 * Temporary and ugly solution. This function is a quick and dirty fix and should not exist in a properly designed
	 * architecture. Eventually, the MeshControllingState system must be refactored to eliminate the need for this.
	 */
	CharacterMoverComponent->SetDefaultMovementMode();
	Character->SetActorTransform(CharacterTransformOnOccupySpot);

	CharacterMesh->SetSimulatePhysics(false);
	CharacterMesh->SetUsingAbsoluteRotation(false);
	CharacterMesh->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	CharacterMesh->SetRelativeTransform(CharacterAttachTransform);

	// === Start async loading of assets ===

	LoadOccupyingAnimMontage();

	if (HasAuthority())
	{
		LoadOccupyingEffect();
	}
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
	
	if (OccupyingAnimMontageHandle.IsValid())
	{
		OnOccupyingAnimMontageLoaded();

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

	if (OccupyingEffectHandle.IsValid())
	{
		OnOccupyingEffectLoaded();

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

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return;
	}

	const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		OccupyingEffectClass.Get(), EffectLevel, AbilitySystemComponent->MakeEffectContext());

	OccupyingEffectSpecHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}

void AActivitySpot::UnoccupySpotWithCharacter(AEscapeChroniclesCharacter* Character)
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

	if (!CharacterMesh->BodyInstance.bSimulatePhysics)
	{
		ApplyInitialCharacterData(Character);
	}

	if (HasAuthority())
	{
		CancelOccupyingEffect(Character);
	}
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

void AActivitySpot::ApplyInitialCharacterData(AEscapeChroniclesCharacter* Character)
{
#if DO_CHECK
	check(IsValid(Character));
#endif

	USkeletalMeshComponent* SkeletalMesh = Character->GetMesh();
	
	if (!ensureAlways(IsValid(SkeletalMesh)))
	{
		return;
	}

	// Restore original mesh transform

	SkeletalMesh->AttachToComponent(Character->GetInitialMeshAttachParent(),
		FAttachmentTransformRules::KeepWorldTransform);

	SkeletalMesh->SetUsingAbsoluteRotation(true);

	SkeletalMesh->SetRelativeTransform(Character->GetInitialMeshTransform());
}

void AActivitySpot::GetOccupyingDollClass(TSoftClassPtr<ADollOccupyingActivitySpot>& OutOccupyingDollClass) const
{
	if (!CurrentOccupyingDollClass.IsNull())
	{
		OutOccupyingDollClass = CurrentOccupyingDollClass;
	}
	else
	{
		OutOccupyingDollClass.Reset();
	}
}

bool AActivitySpot::SpawnOccupyingDoll(const TSoftClassPtr<ADollOccupyingActivitySpot>& OccupyingDollClass,
	const FTransform* DollTransformOnOccupy)
{
	if (ensureAlways(!OccupyingDollClass.IsNull()) && HasAuthority() && !IsOccupied())
	{
		return SpawnOccupyingDollChecked(OccupyingDollClass, DollTransformOnOccupy);
	}

	return false;
}

bool AActivitySpot::SpawnOccupyingDollChecked(const TSoftClassPtr<ADollOccupyingActivitySpot>& OccupyingDollClass,
	const FTransform* DollTransformOnOccupy)
{
#if DO_ENSURE
	ensureAlways(!OccupyingDollClass.IsNull());
	ensureAlways(HasAuthority());
#endif

	/**
	 * Remember the class of the doll that occupies the spot. It isn't spawned yet, but we can already consider the
	 * spot as occupied by this doll.
	 */
	CurrentOccupyingDollClass = OccupyingDollClass;

	// Spawn a doll at the given transform or at the transform that was set for the character if none was provided
	CurrentOccupyingDollTransform = FTransform(
		DollTransformOnOccupy ? *DollTransformOnOccupy : CharacterAttachTransform);

	// Asynchronously load the class of the doll to spawn it at the spot and at the needed transform
	LoadOccupyingDollClassHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		CurrentOccupyingDollClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnOccupyingDollClassLoaded,
			CurrentOccupyingDollTransform));

	return true;
}

// ReSharper disable once CppPassValueParameterByConstReference
void AActivitySpot::OnOccupyingDollClassLoaded(const FTransform SpawnTransform)
{
	// === Spawn a doll with a zero transform without checking for collisions ===

#if DO_CHECK
	check(CurrentOccupyingDollClass.IsValid());
#endif

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedOccupyingDoll = GetWorld()->SpawnActor<ADollOccupyingActivitySpot>(CurrentOccupyingDollClass.Get(),
		SpawnParameters);

#if DO_CHECK
	check(SpawnedOccupyingDoll.IsValid());
#endif

	// === Attach a doll to the spot and set the given transform ===

	SpawnedOccupyingDoll->AttachToComponent(MeshComponent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	SpawnedOccupyingDoll->SetActorRelativeTransform(SpawnTransform, false, nullptr,
		ETeleportType::TeleportPhysics);
}

void AActivitySpot::DestroyOccupyingDoll()
{
	// Destroy a doll if it's valid
	if (SpawnedOccupyingDoll.IsValid())
	{
		SpawnedOccupyingDoll->Destroy();
		SpawnedOccupyingDoll.Reset();
	}

	// Unload the class of the doll or cancel its loading if it's still in progress
	if (LoadOccupyingDollClassHandle.IsValid())
	{
		LoadOccupyingDollClassHandle->CancelHandle();
	}

	// Reset the handle just in case
	LoadOccupyingDollClassHandle.Reset();

	// Forget the class of the doll
	CurrentOccupyingDollClass.Reset();

	// Reset the transform of the spawned doll
	CurrentOccupyingDollTransform = FTransform::Identity;
}

void AActivitySpot::OnPreLoadObject()
{
	// Always destroy an occupying doll if it exists. We will spawn a new one after the game is loaded if needed.
	DestroyOccupyingDoll();
}

void AActivitySpot::OnPostLoadObject()
{
	// Spawn an occupying doll if it was loaded
	if (!CurrentOccupyingDollClass.IsNull())
	{
		/**
		 * Set the occupying character to nullptr because we can't be sure the character was already loaded and
		 * unoccupied the spot itself.
		 */
		SetOccupyingCharacter(nullptr);

		// Spawn a doll at the loaded transform
		SpawnOccupyingDollChecked(CurrentOccupyingDollClass, &CurrentOccupyingDollTransform);
	}
}