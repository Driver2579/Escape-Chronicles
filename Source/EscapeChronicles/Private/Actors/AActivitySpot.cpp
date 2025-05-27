// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/AActivitySpot.h"

#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponents/InteractableComponent.h"
#include "Components/CharacterMoverComponents/EscapeChroniclesCharacterMoverComponent.h"
#include "Engine/AssetManager.h"

AAActivitySpot::AAActivitySpot()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

	Mesh->ComponentTags.Add(InteractableComponent->GetHintMeshTag());
}

void AAActivitySpot::BeginPlay()
{
	Super::BeginPlay();

	InteractableComponent->AddInteractionHandler(FInteractDelegate::FDelegate::CreateUObject(this,
		&ThisClass::InteractHandler));
}

void AAActivitySpot::InteractHandler(UInteractionManagerComponent* InteractionManagerComponent)
{
	if (!ensureAlways(IsValid(InteractableComponent)))
	{
		return;
	}
	
	AEscapeChroniclesCharacter* Character = Cast<AEscapeChroniclesCharacter>(
		InteractionManagerComponent->GetOwner());

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}
	
	OccupySpot(Character);
}

void AAActivitySpot::OccupySpot(AEscapeChroniclesCharacter* Character)
{
	if (OccupyingSpotCharacter.IsValid())
	{
		return;
	}

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	UEscapeChroniclesCharacterMoverComponent* Mover = Character->GetCharacterMoverComponent();

	if (!ensureAlways(IsValid(CharacterMesh) && IsValid(Capsule)))
	{
		return;
	}

	Character->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocketName);
	Mover->DisableMovement();
	
	//Capsule->SetCollisionProfileName(TEXT("NoCollision"));
	
	/*
	GameplayEffectHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(GameplayEffectClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnGameplayEffectLoaded));

	AnimMontageHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(AnimMontage.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnAnimMontageLoaded));
		*/

	OccupyingSpotCharacter = Character;
}

void AAActivitySpot::VacateSeat()
{
	if (GameplayEffectHandle.IsValid())
	{
		GameplayEffectHandle->CancelHandle();
		GameplayEffectHandle.Reset();
	}

	if (AnimMontageHandle.IsValid())
	{
		AnimMontageHandle->CancelHandle();
		AnimMontageHandle.Reset();
	}
}

void AAActivitySpot::OnGameplayEffectLoaded()
{
}

void AAActivitySpot::OnAnimMontageLoaded()
{
	
}

