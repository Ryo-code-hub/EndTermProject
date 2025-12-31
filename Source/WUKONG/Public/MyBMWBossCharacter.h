#pragma once

#include "CoreMinimal.h"
#include "BMWBaseCharacter.h"
#include "MyBMWBossCharacter.generated.h"

class UBMWAttributeComponent;
class UWidgetComponent;
class UBehaviorTree;

UCLASS()
class WUKONG_API AMyBMWBossCharacter : public ABMWBaseCharacter
{
	GENERATED_BODY()

public:
	AMyBMWBossCharacter();

	// AI 相关
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UBehaviorTree *BossBehaviorTree;

	// 获取是否处于二阶段
	bool IsPhaseTwo() const { return bIsPhaseTwo; }

protected:
	virtual void PostInitializeComponents() override;

	// 用于恢复玩家控制的函数
	void ResumeFight();

	// 定时器句柄
	FTimerHandle TimerHandle_ResumeFight;

	// 血条组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent *HealthBarWidget;

	// 战斗配置
	// 阶段转换的阈值
	UPROPERTY(EditDefaultsOnly, Category = "Boss Phase")
	float PhaseTwoThreshold = 0.5f;

	// 是否处于二阶段
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss Phase")
	bool bIsPhaseTwo;

	// 转阶段时播放的蒙太奇
	UPROPERTY(EditDefaultsOnly, Category = "Boss Phase")
	UAnimMontage *PhaseTransitionMontage;

	// 死亡动作蒙太奇
	//UPROPERTY(EditDefaultsOnly, Category = "Combat")
	//UAnimMontage *DeathMontage;

	// 受击蒙太奇
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage *HitMontage;

	// 二阶段的攻击动作池
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<UAnimMontage *> PhaseTwoAttacks;

	// 函数
	//UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, UBMWAttributeComponent* OwningComp, float NewHealth, float Delta) override;

	void EnterPhaseTwo();
	void Die();

public:
	void ProcessHit(FHitResult HitResult);
	void StartWeaponCollision() { HitActors.Empty(); }
	void EndWeaponCollision() { HitActors.Empty(); }

protected:
	TArray<AActor*> HitActors;

public:
	// 用于在 UI 上显示的 Boss 名字
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FText BossDisplayName;

};