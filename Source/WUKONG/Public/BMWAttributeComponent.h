#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BMWAttributeComponent.generated.h"

class AActor;
class UBMWAttributeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAttributeChanged, AActor*, Instigator, UBMWAttributeComponent*, OwningComp, float, NewValue, float, Delta);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WUKONG_API UBMWAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBMWAttributeComponent();

	//**************************************************************************
	// 战斗体力接口
	// ========================================================================

	// 尝试扣除轻攻击体力
	// 返回 true: 体力足够，已扣除 -> 允许攻击
	// 返回 false: 体力不足 -> 禁止攻击
	UFUNCTION(BlueprintCallable, Category = "Attributes|Combat")
	bool TryConsumeLightAttackStamina();

	// 处理重击蓄力的持续消耗 (在Tick中调用)
	// 返回 true: 还有体力 -> 继续蓄力
	// 返回 false: 体力耗尽 -> 通知角色强制释放攻击
	UFUNCTION(BlueprintCallable, Category = "Attributes|Combat")
	bool ConsumeHeavyChargeStamina(float DeltaTime);

	// 检查是否有起手体力 (用于判断是否允许按下重击键)
	// 不需要扣除，只看够不够最低标准 (比如 > 0 或 > 5)
	UFUNCTION(BlueprintCallable, Category = "Attributes|Combat")
	bool HasEnoughStaminaToStart();

	// 尝试执行消耗体力的动作
	// 返回 true 表示体力足够且已扣除，返回 false 表示体力不足
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool TryConsumeActionStamina(float Amount);

	// 处理奔跑消耗
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void HandleSprintingStamina(bool bIsMoving, bool bIsRunning, float DeltaTime);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxHealth = 100.0f;

protected:
	virtual void BeginPlay() override;
	UPROPERTY()
	class UBMWMainHUD* MainHUDInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Mana;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxMana = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Stamina;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Costs")
	float LightAttackCost = 0.5f; // 轻攻击单次消耗

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Costs")
	float HeavyChargeCostPerSec = 2.0f; // 重击蓄力每秒消耗

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Costs")
	float StaminaRegenDelay = 1.0f; // 消耗体力后，等待多久开始恢复(秒)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	float CurrentXP = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	float XPToNextLevel = 1000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	int32 Level = 1;

public:
	// 广播事件
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnAttributeChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnAttributeChanged OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnAttributeChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnAttributeChanged OnXPChanged;

	// 接口

	// 通用的修改函数
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ApplyHealthChange(AActor* Instigator, float Delta);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ApplyManaChange(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ApplyStaminaChange(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void AddXP(float DeltaXP);

	// Getters
	UFUNCTION(BlueprintCallable) float GetMaxHealth() const { return MaxHealth; }
	UFUNCTION(BlueprintCallable) float GetHealth() const { return Health; }
	UFUNCTION(BlueprintCallable) float GetMaxMana() const { return MaxMana; }
	UFUNCTION(BlueprintCallable) float GetMana() const { return Mana; }
	UFUNCTION(BlueprintCallable) float GetMaxStamina() const { return MaxStamina; }
	UFUNCTION(BlueprintCallable) float GetStamina() const { return Stamina; }
	UFUNCTION(BlueprintCallable) float GetXPPercent() const { return (MaxXP > 0) ? CurrentXP / MaxXP : 0.0f; }
	UFUNCTION(BlueprintCallable) int32 GetLevel() const { return Level; }

protected:
	// 用于体力自动恢复
	void RegenStamina();
	FTimerHandle TimerHandle_StaminaRegen;
	float MaxXP = 1000.0f;

	double LastStaminaConsumeTime = 0.0;
};