// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/CarryCharacterComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

UCarryCharacterComponent::UCarryCharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCarryCharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CarriedCharacter);
}

bool UCarryCharacterComponent::SetCarriedCharacter(AEscapeChroniclesCharacter* InCarriedCharacter)
{
	// Check if the new character has the required tags
	if (InCarriedCharacter != nullptr)
	{
		const UAbilitySystemComponent* AbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InCarriedCharacter);

		if (!AbilitySystemComponent->HasAnyMatchingGameplayTags(CarryableCharacterTags))
		{
			return false;
		}
	}

	// Validate owner and authority (only the server can modify CarriedCharacter)
	const AActor* Owner = GetOwner();
	if (!ensureAlways(IsValid(Owner)) || !ensureAlways(Owner->HasAuthority()))
	{
		return false;
	}

	// === Update CarriedCharacter and trigger replacement logic ===

	AEscapeChroniclesCharacter* OldCarriedCharacter = CarriedCharacter;

	if (ReplaceCarriedCharacter(OldCarriedCharacter, InCarriedCharacter))
	{
		CarriedCharacter = InCarriedCharacter;

		return true;
	}

	CarriedCharacter = nullptr;

	return false;
}

bool UCarryCharacterComponent::ReplaceCarriedCharacter(AEscapeChroniclesCharacter* OldCarriedCharacter,
	AEscapeChroniclesCharacter* NewCarriedCharacter)
{
	// Drop the old character if it exists
	if (IsValid(OldCarriedCharacter))
	{
		DropCharacter(OldCarriedCharacter);
	}

	bool bPickupCharacterResult = true;

	// Pick up the new character if specified
	if (IsValid(NewCarriedCharacter))
	{
		bPickupCharacterResult = PickupCharacter(NewCarriedCharacter);
	}

	// Cancel animation loading if no new character is set
	if (NewCarriedCharacter == nullptr && CarryAnimMontagesLoadedHandle.IsValid())
	{
		CarryAnimMontagesLoadedHandle->CancelHandle();
		CarryAnimMontagesLoadedHandle.Reset();
	}

	return bPickupCharacterResult;
}

bool UCarryCharacterComponent::PickupCharacter(AEscapeChroniclesCharacter* InCarriedCharacter)
{
#if DO_CHECK
	check(IsValid(InCarriedCharacter));
#endif

	// Cache the mesh for animation
	CachedCarriedCharacterMesh = InCarriedCharacter->GetMesh();

	if (!ensureAlways(CachedCarriedCharacterMesh.IsValid()))
	{
		return false;
	}

	AEscapeChroniclesCharacter* CarryingCharacter = GetOwner<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(CarryingCharacter)))
	{
		return false;
	}

	AEscapeChroniclesPlayerState* CarriedCharacterPlayerState = InCarriedCharacter->GetPlayerState<
		AEscapeChroniclesPlayerState>();

	if (ensureAlways(IsValid(CarriedCharacterPlayerState)))
	{
		// Return false if the character is already being carried by another character
		if (IsValid(CarriedCharacterPlayerState->GetCarryingCharacter()))
		{
			return false;
		}

		CarriedCharacterPlayerState->SetCarryingCharacter(CarryingCharacter);
	}

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

	return true;
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

void UCarryCharacterComponent::DropCharacter(AEscapeChroniclesCharacter* InCarriedCharacter) const
{
#if DO_CHECK
	check(IsValid(InCarriedCharacter));
#endif

	USkeletalMeshComponent* CarriedCharacterMesh = InCarriedCharacter->GetMesh();

	if (!ensureAlways(IsValid(CarriedCharacterMesh)))
	{
		return;
	}

	// Enable physics and detach the mesh
	CarriedCharacterMesh->SetSimulatePhysics(true);
	CarriedCharacterMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	// Stop the carried character's animation
	UAnimInstance* CarriedCharacterAnimInstance = CarriedCharacterMesh->GetAnimInstance();
	if (ensureAlways(CarriedCharacterAnimInstance))
	{
		CarriedCharacterAnimInstance->Montage_Stop(0);
	}

	AEscapeChroniclesPlayerState* CarriedCharacterPlayerState = InCarriedCharacter->GetPlayerState<
		AEscapeChroniclesPlayerState>();

	if (ensureAlways(IsValid(CarriedCharacterPlayerState)))
	{
		CarriedCharacterPlayerState->SetCarryingCharacter(nullptr);
	}

	const AEscapeChroniclesCharacter* CarryingCharacter = GetOwner<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(CarryingCharacter)))
	{
		return;
	}

	// === Stop the carrying character's animation ===
	
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

	return;
}
