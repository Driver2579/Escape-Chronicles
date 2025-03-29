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
	if (!OwnerController.IsValid() || InteractableComponentsPool.Num() == 0)
	{
		return;
	}
	
	// Find the view location and view direction where the pawn is looking
	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector ViewDirection = ViewRotation.Vector();

	// Select InteractableComponent
	UInteractableComponent* InteractableComponentToSelected = nullptr;
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

			InteractableComponentToSelected = InteractableComponent.Get();
		}
	}
	
	if (InteractableComponentToSelected == nullptr || InteractableComponentToSelected == SelectedInteractableComponent)
	{
		return;
	}

	if (SelectedInteractableComponent.IsValid())
	{
		SelectedInteractableComponent->HideInteractionHint();
		SelectedInteractableComponent = nullptr;
	}

	SelectedInteractableComponent = InteractableComponentToSelected;
	SelectedInteractableComponent->ShowInteractionHint();
}

bool UInteractionManagerComponent::TryInteract()
{
	return TryInteract(SelectedInteractableComponent.Get());
}

bool UInteractionManagerComponent::TryInteract(UInteractableComponent* InteractableComponent)
{
#if DO_ENSURE
	const APawn* OwningPawn = GetOwner<APawn>();

	if (ensureAlways(IsValid(OwningPawn)))
	{
		ensureAlways(OwningPawn->IsLocallyControlled() || OwningPawn->IsBotControlled());
	}
#endif

	if (IsValid(InteractableComponent))
	{
		Server_TryInteract(InteractableComponent);

		return true;
	}

	return false;
}

void UInteractionManagerComponent::Server_TryInteract_Implementation(UInteractableComponent* InteractableComponent)
{
	InteractableComponent->Interact(this);
}

bool UInteractionManagerComponent::Server_TryInteract_Validate(UInteractableComponent* InteractableComponent)
{
	if (!IsValid(InteractableComponent))
	{
		return false;
	}

	const FVector InteractableComponentLocation = InteractableComponent->GetOwner()->GetActorLocation();
	const FVector OwnerLocation = GetOwner()->GetActorLocation();

	const float DistanceToInteractableComponent = FVector::Distance(InteractableComponentLocation, OwnerLocation);

	return DistanceToInteractableComponent <= MaxInteractionDistance;
}
