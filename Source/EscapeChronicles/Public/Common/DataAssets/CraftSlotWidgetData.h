// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CraftSlotWidgetData.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UCraftSlotWidgetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FSlateBrush InvalidItemInstanceBrush;

	/*
	UPROPERTY(Transient)
	TObjectPtr<UImage> DragVisualWidget;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		DragVisualWidget = NewObject<UImage>(this);
	}
#endif

	virtual void PostLoad() override
	{
		Super::PostLoad();

		DragVisualWidget = NewObject<UImage>(this);
	}*/
};