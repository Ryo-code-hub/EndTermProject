#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BMWAIMinionController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;

UCLASS()
class WUKONG_API ABMWAIMinionController : public AAIController
{
	GENERATED_BODY()

public:
	ABMWAIMinionController();

	// 视觉配置容器
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UAISenseConfig_Sight* SightConfig;

	// 当感知更新时触发的回调函数
	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

protected:
	// 当 AI 附身到怪物身上时触发
	virtual void OnPossess(APawn* InPawn) override;

protected:
	// 行为树组件：执行逻辑
	UPROPERTY(VisibleAnywhere, Category = "AI")
	UBehaviorTreeComponent* BehaviorTreeComp;

	// 黑板组件：存储记忆
	UPROPERTY(VisibleAnywhere, Category = "AI")
	UBlackboardComponent* BlackboardComp;

	// 感知组件：视觉/听觉
	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAIPerceptionComponent* PerceptionComp;
};