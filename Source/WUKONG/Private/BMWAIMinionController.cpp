#include "BMWAIMinionController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BMWBaseCharacter.h"

ABMWAIMinionController::ABMWAIMinionController()
{
    BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

    // 1. 创建视觉配置
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    // 2. 配置参数
    SightConfig->SightRadius = 1500.0f; // 看得见的距离（15米）
    SightConfig->LoseSightRadius = 2000.0f; // 超过这个距离就算跟丢了
    SightConfig->PeripheralVisionAngleDegrees = 90.0f; // 视野角度（90度）

    // 设置能识别哪种阵营（这里设为识别所有人，包括中立）
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // 3. 将配置应用到感知组件
    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ABMWAIMinionController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 4. 绑定事件：当有东西进入或离开视野时触发
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ABMWAIMinionController::OnTargetDetected);

    // 1. 获取怪物身上挂着的行为树资源
    // 假设你在怪物的蓝图里设置了一个变量叫 BehaviorTreeAsset
    if (ABMWBaseCharacter* BMWChar = Cast<ABMWBaseCharacter>(InPawn))
    {
        // 这里假设你的 Character 类里有个 UBehaviorTree* 类型的变量
        // 如果没有，你需要在 Character 里加一个，或者在这里直接硬编码加载（不推荐）
        if (BMWChar->BehaviorTreeAsset)
        {
            // 初始化黑板
            BlackboardComp->InitializeBlackboard(*BMWChar->BehaviorTreeAsset->BlackboardAsset);
            // 运行行为树
            BehaviorTreeComp->StartTree(*BMWChar->BehaviorTreeAsset);
        }
    }
}

void ABMWAIMinionController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    // 检查是不是主角（通过你的基类判断，或者简单的 Tag 判断）
    // 假设你的主角类叫 ABMWPlayerCharacter
    // if (Actor->IsA<ABMWPlayerCharacter>()) 
    // {
        // Stimulus.WasSuccessfullySensed() 为 true 代表刚看见，false 代表刚丢失
    if (Stimulus.WasSuccessfullySensed())
    {
        // 看见主角了！
        // 1. 设置黑板里的 TargetActor
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
        // 2. 设置 HasSeenPlayer 为 true
        BlackboardComp->SetValueAsBool(TEXT("HasSeenPlayer"), true);
    }
    else
    {
        // 跟丢了...
        // 可以在这里加个延迟再清除，或者立刻清除
        BlackboardComp->SetValueAsBool(TEXT("HasSeenPlayer"), false);
        // TargetActor 通常保留，以便 AI 跑到最后一次看见的位置
    }
    // }
}