// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInstances/EscapeChroniclesGameInstance.h"
#include "UI/Widgets/UserWidgets/ActivatableWidgets/RoutableWidget.h"
#include "UI/Widgets/UserWidgets/Buttons/TextButtonBaseWidget.h"
#include "WinMenuWidget.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UWinMenuWidget : public URoutableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();

		EndButton->OnClicked().AddWeakLambda(this, [this]
		{
			UEscapeChroniclesGameInstance* GameInstance =
				GetWorld()->GetGameInstanceChecked<UEscapeChroniclesGameInstance>();

			if (!ensureAlways(IsValid(GameInstance))) return;

			GameInstance->DestroyHostSession(FOnDestroySessionCompleteDelegate(), true);
		});
	}

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UTextButtonBaseWidget> EndButton;

	
};