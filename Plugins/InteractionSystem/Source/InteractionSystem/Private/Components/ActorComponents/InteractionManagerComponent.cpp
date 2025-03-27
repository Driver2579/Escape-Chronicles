// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractionSystem/Public/Components/ActorComponents/InteractionManagerComponent.h"

#include "Components/ActorComponents/InteractableComponent.h"

UInteractionManagerComponent::UInteractionManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerController = GetOwner<APawn>()->GetController<APlayerController>();
	
	// Add and remove UInteractableComponent from InteractableComponentsPool that enter and exit the InteractionZone
	OnComponentBeginOverlap.AddDynamic(this,
		&UInteractionManagerComponent::OnAddToInteractableComponentsPool);
	OnComponentEndOverlap.AddDynamic(this,
		&UInteractionManagerComponent::OnDeleteFromInteractableComponentsPool);
}

void UInteractionManagerComponent::OnAddToInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	UInteractableComponent* InteractableComponent = OtherActor->FindComponentByClass<UInteractableComponent>();
	if (IsValid(InteractableComponent))
	{
		InteractableComponentsPool.Add(InteractableComponent);
	}
}

void UInteractionManagerComponent::OnDeleteFromInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	UInteractableComponent* InteractableComponent = OtherActor->FindComponentByClass<UInteractableComponent>();
	if (!IsValid(InteractableComponent))
	{
		return;
	}

	// Remove hint if item was selected
	if (SelectedInteractableComponent == InteractableComponent)
	{
		SelectedInteractableComponent->HideInteractionHint();
		SelectedInteractableComponent = nullptr;
	}
		
	InteractableComponentsPool.Remove(InteractableComponent);
}

void UInteractionManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	SelectInteractableComponent();
}

void UInteractionManagerComponent::SelectInteractableComponent()
{
	if (InteractableComponentsPool.Num() == 0)
	{
		return;
	}

	if (SelectedInteractableComponent.IsValid())
	{
		SelectedInteractableComponent->HideInteractionHint();
		SelectedInteractableComponent = nullptr;
	}
	
	// Find the view location and view direction where the pawn is looking
	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector ViewDirection = ViewRotation.Vector();

	// Select InteractableComponent
	float BestDotProduct = -1.0f;
	for (TWeakObjectPtr<UInteractableComponent> InteractableComponent : InteractableComponentsPool)
	{
		if (!IsValid(InteractableComponent.Get())) continue;

		FVector DirectionToActor = (InteractableComponent.Get()->GetOwner()->GetActorLocation() - ViewLocation)
			.GetSafeNormal();

		const float DotProduct = FVector::DotProduct(ViewDirection, DirectionToActor);

		// The closer to 1.0, the more accurate the ViewDirection of DirectionToActor
		if (DotProduct > BestDotProduct) 
		{
			BestDotProduct = DotProduct;

			SelectedInteractableComponent = InteractableComponent;
		}
	}
	
	if (SelectedInteractableComponent != nullptr)
	{
		SelectedInteractableComponent->ShowInteractionHint();
	}
}

bool UInteractionManagerComponent::TryInteract()
{
	return TryInteract(SelectedInteractableComponent.Get());
}

bool UInteractionManagerComponent::TryInteract(UInteractableComponent* Interactable)
{
#if DO_ENSURE
	const APawn* OwningPawn = GetOwner<APawn>();

	if (ensureAlways(IsValid(OwningPawn)))
	{
		ensureAlways(OwningPawn->IsLocallyControlled() || OwningPawn->IsBotControlled());
	}
#endif

	if (SelectedInteractableComponent.IsValid())
	{
		Server_TryInteract(Interactable);

		return true;
	}

	return false;
}

void UInteractionManagerComponent::Server_TryInteract_Implementation(UInteractableComponent* Interactable)
{
	Interactable->Interact(this);
}

bool UInteractionManagerComponent::Server_TryInteract_Validate(UInteractableComponent* Interactable)
{
	return GetDistanceToSelectedInteractableComponent() <= MaxInteractionDistance;
}
