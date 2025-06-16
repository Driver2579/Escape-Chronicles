// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/Image.h"
#include "ItemSlotWidgetData.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UItemSlotWidgetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FSlateBrush EmptySlotBrush;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush InvalidItemInstanceBrush;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush DefaultSlotBackgroundBrush;

	UPROPERTY(EditDefaultsOnly)
	FSlateBrush SelectedSlotBackgroundBrush;

	UPROPERTY(EditDefaultsOnly)
	FKey DragAndDropKey = EKeys::LeftMouseButton;

	UPROPERTY(EditDefaultsOnly)
	float DragIconOpacity;

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
	}
};