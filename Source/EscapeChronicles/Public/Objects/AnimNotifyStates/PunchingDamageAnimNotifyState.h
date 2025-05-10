// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PunchingDamageAnimNotifyState.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="Punching Damage"))
class ESCAPECHRONICLES_API UPunchingDamageAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
private:
	UPROPERTY(EditAnywhere)
	FName SocketName;

	UPROPERTY(EditAnywhere)
	float Radius = 50.0f;

	UPROPERTY(EditAnywhere)
	float TickInterval = 0.5f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UGameplayEffect> DamagingEffectClass;
	
	UPROPERTY(EditAnywhere)
	float DamagingEffectLevel = 1.f;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	bool bDrawDebugSpheres = false;
#endif
	
	float ElapsedTime = 0.0f;
	bool bIsTargetDamaged = false;
	TWeakObjectPtr<AActor> StateOwner = nullptr;
	
	void FindAndDamageTarget(const USkeletalMeshComponent* MeshComp);
	void DamageTarget(const class AEscapeChroniclesCharacter* TargetCharacter);
};
