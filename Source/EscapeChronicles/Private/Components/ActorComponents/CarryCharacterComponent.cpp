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
	if (InCarriedCharacter != nullptr)
	{
		const UAbilitySystemComponent* AbilitySystemComponent =
			UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InCarriedCharacter);
		if (!AbilitySystemComponent->HasAnyMatchingGameplayTags(CarryableCharacterTags))
		{
			return;
		}
	}

	const AActor* Owner = GetOwner();
	if (!ensureAlways(IsValid(Owner)) || !ensureAlways(Owner->HasAuthority()))
	{
		return;
	}

	const AEscapeChroniclesCharacter* OldCarriedCharacter = CarriedCharacter;

	CarriedCharacter = InCarriedCharacter;

	ReplaceCarriedCharacter(OldCarriedCharacter, InCarriedCharacter);
}

void UCarryCharacterComponent::ReplaceCarriedCharacter(const AEscapeChroniclesCharacter* OldCarriedCharacter,
	const AEscapeChroniclesCharacter* NewCarriedCharacter)
{
	if (IsValid(OldCarriedCharacter))
	{
		USkeletalMeshComponent* OldCarriedCharacterMesh = OldCarriedCharacter->GetMesh();

		if (ensureAlways(OldCarriedCharacterMesh))
		{
			DropCharacter(OldCarriedCharacterMesh);
		}
	}

	if (IsValid(NewCarriedCharacter))
	{
		USkeletalMeshComponent* NewCarriedCharacterMesh = NewCarriedCharacter->GetMesh();

		if (ensureAlways(NewCarriedCharacterMesh))
		{
			PickupCharacter(NewCarriedCharacterMesh);
		}
	}

	if (NewCarriedCharacter == nullptr && CarryAnimMontagesLoadedHandle.IsValid())
	{
		CarryAnimMontagesLoadedHandle->CancelHandle();
		CarryAnimMontagesLoadedHandle.Reset();
	}
}

void UCarryCharacterComponent::PickupCharacter(USkeletalMeshComponent* CarriedCharacterMesh)
{
	// Cache mesh to animate it after loading
	CachedCarriedCharacterMesh = CarriedCharacterMesh;

	// === Load Animation ===

	TArray AnimMontagesToLoad = {
	CarryingCharacterAnimMontage.ToSoftObjectPath(),
	CarriedCharacterAnimMontage.ToSoftObjectPath()
	};

	CarryAnimMontagesLoadedHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			AnimMontagesToLoad,
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnCarryAnimMontagesLoaded));

	if (!CarryingCharacterAnimMontage.IsValid())
	{
		AnimMontagesToLoad.Add(CarryingCharacterAnimMontage.ToSoftObjectPath());
	}

	if (!CarriedCharacterAnimMontage.IsValid())
	{
		AnimMontagesToLoad.Add(CarriedCharacterAnimMontage.ToSoftObjectPath());
	}

	if (AnimMontagesToLoad.Num() == 0)
	{
		OnCarryAnimMontagesLoaded();
	}
	else
	{
		CarryAnimMontagesLoadedHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			AnimMontagesToLoad,
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

	// === Play anim montages ===

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

	// === Attach to carrying character ===

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

	// === Restore carried mesh ===

	CarriedCharacterMesh->SetSimulatePhysics(true);
	CarriedCharacterMesh->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

	UAnimInstance* CarriedCharacterAnimInstance = CarriedCharacterMesh->GetAnimInstance();
	if (ensureAlways(CarriedCharacterAnimInstance))
	{
		CarriedCharacterAnimInstance->Montage_Stop(0);
	}

	// === Restore carrying mesh ===

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
