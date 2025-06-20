// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NameplateWidgetComponent.generated.h"

class AEscapeChroniclesPlayerState;

/**
 * A widget component that displays the name of the owning player or bot.
 * @remark The widget must be a subclass of UTextContainerWidget.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPECHRONICLES_API UNameplateWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNameplateWidgetComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnOwningPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState);

private:
	FDelegateHandle OnPlayerNameChangedDelegateHandle;

	void SetNameplateText(const FString& NewName);

	TWeakObjectPtr<APlayerCameraManager> LocalPlayerCameraManager;
};