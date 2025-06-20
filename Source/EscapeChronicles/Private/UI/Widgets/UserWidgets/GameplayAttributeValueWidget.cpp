// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Widgets/UserWidgets/GameplayAttributeValueWidget.h"

#include "CommonTextBlock.h"
#include "Controllers/PlayerControllers/EscapeChroniclesPlayerController.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

void UGameplayAttributeValueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AEscapeChroniclesPlayerController* OwningPlayerController = GetOwningPlayer<AEscapeChroniclesPlayerController>();

	// Wait for the PlayerState to be initialized if it isn't already
	if (ensureAlways(IsValid(OwningPlayerController)))
	{
		OwningPlayerController->CallOrRegister_OnPlayerStateInitialized(
			FOnPlayerStateInitializedDelegate::FDelegate::CreateUObject(this,
				&ThisClass::OnOwningPlayerStateInitialized));
	}
}

void UGameplayAttributeValueWidget::OnOwningPlayerStateInitialized(AEscapeChroniclesPlayerState* PlayerState)
{
#if DO_CHECK
	check(IsValid(PlayerState));
#endif

	UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

#if DO_ENSURE
	ensureAlways(AttributeToCapture.IsValid());
#endif

	// Try to get the current value of the attribute
	bool bFoundAttribute;
	const float CurrentAttributeValue = AbilitySystemComponent->GetGameplayAttributeValue(AttributeToCapture,
		bFoundAttribute);

	if (!ensureAlways(bFoundAttribute))
	{
		return;
	}

	// Initialize the string with the current attribute value
	SetAttributeValueText(CurrentAttributeValue);

	// === Listen for attribute changes to update the text ===

	FOnGameplayAttributeValueChange& OnAttributeValueChangedDelegate =
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeToCapture);

	OnAttributeValueChangedDelegate.AddUObject(this, &ThisClass::OnAttributeValueChanged);
}

void UGameplayAttributeValueWidget::OnAttributeValueChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	SetAttributeValueText(OnAttributeChangeData.NewValue);
}

void UGameplayAttributeValueWidget::SetAttributeValueText(const float NewValue) const
{
#if DO_CHECK
	check(AttributeValueTextWidget);
#endif

	// Set the new attribute value to the text and append the postfix to it
	AttributeValueTextWidget->SetText(
		FText::FromString(FString::FormatAsNumber(NewValue) + AttributeValueTextPostfix.ToString()));
}