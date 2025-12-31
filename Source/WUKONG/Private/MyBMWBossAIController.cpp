#include "MyBMWBossAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "MyBMWBossCharacter.h"

AMyBMWBossAIController::AMyBMWBossAIController()
{
    // 创建组件
    BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

    // 配置视力 (Boss 视野更远)
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 4000.0f;
    SightConfig->PeripheralVisionAngleDegrees = 180.0f;

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AMyBMWBossAIController::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);

    // 绑定感知事件
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AMyBMWBossAIController::OnTargetDetected);

    // 启动行为树
    if (AMyBMWBossCharacter *BossChar = Cast<AMyBMWBossCharacter>(InPawn))
    {
        if (BossChar->BossBehaviorTree)
        {
            BlackboardComp->InitializeBlackboard(*BossChar->BossBehaviorTree->BlackboardAsset);
            BehaviorTreeComp->StartTree(*BossChar->BossBehaviorTree);
        }
    }
}

void AMyBMWBossAIController::OnTargetDetected(AActor *Actor, FAIStimulus Stimulus)
{
    if (Stimulus.WasSuccessfullySensed())
    {
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
        BlackboardComp->SetValueAsBool(TEXT("HasSeenPlayer"), true);
        SetFocus(Actor);
    }
}

void AMyBMWBossAIController::SetPhaseTwoState(bool bIsPhase2)
{
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsBool(TEXT("IsPhaseTwo"), bIsPhase2);
    }
}