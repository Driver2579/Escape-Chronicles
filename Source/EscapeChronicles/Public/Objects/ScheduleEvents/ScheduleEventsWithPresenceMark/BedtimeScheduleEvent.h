// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScheduleEventWithPresenceMark.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "BedtimeScheduleEvent.generated.h"

/**
 * TODO: Implement me
 */
UCLASS()
class ESCAPECHRONICLES_API UBedtimeScheduleEvent : public UScheduleEventWithPresenceMark
{
	GENERATED_BODY()

public:
	UBedtimeScheduleEvent();

protected:
	virtual void OnEventStarted(const bool bStartPaused) override;

	virtual bool CanCheckInPlayer(const AActor* PresenceMarkTrigger,
		const AEscapeChroniclesPlayerState* PlayerToCheckIn) const override;

	virtual void OnEventEnded() override;

private:
	TMap<TWeakObjectPtr<UPlayerOwnershipComponent>, FDelegateHandle> OnOwningPlayerInitializedDelegateHandles;

	void OnPrisonerChamberZoneOwningPlayerInitialized(UPlayerOwnershipComponent* PlayerOwnershipComponent,
		const FUniquePlayerID& OwningPlayer, const FPlayerOwnershipComponentGroup& Group);

	void UnregisterOnOwningPlayerInitializedDelegates();
};