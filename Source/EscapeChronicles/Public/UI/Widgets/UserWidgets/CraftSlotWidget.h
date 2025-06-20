// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Common/DataAssets/CraftSlotWidgetData.h"
#include "Common/Structs/TableRowBases/InventoryManagerCraftData.h"
#include "Components/Image.h"
#include "CraftSlotWidget.generated.h"

struct FInventorySlot;
class UItemSlotWidgetData;

UCLASS()
class ESCAPECHRONICLES_API UCraftSlotWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	const FName& GetAssociatedRowName() const { return *AssociatedRowName; }
	const FInventoryManagerCraftData& GetAssociatedRowData() const { return *AssociatedRowData; }

	void SetAssociate(const FName& InAssociatedRowName, const FInventoryManagerCraftData& InAssociatedRowData);

protected:
	virtual void NativeOnInitialized() override;
	
	virtual void NativeConstruct() override;

	virtual void NativeOnClicked() override;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemInstanceIcon;

	UPROPERTY(EditDefaultsOnly)
	bool bSelectSlotOnHover = true;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCraftSlotWidgetData> Data;

	const FName* AssociatedRowName;
	const FInventoryManagerCraftData* AssociatedRowData;

	void UpdateEnabled();
};
