// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/HUD.h"
#include "UI/Widgets/ActivatableWidgets/EscapeChroniclesContainerWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "EscapeChroniclesHUD.generated.h"

class UMenuWidget;
class UEscapeChroniclesActivatableWidget;

/**
 * This HUD works with one widget which is a container for all the others on the screen. Works like a state machine
 * changing the widget on the zero layer, and it is also convenient to add new widgets on top.
 */
UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesHUD : public AHUD
{
	GENERATED_BODY()
public:
	AEscapeChroniclesHUD();
	
	FGameplayTag GetCurrentMenuTag() const { return CurrentMenuTag; }
	
	void GoToMenu(FGameplayTag NewMenuTag);
	void GoToRootMenu();

	template <typename T = UEscapeChroniclesActivatableWidget>
	T* AddWidget(TSubclassOf<UEscapeChroniclesActivatableWidget> WidgetClass)
	{
		UCommonActivatableWidgetStack* WidgetStack = ContainerWidget->GetWidgetStack();

		if (ensureAlways(IsValid(WidgetStack)))
		{
			return WidgetStack->AddWidget<T>(WidgetClass);
		}

		return nullptr;
	}
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UEscapeChroniclesContainerWidget> ContainerWidgetClass;

	UPROPERTY()
	TObjectPtr<UEscapeChroniclesContainerWidget> ContainerWidget;

	// === Menus ===
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag RootMenuTag;
	
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, TSubclassOf<UMenuWidget>> Menus;
	
	FGameplayTag CurrentMenuTag;
};
