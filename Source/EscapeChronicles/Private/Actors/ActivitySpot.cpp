// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/ActivitySpot.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/AttributeSets/VitalAttributeSet.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

AActivitySpot::AActivitySpot()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

	Mesh->ComponentTags.Add(InteractableComponent->GetHintMeshTag());
}

void AActivitySpot::BeginPlay()
{
	Super::BeginPlay();

	InteractableComponent->AddInteractionHandler(FInteractDelegate::FDelegate::CreateUObject(this,
		&ThisClass::InteractHandler));
}

UAbilitySystemComponent* AActivitySpot::GetAbilitySystemComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CharacterOccupyingSpot.Get());
}

void AActivitySpot::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CharacterOccupyingSpot)
}

void AActivitySpot::InteractHandler(UInteractionManagerComponent* InteractionManagerComponent)
{
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(
		InteractionManagerComponent->GetOwner());

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	if (!IsValid(CharacterOccupyingSpot))
	{
		SetCharacterOccupyingSpot(Character);
	}
	else if (CharacterOccupyingSpot == Character)
	{
		SetCharacterOccupyingSpot(nullptr);
	}
}

bool AActivitySpot::SetCharacterOccupyingSpot(AEscapeChroniclesCharacter* Character)
{
	check(HasAuthority());
	
	if (CharacterOccupyingSpot == Character)
	{
		return false;
	}

	UAbilitySystemComponent* AbilitySystemComponent = Character == nullptr ?
		UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CharacterOccupyingSpot) :
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
		// If there is no character, then there is no need to track fating
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalAttributeSet->GetHealthAttribute())
			.Remove(UnoccupyIfAttributeHasDecreasedDelegateHandle);
		
		UnoccupySpot(CharacterOccupyingSpot);
	}
	else
	{
		OccupySpot(Character);

		// Subscribe to when the character fating
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(VitalAttributeSet->GetHealthAttribute())
			.AddUObject(this, &ThisClass::UnoccupyIfAttributeHasDecreased);
	}

	CharacterOccupyingSpot = Character;
	return true;
}

void AActivitySpot::OnRep_CharacterOccupyingSpot(AEscapeChroniclesCharacter* OldValue)
{
	if (CharacterOccupyingSpot == nullptr)
	{
		UnoccupySpot(OldValue);
	}
	else
	{
		OccupySpot(CharacterOccupyingSpot);
	}
}

void AActivitySpot::OccupySpot(AEscapeChroniclesCharacter* Character)
{
	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}
	
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	UCapsuleComponent* CharacterCapsule = Character->GetCapsuleComponent();
	UEscapeChroniclesCharacterMoverComponent* CharacterMover = Character->GetCharacterMoverComponent();

	if (!ensureAlways(IsValid(CharacterMesh) && IsValid(CharacterCapsule) && IsValid(CharacterMover)))
	{
		return;
	}

	// === We block the movement of the actor and attach only the mesh to the desired location ===
	
	CharacterMover->DisableMovement();

	CachedCapsuleCollisionProfileName = CharacterCapsule->GetCollisionProfileName();
	CharacterCapsule->SetCollisionProfileName(TEXT("NoCollision"));

	CachedMeshAttachParent = CharacterMesh->GetAttachParent();
	CachedMeshTransform = CharacterMesh->GetRelativeTransform();
	CharacterMesh->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);
	
	// === Load and apply animation and effect ===
	
	GameplayEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(GameplayEffectClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameplayEffectLoaded));

	SelectedAnimMontage = FMath::Rand() % AnimMontages.Num();
	
	AnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		AnimMontages[SelectedAnimMontage].ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this,
			&ThisClass::OnAnimMontageLoaded));
}

void AActivitySpot::UnoccupySpot(AEscapeChroniclesCharacter* Character)
{
	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}
	
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	UCapsuleComponent* CharacterCapsule = Character->GetCapsuleComponent();
	UEscapeChroniclesCharacterMoverComponent* CharacterMover = Character->GetCharacterMoverComponent();

	if (!ensureAlways(IsValid(CharacterMesh) && IsValid(CharacterCapsule) && IsValid(CharacterMover)))
	{
		return;
	}

	// === Return the mesh to the state it was in before occupying spot ===
	
	CharacterMover->SetDefaultMovementMode();
	
	CharacterMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	CharacterMesh->AttachToComponent(CachedMeshAttachParent, FAttachmentTransformRules::KeepWorldTransform);
	CharacterMesh->SetRelativeTransform(CachedMeshTransform);
	
	CharacterCapsule->SetCollisionProfileName(CachedCapsuleCollisionProfileName);
	
	CancelAnimationAndEffect(Character);
}

void AActivitySpot::UnoccupyIfAttributeHasDecreased(const FOnAttributeChangeData& AttributeChangeData)
{
	if (AttributeChangeData.OldValue > AttributeChangeData.NewValue)
	{
		SetCharacterOccupyingSpot(nullptr);	
	}
}

void AActivitySpot::OnAnimMontageLoaded()
{
	if (!ensureAlways(IsValid(CharacterOccupyingSpot)))
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = CharacterOccupyingSpot->GetMesh();

	if (!ensureAlways(IsValid(CharacterMesh)))
	{
		return;
	}

	UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();

	if (!ensureAlways(IsValid(AnimInstance)))
	{
		return;
	}

	AnimInstance->Montage_Play(AnimMontages[SelectedAnimMontage].Get());
}

void AActivitySpot::OnGameplayEffectLoaded()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	
	const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		GameplayEffectClass.Get(), EffectLevel, AbilitySystemComponent->MakeEffectContext());

	ActiveEffectSpecHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}

void AActivitySpot::CancelAnimationAndEffect(AEscapeChroniclesCharacter* Character)
{
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

	AnimInstance->Montage_Stop(0.0, AnimMontages[SelectedAnimMontage].Get());

	// === Stop effect ===

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();

	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}

	AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectSpecHandle);
	
	// === Releasing the memory ===
	
	if (AnimMontageHandle.IsValid())
	{
		AnimMontageHandle->CancelHandle();
		AnimMontageHandle.Reset();
	}

	if (GameplayEffectHandle.IsValid())
	{
		GameplayEffectHandle->CancelHandle();
		GameplayEffectHandle.Reset();
	}
}
