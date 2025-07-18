// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/HUD.h"
#include "EscapeChroniclesHUD.generated.h"

class URootContainerWidget;
class UCommonActivatableWidget;

UENUM(BlueprintType)
enum class ERouteInputMode : uint8
{
	Game,       // Input handled by game only (no UI interaction)
	UI,         // Input handled by UI only (game paused or ignored)
	GameAndUI   // Hybrid mode (e.g. for menus that don't ignored the game)
};

// Defines a HUD route configuration
USTRUCT(BlueprintType)
struct ESCAPECHRONICLES_API FHUDRoute
{
	GENERATED_BODY()

	// Widget class to instantiate when this route is activated 
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> WidgetClass;

	// Input mode to set when this route is active 
	UPROPERTY(EditDefaultsOnly)
	ERouteInputMode InputMode;

	// Should mouse cursor be visible when this route is active 
	UPROPERTY(EditDefaultsOnly)
	bool bCursorVisible;
};

/**
 * HUD based state-machine that manages widget navigation.
 * 
 * Features:
 * - Single root container widget (URootWidget) managing all UI layers;
 * - Route-based navigation using GameplayTags;
 * - Automatic input mode management;
 * - Stackless navigation (previous widgets are hidden when moving forward);
 */
UCLASS()
class ESCAPECHRONICLES_API AEscapeChroniclesHUD : public AHUD
{
	GENERATED_BODY()
public:
	AEscapeChroniclesHUD();

	/**
	 * Navigates to the specified route.
	 * @param RouteTag GameplayTag identifying the desired route (must exist in Routes map).
	 * @note Will deactivate all currently active widgets in the root container.
	 */
	void GoTo(const FGameplayTag& RouteTag);

	// Returns to the root state of the HUD 
	void GoToRoot() const;
	
protected:
	virtual void BeginPlay() override;
	
private:
	// Widget class used as the root container for all UI elements 
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<URootContainerWidget> RootWidgetClass;

	// Instance of the root widget created at BeginPlay 
	UPROPERTY(Transient)
	TObjectPtr<URootContainerWidget> RootWidget;

	// Default input mode when no routes are active 
	UPROPERTY(EditDefaultsOnly)
	ERouteInputMode RootInputMode = ERouteInputMode::Game;

	// Default cursor visibility when no routes are active 
	UPROPERTY(EditDefaultsOnly)
	bool bRootCursorVisible = false;
	
	/**
	 * Route definitions mapping GameplayTags to widget configurations.
	 * @tparam FGameplayTag The gameplay tag used as a key to identify the route.
	 * @tparam FHUDRoute The configuration that will be used to open the widget.
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FHUDRoute> Routes;

	/**
	 * Updates the player's input mode and cursor visibility.
	 * @param NewInputMode Desired input mode.
	 * @param bNewCursorVisible Whether the cursor should be visible.
	 */
	void SetInputMode(ERouteInputMode NewInputMode, bool bNewCursorVisible) const;
};
