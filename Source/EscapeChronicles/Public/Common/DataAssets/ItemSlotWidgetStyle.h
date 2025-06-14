// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/Image.h"
#include "ItemSlotWidgetStyle.generated.h"

UCLASS()
class ESCAPECHRONICLES_API UItemSlotWidgetStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EmptySlotTexture;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> InvalidItemInstanceIconTexture;

	UPROPERTY(EditDefaultsOnly)
	FKey DragAndDropKey = EKeys::LeftMouseButton;

	UPROPERTY(Transient)
	TObjectPtr<UImage> DragVisualWidget;

	virtual void PostLoad() override
	{
		Super::PostLoad();

		DragVisualWidget = NewObject<UImage>(this);
	}
};