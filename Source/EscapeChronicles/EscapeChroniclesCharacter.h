// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MoverSimulationTypes.h"
#include "EscapeChroniclesCharacter.generated.h"

class UCapsuleComponent;
class UCharacterMoverComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

struct FInputActionValue;

UCLASS(config=Game)
class AEscapeChroniclesCharacter : public APawn, public IMoverInputProducerInterface
{
	GENERATED_BODY()

public:
	AEscapeChroniclesCharacter();

	virtual void PostLoad() override;

	// Returns CapsuleComponent subobject
	UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

	// Returns MeshComponent subobject
	USkeletalMeshComponent* GetMesh() const { return MeshComponent; }

#if WITH_EDITORONLY_DATA
	// Returns ArrowComponent subobject
	class UArrowComponent* GetArrowComponent() const { return ArrowComponent; }
#endif

	// Returns CameraBoomComponent subobject
	USpringArmComponent* GetCameraBoomComponent() const { return CameraBoomComponent; }

	// Returns FollowCameraComponent subobject
	UCameraComponent* GetFollowCameraComponent() const { return FollowCameraComponent; }

	// Returns CharacterMoverComponent subobject
	UCharacterMoverComponent* GetCharacterMoverComponent() const { return CharacterMoverComponent; }

protected:
	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/**
	 * Entry point for input production. Override this function in a native class to author input for the next
	 * simulation frame. Consider also calling Super method.
	 */
	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

	// Called for movement input
	void Move(const FInputActionValue& Value);

	// Called for looking input
	void Look(const FInputActionValue& Value);

private:
	/**
	 * The CapsuleComponent being used for movement collision (by CharacterMovement). Always treated as being vertically
	 * aligned in simple collision check functions.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	// The main skeletal mesh associated with this Character (optional sub-object). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

#if WITH_EDITORONLY_DATA
	// Component shown in the editor only to indicate character facing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UArrowComponent> ArrowComponent;
#endif

	// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Camera", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> CameraBoomComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Camera", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FollowCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Movement", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCharacterMoverComponent> CharacterMoverComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> LookAction;
};