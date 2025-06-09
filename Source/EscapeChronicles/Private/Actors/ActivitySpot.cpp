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
#include "Net/UnrealNetwork.h"

AActivitySpot::AActivitySpot()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

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

	InteractableComponent->OnInteract.AddUObject(this, &ThisClass::OnInteract);
}

UAbilitySystemComponent* AActivitySpot::GetAbilitySystemComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CachedOccupyingCharacter);
}

void AActivitySpot::OnInteract(UInteractionManagerComponent* InteractionManagerComponent)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(
		InteractionManagerComponent->GetOwner());

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	// === Toggle occupation state ===

	// Occupy if empty
	if (!IsValid(CachedOccupyingCharacter))
	{
		SetOccupyingCharacter(Character);
	}
	// Unoccupy if same character
	else if (CachedOccupyingCharacter == Character)
	{
		SetOccupyingCharacter(nullptr);
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
	}
	else
	{
		// Check for blocking tags
		if (AbilitySystemComponent->HasAnyMatchingGameplayTags(OccupyingBlockedTags))
		{
			return false;
		}

		OccupySpot(Character);

		// Setup health monitoring
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

	const USkeletalMeshComponent* CharacterMesh = Character->GetMesh();

	if (!ensureAlways(IsValid(CharacterMesh)))
	{
		return;
	}

	CacheMeshData(CharacterMesh);

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
		AttachOccupyingCharacterMesh();
		
		return;
	}

	// Randomly select and load animation

	SelectedOccupyingAnimMontage = FMath::Rand() % OccupyingAnimMontages.Num();

	if (!ensureAlways(!OccupyingAnimMontages[SelectedOccupyingAnimMontage].IsNull()))
	{
		AttachOccupyingCharacterMesh();

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

	if (!ensureAlways(IsValid(AnimInstance)))
	{
		return;
	}

	AnimInstance->Montage_Play(OccupyingAnimMontages[SelectedOccupyingAnimMontage].Get());

	AttachSkeletalMesh(CharacterMesh);
}

void AActivitySpot::AttachSkeletalMesh(USkeletalMeshComponent* SkeletalMesh) const
{
	// Set this false to make the mesh rotate when attach it
	SkeletalMesh->SetUsingAbsoluteRotation(false);

	// Attach only the mesh to the desired location
	SkeletalMesh->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		AttachSocketName);
}

void AActivitySpot::AttachOccupyingCharacterMesh() const
{
	if (!ensureAlways(IsValid(CachedOccupyingCharacter)))
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = CachedOccupyingCharacter->GetMesh();

	if (ensureAlways(IsValid(CharacterMesh)))
	{
		AttachSkeletalMesh(CharacterMesh);
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

	// Move the character (must be before mesh controlling)
	Character->SetActorLocation(CharacterLocationOnOccupySpot);
	Character->SetActorRotation(CharacterRotationOnOccupySpot);
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

