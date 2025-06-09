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
#include "Net/UnrealNetwork.h"

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
		OccupingEffectClass.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&ThisClass::OnOccupyingEffectLoaded));

#if DO_CHECK
	check(OccupingAnimMontages.Num() > 0);
#endif

	SelectedOccupingAnimMontage = FMath::Rand() % OccupingAnimMontages.Num();

	OccupyingAnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		OccupingAnimMontages[SelectedOccupingAnimMontage].ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
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
		AnimInstance->Montage_Play(OccupingAnimMontages[SelectedOccupingAnimMontage].Get());
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
		OccupingEffectClass.Get(), EffectLevel, AbilitySystemComponent->MakeEffectContext());

	OccupingEffectSpecHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
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

	AnimInstance->Montage_Stop(0.0, OccupingAnimMontages[SelectedOccupingAnimMontage].Get());

	// === Stop effect ===

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!ensureAlways(IsValid(AbilitySystemComponent)))
	{
		return;
	}

	AbilitySystemComponent->RemoveActiveGameplayEffect(OccupingEffectSpecHandle);
	
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
