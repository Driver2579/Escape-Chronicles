// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActorComponents/PlayerOwnershipComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "Net/UnrealNetwork.h"

#if WITH_EDITOR
EDataValidationResult FPlayerOwnershipComponentGroupTableRow::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = EDataValidationResult::Valid;;

	if (AllowedControlledCharacterTypes.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("AllowedControlledCharacterTypes must not be empty!")));

		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

void UPlayerOwnershipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, OwningPlayer);
}

TPair<FName, FPlayerOwnershipComponentGroupTableRow*> UPlayerOwnershipComponent::GetGroup() const
{
	TPair<FName, FPlayerOwnershipComponentGroupTableRow*> Result;

	Result.Key = Group.RowName;
	Result.Value = Group.GetRow<FPlayerOwnershipComponentGroupTableRow>(TEXT("GetGroup"));

	return Result;
}