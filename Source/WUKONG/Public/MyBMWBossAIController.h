#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "MyBMWBossAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class WUKONG_API AMyBMWBossAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMyBMWBossAIController();

	void SetPhaseTwoState(bool bIsPhase2);

protected:
	virtual void OnPossess(APawn *InPawn) override;

	// 感知回调
	UFUNCTION()
	void OnTargetDetected(AActor *Actor, FAIStimulus Stimulus);

	// 组件
	UPROPERTY(VisibleAnywhere, Category = "AI")
	UBehaviorTreeComponent *BehaviorTreeComp;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	UBlackboardComponent *BlackboardComp;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAIPerceptionComponent *PerceptionComp;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAISenseConfig_Sight *SightConfig;
};