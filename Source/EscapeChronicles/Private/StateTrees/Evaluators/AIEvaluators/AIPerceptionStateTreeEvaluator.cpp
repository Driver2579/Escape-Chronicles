// Fill out your copyright notice in the Description page of Project Settings.

#include "StateTree/Evaluators/AIEvaluators/AIPerceptionStateTreeEvaluator.h"

#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

void FAIPerceptionStateTreeEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.Pawn);
	check(IsValid(InstanceData.Pawn->GetController()));
	check(InstanceData.Pawn->GetController()->IsA<AAIController>());
#endif

	InstanceData.AIController = CastChecked<AAIController>(InstanceData.Pawn->GetController());
}

void FAIPerceptionStateTreeEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

#if DO_CHECK
	check(InstanceData.AIController.IsValid());
#endif

	const UAIPerceptionComponent* AIPerceptionComponent = InstanceData.AIController->GetAIPerceptionComponent();

	if (!ensure(IsValid(AIPerceptionComponent)))
	{
		return;
	}

	TArray<AActor*> SeenActors;
	AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), SeenActors);

	/**
	 * Update the SeenActors array with a new one and broadcast the delegate if it was changed. We use MoveTemp to avoid
	 * unnecessary copies.
	 */
	if (SeenActors != InstanceData.SeenActors)
	{
		InstanceData.SeenActors = MoveTemp(SeenActors);
		Context.BroadcastDelegate(InstanceData.OnSeenActorsChangedDispatcher);
	}
}