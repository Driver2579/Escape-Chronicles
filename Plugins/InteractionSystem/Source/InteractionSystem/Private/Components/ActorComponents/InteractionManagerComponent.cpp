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

	const APawn* OwningPawn = GetOwner<APawn>();

	if (!ensureAlways(IsValid(OwningPawn)))
	{
		return;
	}

	bIsLocallyControlled = OwningPawn->IsLocallyControlled();

	if (!bIsLocallyControlled)
	{
		return;
	}
		
	OwnerController = OwningPawn->GetController<APlayerController>();

	bool bWasThereCollisionBinding = false;
	
	for (USceneComponent* Component : GetAttachChildren())
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component);
		
		if (IsValid(Primitive))
		{
			// Subscribe to the events of the beginning and end of the collision intersection of a primitive
			Primitive->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAddToInteractableComponentsPool);
			Primitive->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnDeleteFromInteractableComponentsPool);

			bWasThereCollisionBinding = true;
		}
	}

#if DO_ENSURE
	ensureAlwaysMsgf(bWasThereCollisionBinding,
		TEXT("Wasn't there an actor suitable to bind a selection component search!"));
#endif
}

// ReSharper disable once CppParameterMayBeConst
void UInteractionManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLocallyControlled)
	{
		SelectInteractableComponent();
	}
}


void UInteractionManagerComponent::OnAddToInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}
	
	// === When the object enters the interaction zone - add it to the pool ===
	
	UInteractableComponent* InteractableComponent = OtherActor->FindComponentByClass<UInteractableComponent>();

	if (IsValid(InteractableComponent) && InteractableComponent->bCanInteraction == true)
	{
		InteractableComponentsPool.Add(InteractableComponent);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UInteractionManagerComponent::OnDeleteFromInteractableComponentsPool(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	// === When the object leaves the zone - remove it from the pool ===
	
	UInteractableComponent* InteractableComponent = OtherActor->FindComponentByClass<UInteractableComponent>();

	if (!IsValid(InteractableComponent))
	{
		return;
	}

	// Remove hint if item was selected
	if (SelectedInteractableComponent == InteractableComponent)
	{
		SelectedInteractableComponent->SetInteractionHintVisibility(false);
		SelectedInteractableComponent = nullptr;
	}
		
	InteractableComponentsPool.Remove(InteractableComponent);
}

bool UInteractionManagerComponent::IsPathObstructed(const UInteractableComponent* InteractableComponent) const
{
	if (!ensureAlways(IsValid(InteractableComponent)))
	{
		return false;
	}

	const AActor* InteractableActor = InteractableComponent->GetOwner();

	const FVector Start = GetOwner()->GetActorLocation();
	const FVector End = InteractableActor->GetActorLocation();

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(GetOwner());
	TraceParams.AddIgnoredActor(InteractableActor);
	TraceParams.bTraceComplex = true;

	FHitResult HitResult;;
	return GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams);
}

void UInteractionManagerComponent::SelectInteractableComponent()
{
	if (!ensureAlways(OwnerController.IsValid()) || InteractableComponentsPool.IsEmpty())
	{
		return;
	}
	
	// Find the view location and view rotation
	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	// Convert the rotation value to the direction
	const FVector ViewDirection = ViewRotation.Vector();

	// === Select a new InteractableComponent ===
	
	UInteractableComponent* NewSelectedInteractableComponent = nullptr;
	float BestDotProduct = -1.0f;

	for (TWeakObjectPtr<UInteractableComponent> InteractableComponent : InteractableComponentsPool)
	{
		if (!IsValid(InteractableComponent.Get()) || IsPathObstructed(InteractableComponent.Get()))
		{
			continue;
		}

		const FVector OffsetVectorToActor = InteractableComponent->GetOwner()->GetActorLocation() - ViewLocation;
		const FVector DirectionToActor = OffsetVectorToActor.GetSafeNormal();

		const float DotProduct = FVector::DotProduct(ViewDirection, DirectionToActor);

		// The closer to 1.0, the more accurate the ViewDirection of DirectionToActor
		if (DotProduct > BestDotProduct) 
		{
			BestDotProduct = DotProduct;

			NewSelectedInteractableComponent = InteractableComponent.Get();
		}
	}
	
	if (!NewSelectedInteractableComponent || NewSelectedInteractableComponent == SelectedInteractableComponent)
	{
		return;
	}

	// Remove selection from the old selected actor
	if (SelectedInteractableComponent.IsValid())
	{
		SelectedInteractableComponent->SetInteractionHintVisibility(false);
		SelectedInteractableComponent = nullptr;
	}
	
	SelectedInteractableComponent = NewSelectedInteractableComponent;
	SelectedInteractableComponent->SetInteractionHintVisibility(true);
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
	if (!IsValid(InteractableComponent) || InteractableComponent->bCanInteraction == false)
	{
		return false;
	}

	const FVector InteractableComponentLocation = InteractableComponent->GetOwner()->GetActorLocation();
	const FVector OwnerLocation = GetOwner()->GetActorLocation();

	const float DistanceToInteractableComponent = FVector::Distance(InteractableComponentLocation, OwnerLocation);

	return DistanceToInteractableComponent <= MaxInteractionDistance && !IsPathObstructed(InteractableComponent);
}
