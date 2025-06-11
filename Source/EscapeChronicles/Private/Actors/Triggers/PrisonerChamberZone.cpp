// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Triggers/PrisonerChamberZone.h"

#include "EngineUtils.h"
#include "Characters/EscapeChroniclesCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/ActorComponents/PlayerOwnershipComponent.h"
#include "GameFramework/GameStateBase.h"
#include "PlayerStates/EscapeChroniclesPlayerState.h"

APrisonerChamberZone::APrisonerChamberZone()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	OuterZoneBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Outer Zone"));
	OuterZoneBoxComponent->SetupAttachment(RootComponent);

	InnerZoneBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Inner Zone"));
	InnerZoneBoxComponent->SetupAttachment(RootComponent);

	OuterZoneBoxComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	InnerZoneBoxComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	PlayerOwnershipComponent = CreateDefaultSubobject<UPlayerOwnershipComponent>(TEXT("Player Ownership"));
}

void APrisonerChamberZone::BeginPlay()
{
	Super::BeginPlay();

	InnerZoneBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnInnerZoneBeginOverlap);
	OuterZoneBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnOuterZoneEndOverlap);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void APrisonerChamberZone::OnInnerZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const AEscapeChroniclesCharacter* OverlappingCharacter = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (IsValid(OverlappingCharacter) && OtherComp == OverlappingCharacter->GetMesh())
	{
		CharactersInChamber.Add(OverlappingCharacter);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void APrisonerChamberZone::OnOuterZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int OtherBodyIndex)
{
	const AEscapeChroniclesCharacter* OverlappingCharacter = Cast<AEscapeChroniclesCharacter>(OtherActor);

	if (IsValid(OverlappingCharacter) && OtherComp == OverlappingCharacter->GetMesh())
	{
		CharactersInChamber.Remove(OverlappingCharacter);
	}
}

bool APrisonerChamberZone::IsCharacterInChamber(const AEscapeChroniclesCharacter* Character) const
{
	return CharactersInChamber.Contains(Character);
}

bool APrisonerChamberZone::IsOwningCharacterInThisChamber(bool& bOutHasOwningCharacter) const
{
	bOutHasOwningCharacter = false;

	const FUniquePlayerID* OwningPlayerID = PlayerOwnershipComponent->GetOwningPlayer();

	// Return false if this chamber doesn't have an owning player
	if (!OwningPlayerID)
	{
		return false;
	}

	const AGameStateBase* GameState = GetWorld()->GetGameState();

	if (!ensureAlways(IsValid(GameState)))
	{
		return false;
	}

	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const AEscapeChroniclesPlayerState* CastedPlayerState = Cast<AEscapeChroniclesPlayerState>(PlayerState);

		/**
		 * Go to the next PlayerState if we failed to cast this one or if it isn't the one of the player that owns this
		 * chamber.
		 */
		if (!IsValid(CastedPlayerState) || CastedPlayerState->GetUniquePlayerID() != *OwningPlayerID)
		{
			continue;
		}

		// We found a player that owns this chamber
		bOutHasOwningCharacter = true;

		const AEscapeChroniclesCharacter* Character = PlayerState->GetPawn<AEscapeChroniclesCharacter>();

		// Make sure the character is valid in case it isn't spawned yet
		if (!IsValid(Character))
		{
			return false;
		}

		// Return true if the owning character is currently in this chamber
		return IsCharacterInChamber(Character);
	}

	// Return false if we didn't find any player that is currently in the game and that owns this chamber
	return false;
}

