#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BMWBaseCharacter.generated.h"

class UBMWAttributeComponent;
class UBehaviorTree;

UCLASS()
class WUKONG_API ABMWBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	void BeginPlay();
	
	ABMWBaseCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBMWAttributeComponent* AttributeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead;

	virtual void Die();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	UBMWAttributeComponent* GetAttributeComponent() const { return AttributeComp; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsAlive() const { return !bIsDead; }

protected:
	// 受击蒙太奇
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	UAnimMontage* HitReactMontage;

	// 死亡蒙太奇
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animation")
	UAnimMontage* DeathMontage;

	// 监听属性变化的函数
	UFUNCTION()
	virtual void OnHealthChanged(AActor* InstigatorActor, UBMWAttributeComponent* OwningComp, float NewHealth, float Delta);

	//  计算攻击者方向
	FName GetHitDirection(AActor* Attacker);

	// 执行受击逻辑
	void PlayHitReact(AActor* InstigatorActor);

protected:
	// 记录出生时的位置和旋转
	FTransform InitialTransform;

public:
	// 重置角色状态
	virtual void ResetCharacter();

	bool bSystemHandleDeath = true;

public:
	// 当前攻击伤害 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CurrentAttackDamage = 15.0f;

protected:
	// 隐藏尸体的定时器句柄
	FTimerHandle TimerHandle_HideCorpse;
};
