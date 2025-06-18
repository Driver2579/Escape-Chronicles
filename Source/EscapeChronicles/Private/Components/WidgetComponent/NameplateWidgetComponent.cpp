// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/WidgetComponent/NameplateWidgetComponent.h"

#include "CommonTextBlock.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"
#include "UI/Widgets/UserWidgets/Containers/TextContainerWidget.h"

UNameplateWidgetComponent::UNameplateWidgetComponent()
{
	SetWidgetClass(UTextContainerWidget::StaticClass());

	SetManuallyRedraw(true);

	// Don't tick when the widget isn't visible
	TickMode = ETickMode::Automatic;
	TickWhenOffscreen = false;
}

void UNameplateWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	AEscapeChroniclesCharacter* OwningCharacter = GetOwner<AEscapeChroniclesCharacter>();

	if (!ensureAlways(IsValid(OwningCharacter)))
	{
		return;
	}

	// We don't need to show the nameplate on the locally controlled character
	if (OwningCharacter->IsLocallyControlled())
	{
		SetVisibility(false);

		return;
	}

	AEscapeChroniclesPlayerState* OwningPlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		OwningCharacter->GetPlayerState(), ECastCheckedType::NullAllowed);

	// Try to call the OnOwningPlayerStateChanged function if the PlayerState is already valid
	if (IsValid(OwningPlayerState))
	{
		OnOwningPlayerStateChanged(OwningPlayerState, nullptr);
	}

	// Listen for PlayerState changes to know when it's initialized if it isn't already or when it changes
	OwningCharacter->OnPlayerStateChangedDelegate.AddUObject(this, &ThisClass::OnOwningPlayerStateChanged);

	LocalPlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
}

void UNameplateWidgetComponent::OnOwningPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	AEscapeChroniclesPlayerState* CastedOldPlayerState = CastChecked<AEscapeChroniclesPlayerState>(OldPlayerState,
		ECastCheckedType::NullAllowed);

	// Unsubscribe from the old PlayerState's OnPlayerNameChanged delegate if it was valid
	if (IsValid(OldPlayerState))
	{
		CastedOldPlayerState->OnPlayerNameChanged.Remove(OnPlayerNameChangedDelegateHandle);
	}

	AEscapeChroniclesPlayerState* CastedNewPlayerState = CastChecked<AEscapeChroniclesPlayerState>(NewPlayerState,
		ECastCheckedType::NullAllowed);

	if (IsValid(NewPlayerState))
	{
		// Set the nameplate widget to the current PlayerState's name
		SetNameplateText(CastedNewPlayerState->GetPlayerName());

		// Update the nameplate widget's text when the PlayerState's name changes
		OnPlayerNameChangedDelegateHandle = CastedNewPlayerState->OnPlayerNameChanged.AddUObject(this,
			&ThisClass::SetNameplateText);
	}
}

void UNameplateWidgetComponent::SetNameplateText(const FString& NewName)
{
	const UTextContainerWidget* NameplateWidget = Cast<UTextContainerWidget>(GetWidget());

	// Update the text and redraw the widget
	if (ensureAlways(IsValid(NameplateWidget)))
	{
		NameplateWidget->GetTextBlock()->SetText(FText::FromString(NewName));

		RequestRedraw();
	}
}

void UNameplateWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!LocalPlayerCameraManager.IsValid())
	{
		return;
	}

	// === Make the component face the camera ===

	const FRotator LocalLookAtComponentRotation = UKismetMathLibrary::FindLookAtRotation(GetComponentLocation(),
		LocalPlayerCameraManager->GetCameraLocation());

	SetWorldRotation(FRotator(GetComponentRotation().Pitch, LocalLookAtComponentRotation.Yaw,
		LocalLookAtComponentRotation.Roll));
}