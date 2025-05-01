// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/HUD.h"
#include "EscapeChroniclesHUD.generated.h"

class UCommonActivatableWidget;
class UEscapeChroniclesBaseWidget;

//
UENUM(BlueprintType)
enum class EInputMode : uint8 {
	GameOnly,
	UiOnly,
	GameAndUi,
};

//
USTRUCT()
struct FHUDState
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> WidgetClass;

	UPROPERTY(EditDefaultsOnly)
	EInputMode InputMode;

	UPROPERTY(EditDefaultsOnly)
	bool bShowMouseCursor;
};

//
UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesHUD : public AHUD
{
	GENERATED_BODY()
public:
	FGameplayTag GetCurrentStateTag() const { return CurrentStateTag; }
	
	void ChangeState(const FGameplayTag StateTag);

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UEscapeChroniclesBaseWidget> BaseWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DefaultState;
	
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FHUDState> States;
	
	FGameplayTag CurrentStateTag;

	TObjectPtr<UEscapeChroniclesBaseWidget> BaseWidget;
};
