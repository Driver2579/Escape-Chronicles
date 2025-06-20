// Vladislav Semchuk, 2025

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ComboBoxStringWidgetContainer.generated.h"

class UComboBoxString;

/**
 * A user widget that simply contains a ComboBoxString. It is used as a base class for buttons in this project to have
 * shared styles.
 */
UCLASS()
class ESCAPECHRONICLES_API UComboBoxStringWidgetContainer : public UUserWidget
{
	GENERATED_BODY()

public:
	UComboBoxString* GetComboBox() const { return ComboBox; }

private:
	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<UComboBoxString> ComboBox;
};