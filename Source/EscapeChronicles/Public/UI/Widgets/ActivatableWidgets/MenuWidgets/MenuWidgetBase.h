// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "MenuWidgetBase.generated.h"


UCLASS()
class ESCAPECHRONICLES_API UMenuWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UMenuWidgetBase();
	
protected:
	virtual bool NativeOnHandleBackAction() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag StateTagOnBackAction;
};
