// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/CarryableComponent.h"

#include "Chaos/CollisionResolutionUtil.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/ActorComponents/InteractionManagerComponent.h"

void UCarryableComponent::BeginPlay()
{
	Super::BeginPlay();

	AddInteractionHandler(FInteractDelegate::FDelegate::CreateUObject(this, &UCarryableComponent::Carry));
}

void UCarryableComponent::Carry(UInteractionManagerComponent* InteractionManagerComponent)
{
	AEscapeChroniclesCharacter* Character = InteractionManagerComponent->GetOwner<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(Character)))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();

	if (!ensureAlways(IsValid(Mesh)))
	{
		return;
	}
	
	GetOwner()->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketToAttachWhenIsCarrying);

	DefaultTransform = GetOwner()->GetActorTransform();
	
	GetOwner()->SetActorRelativeTransform(TransformWhenIsCarrying);
	GetOwner()->SetActorEnableCollision(false);

	AEscapeChroniclesCharacter* OwnerCharacter = GetOwner<AEscapeChroniclesCharacter>();
	
	OwnerCharacter->GetMesh()->SetSimulatePhysics(false);
	OwnerCharacter->GetMesh()->bBlendPhysics = false;
}