bool APrisonerChamberZone::IsOwningCharacterBedOccupied(bool& bOutHasBedWithSameOwningCharacter) const
{
	bOutHasBedWithSameOwningCharacter = false;

	// If the bed class is invalid, then it for sure isn't on the scene, so none of the beds are occupied
	if (!PrisonerBedClass.IsValid())
	{
		return false;
	}

	const FUniquePlayerID* OwningPlayerID = PlayerOwnershipComponent->GetOwningPlayer();

	// Return false if this chamber doesn't have an owning player
	if (!OwningPlayerID)
	{
		return false;
	}

	TSet<AActor*> OverlappingBeds;
	GetOverlappingActors(OverlappingBeds, PrisonerBedClass.Get());

	for (AActor* OverlappingBed : OverlappingBeds)
	{
#if DO_CHECK
		check(IsValid(OverlappingBed));
		check(OverlappingBed->IsA<AActivitySpot>());
#endif

		const AActivitySpot* CastedBed = CastChecked<AActivitySpot>(OverlappingBed);

		const FUniquePlayerID* BedOwningPlayerID = CastedBed->GetPlayerOwnershipComponent()->GetOwningPlayer();

		// Go to the next bed if this one doesn't have an owning player or if it isn't the one that owns this chamber
		if (!BedOwningPlayerID || *BedOwningPlayerID != *OwningPlayerID)
		{
			continue;
		}

		// We found a bed that overlaps this chamber and has the same owning player
		bOutHasBedWithSameOwningCharacter = true;

		// TODO: Uncomment this

		/*// If the bed is occupied, then return true
		if (Bed->IsOccupied())
		{
			return true;
		}*/
	}

	/**
	 * Return false if we didn't find any bed that overlaps this chamber and has the same owning player, or if the bed
	 * is empty.
	 */
	return false;
}

bool APrisonerChamberZone::IsCharacterInHisChamber(const AEscapeChroniclesCharacter* Character,
	bool& bOutHasOwningChamber)
{
	bOutHasOwningChamber = false;

#if DO_CHECK
	check(IsValid(Character));
#endif

	const AEscapeChroniclesPlayerState* PlayerState = CastChecked<AEscapeChroniclesPlayerState>(
		Character->GetPlayerState(), ECastCheckedType::NullAllowed);

	// Character doesn't have a PlayerState yet, so we can't check if he is in his chamber
	if (!IsValid(PlayerState))
	{
		return false;
	}

	const FUniquePlayerID& UniquePlayerID = PlayerState->GetUniquePlayerID();

	for (TActorIterator<APrisonerChamberZone> It(Character->GetWorld()); It; ++It)
	{
		const APrisonerChamberZone* PrisonerChamberZone = *It;

#if DO_CHECK
		check(IsValid(PrisonerChamberZone));
#endif

		const FUniquePlayerID* ChamberOwningPlayerID =
			PrisonerChamberZone->GetPlayerOwnershipComponent()->GetOwningPlayer();

		/**
		 * Go to the next chamber if this one doesn't have an owning player or if the character isn't the owner of this
		 * chamber.
		 */
		if (!ChamberOwningPlayerID || *ChamberOwningPlayerID != UniquePlayerID)
		{
			continue;
		}

		// Remember that the given character has an owning chamber
		bOutHasOwningChamber = true;

		// If the character that owns this chamber is in it, then return true
		if (PrisonerChamberZone->IsCharacterInChamber(Character))
		{
			return true;
		}
	}

	// Return false if we didn't find a chamber that the character owns and currently is in
	return false;
}

void APrisonerChamberZone::GetAllCharactersOutsideTheirChambers(const UObject* WorldContextObject,
	TArray<AEscapeChroniclesCharacter*>& OutCharactersOutsideTheirChambers)
{
	// Reset the array just in case
	OutCharactersOutsideTheirChambers.Empty();

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	if (!ensureAlways(IsValid(World)))
	{
		return;
	}

	AGameStateBase* GameState = World->GetGameState();

	if (!ensureAlways(IsValid(GameState)))
	{
		return;
	}

	// Iterate all characters to find those that are outside their chambers
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
#if DO_CHECK
		check(IsValid(PlayerState));
#endif

		AEscapeChroniclesCharacter* Character = PlayerState->GetPawn<AEscapeChroniclesCharacter>();

		if (!IsValid(Character))
		{
			continue;
		}

		bool bHasOwningChamber = false;

		// If the character isn't in his chamber and if he owns any, then add him to the output array
		if (!IsCharacterInHisChamber(Character, bHasOwningChamber) && bHasOwningChamber)
		{
			OutCharactersOutsideTheirChambers.Add(Character);
		}
	}
}