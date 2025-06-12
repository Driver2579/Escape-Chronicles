// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/CarryCharacterComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

UCarryCharacterComponent::UCarryCharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCarryCharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CarriedCharacter);
}

void UCarryCharacterComponent::SetCarriedCharacter(AEscapeChroniclesCharacter* InCarriedCharacter)
{
	// Check if the new character has the required tags
	if (InCarriedCharacter != nullptr)
	{
		const UAbilitySystemComponent* AbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InCarriedCharacter);

		if (!AbilitySystemComponent->HasAnyMatchingGameplayTags(CarryableCharacterTags))
		{
			return;
		}
	}

	// Validate owner and authority (only the server can modify CarriedCharacter)
	const AActor* Owner = GetOwner();
	if (!ensureAlways(IsValid(Owner)) || !ensureAlways(Owner->HasAuthority()))
	{
		return;
	}

	// === Update CarriedCharacter and trigger replacement logic ===

	const AEscapeChroniclesCharacter* OldCarriedCharacter = CarriedCharacter;

	CarriedCharacter = InCarriedCharacter;

	ReplaceCarriedCharacter(OldCarriedCharacter, InCarriedCharacter);
}

void UCarryCharacterComponent::ReplaceCarriedCharacter(const AEscapeChroniclesCharacter* OldCarriedCharacter,
	const AEscapeChroniclesCharacter* NewCarriedCharacter)
{
	// Drop the old character if it exists
	if (IsValid(OldCarriedCharacter))
	{
		USkeletalMeshComponent* OldCarriedCharacterMesh = OldCarriedCharacter->GetMesh();

		if (ensureAlways(OldCarriedCharacterMesh))
		{
			DropCharacter(OldCarriedCharacterMesh);
		}
	}

	// Pick up the new character if specified
	if (IsValid(NewCarriedCharacter))
	{
		USkeletalMeshComponent* NewCarriedCharacterMesh = NewCarriedCharacter->GetMesh();

		if (ensureAlways(NewCarriedCharacterMesh))
		{
			PickupCharacter(NewCarriedCharacterMesh);
		}
	}

	// Cancel animation loading if no new character is set
	if (NewCarriedCharacter == nullptr && CarryAnimMontagesLoadedHandle.IsValid())
	{
		CarryAnimMontagesLoadedHandle->CancelHandle();
		CarryAnimMontagesLoadedHandle.Reset();
	}
}

void UCarryCharacterComponent::PickupCharacter(USkeletalMeshComponent* CarriedCharacterMesh)
{
	// Cache the mesh for animation
	CachedCarriedCharacterMesh = CarriedCharacterMesh;

	// === Load and play animation ===

	TArray AnimMontagesToLoad = {
		CarryingCharacterAnimMontage.ToSoftObjectPath(),
		CarriedCharacterAnimMontage.ToSoftObjectPath()
	};

	// If animations are already loaded, apply them immediately
	if (CarryAnimMontagesLoadedHandle.IsValid())
	{
		OnCarryAnimMontagesLoaded();
	}
	else
	{
		CarryAnimMontagesLoadedHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(AnimMontagesToLoad,
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnCarryAnimMontagesLoaded));
	}
}

void UCarryCharacterComponent::OnCarryAnimMontagesLoaded()
{
#if DO_CHECK
	check(CarryingCharacterAnimMontage.IsValid());
	check(CarriedCharacterAnimMontage.IsValid());
	check(CachedCarriedCharacterMesh.IsValid());
#endif

	// === Get the carrying character and its mesh ===

	const AEscapeChroniclesCharacter* CarryingCharacter = GetOwner<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(CarryingCharacter)))
	{
		return;
	}

	USkeletalMeshComponent* CarryingCharacterMesh = CarryingCharacter->GetMesh();

	if (!ensureAlways(IsValid(CarryingCharacterMesh)))
	{
		return;
	}

	// === Play loaded animation ===

	UAnimInstance* CarryingCharacterAnimInstance = CarryingCharacterMesh->GetAnimInstance();
	if (ensureAlways(CarryingCharacterAnimInstance))
	{
		CarryingCharacterAnimInstance->Montage_Play(CarryingCharacterAnimMontage.Get());
	}

	UAnimInstance* CarriedCharacterAnimInstance = CachedCarriedCharacterMesh->GetAnimInstance();
	if (ensureAlways(CarriedCharacterAnimInstance))
	{
		CarriedCharacterAnimInstance->Montage_Play(CarriedCharacterAnimMontage.Get());
	}

	// === Attach the carried character's mesh ===

	CachedCarriedCharacterMesh->SetSimulatePhysics(false);
	CachedCarriedCharacterMesh->SetUsingAbsoluteRotation(false);

	CachedCarriedCharacterMesh->AttachToComponent(CarryingCharacterMesh,
		FAttachmentTransformRules::KeepRelativeTransform, AttachSocketName);

	CachedCarriedCharacterMesh->SetRelativeTransform(AttachSocketTransform);
}

void UCarryCharacterComponent::DropCharacter(USkeletalMeshComponent* CarriedCharacterMesh) const
{
#if DO_CHECK
	check(IsValid(CarriedCharacterMesh));
#endif

	// Enable physics and detach the mesh
	CarriedCharacterMesh->SetSimulatePhysics(true);
	CarriedCharacterMesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

	// Stop the carried character's animation
	UAnimInstance* CarriedCharacterAnimInstance = CarriedCharacterMesh->GetAnimInstance();
	if (ensureAlways(CarriedCharacterAnimInstance))
	{
		CarriedCharacterAnimInstance->Montage_Stop(0);
	}

	// === Stop the carrying character's animation ===

	const AEscapeChroniclesCharacter* CarryingCharacter = GetOwner<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(CarryingCharacter)))
	{
		return;
	}
	
	const USkeletalMeshComponent* CarryingCharacterMesh = CarryingCharacter->GetMesh();

	if (!ensureAlways(IsValid(CarryingCharacterMesh)))
	{
		return;
	}

	UAnimInstance* CarryingCharacterAnimInstance = CarryingCharacterMesh->GetAnimInstance();
	if (ensureAlways(CarryingCharacterAnimInstance))
	{
		CarryingCharacterAnimInstance->Montage_Stop(0);
	}
}
