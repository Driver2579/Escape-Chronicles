// Vladislav Semchuk, 2025

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "SliderWidgetContainer.generated.h"

class USlider;

/**
 * A user widget that simply contains a slider. It is used as a base class for sliders in this project to have shared
 * styles.
 */
UCLASS()
class ESCAPECHRONICLES_API USliderWidgetContainer : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	USlider* GetSlider() const { return Slider; }

private:
	UPROPERTY(Transient, BlueprintReadOnly, meta=(BindWidget, AllowPrivateAccess="true"))
	TObjectPtr<USlider> Slider;
};