#include "BMWAIMinionController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BMWBaseCharacter.h"
#include "BMWMinionCharacter.h" 
#include "BMWPlayerCharacter.h"

ABMWAIMinionController::ABMWAIMinionController()
{
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 2000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ABMWAIMinionController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABMWAIMinionController::OnTargetDetected);

	if (ABMWBaseCharacter* BMWChar = Cast<ABMWBaseCharacter>(InPawn))
	{
		if (BMWChar->BehaviorTreeAsset)
		{
			BlackboardComp->InitializeBlackboard(*BMWChar->BehaviorTreeAsset->BlackboardAsset);
			BehaviorTreeComp->StartTree(*BMWChar->BehaviorTreeAsset);
		}
	}
}

void ABMWAIMinionController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// 阵营过滤
	// 如果看到的不是玩家，直接忽略，不产生仇恨
	if (!Actor || !Actor->IsA<ABMWPlayerCharacter>())
	{
		return;
	}

	ABMWMinionCharacter* MyMinion = Cast<ABMWMinionCharacter>(GetPawn());

	if (Stimulus.WasSuccessfullySensed())
	{
		// 只有确认是玩家，才写入黑板
		BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
		BlackboardComp->SetValueAsBool(TEXT("HasSeenPlayer"), true);

		// 强制注视玩家
		SetFocus(Actor);

		if (MyMinion && !MyMinion->bHasAggro)
		{
			MyMinion->bHasAggro = true;
			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Sight Triggered Aggro!"));
		}
	}
	else
	{
		// 视野丢失不做处理，保持追逐
	}
}