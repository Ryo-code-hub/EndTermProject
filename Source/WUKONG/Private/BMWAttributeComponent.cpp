	#include "BMWAttributeComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UBMWAttributeComponent::UBMWAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Health = MaxHealth;
	Mana = MaxMana;
	Stamina = MaxStamina;
	MaxXP = XPToNextLevel;
}

void UBMWAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	// 每 0.1秒恢复体力
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_StaminaRegen, this, &UBMWAttributeComponent::RegenStamina, 0.05f, true);
}

bool UBMWAttributeComponent::ApplyHealthChange(AActor* Instigator, float Delta)
{
	if (Health <= 0.0f && Delta < 0.0f) return false;

	float OldValue = Health;
	Health = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);
	float ActualDelta = Health - OldValue;

	if (ActualDelta != 0.0f)
	{
		OnHealthChanged.Broadcast(Instigator, this, Health, ActualDelta);
	}
	return ActualDelta != 0.0f;
}

bool UBMWAttributeComponent::ApplyManaChange(float Delta)
{
	float OldValue = Mana;
	Mana = FMath::Clamp(Mana + Delta, 0.0f, MaxMana);

	if (Mana != OldValue)
	{
		OnManaChanged.Broadcast(nullptr, this, Mana, Mana - OldValue);
		return true;
	}
	return false;
}

// 体力变更函数：增加了记录消耗时间的逻辑
bool UBMWAttributeComponent::ApplyStaminaChange(float Delta)
{
	// 如果是消耗体力 (Delta < 0)
	if (Delta < 0.0f)
	{
		// 检查是否足够扣除
		if (Stamina + Delta < 0.0f)
		{
			return false; // 体力不足，操作失败
		}

		// [新增] 只要成功扣除体力，就记录当前时间，用于暂停恢复逻辑
		if (GetWorld())
		{
			LastStaminaConsumeTime = GetWorld()->GetTimeSeconds();
		}
	}

	float OldValue = Stamina;
	Stamina = FMath::Clamp(Stamina + Delta, 0.0f, MaxStamina);

	if (Stamina != OldValue)
	{
		OnStaminaChanged.Broadcast(nullptr, this, Stamina, Stamina - OldValue);
	}
	return true;
}

// 自动恢复逻辑：增加了延迟判断
void UBMWAttributeComponent::RegenStamina()
{
	// 检查恢复延迟
	// 如果刚刚才消耗过体力，还没过冷却期 (StaminaRegenDelay)，则不恢复
	if (GetWorld())
	{
		double CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime - LastStaminaConsumeTime < StaminaRegenDelay)
		{
			return; // 还在冷却中，跳过本次恢复
		}
	}

	// 简单的自动恢复逻辑：如果没有满，就加一点
	if (Stamina < MaxStamina)
	{
		ApplyStaminaChange(2.0f); // 每0.1秒回2.0点，即每秒20点
	}
}

bool UBMWAttributeComponent::TryConsumeActionStamina(float Amount)
{
	// 复用核心逻辑
	return ApplyStaminaChange(-Amount);
}

void UBMWAttributeComponent::HandleSprintingStamina(bool bIsMoving, bool bIsRunning, float DeltaTime)
{
	if (bIsMoving && bIsRunning)
	{
		// 必须调用 ApplyStaminaChange，这样它会更新 LastStaminaConsumeTime，从而暂停自动回体
		ApplyStaminaChange(-0.0f * DeltaTime);
	}
}

bool UBMWAttributeComponent::TryConsumeLightAttackStamina()
{
	return ApplyStaminaChange(-LightAttackCost);
}

bool UBMWAttributeComponent::ConsumeHeavyChargeStamina(float DeltaTime)
{
	// 如果当前体力已经是 0，说明耗尽了，返回 false 通知外部释放
	if (Stamina <= 0.0f)
	{
		return false;
	}

	// 计算本帧需要消耗的量
	float Cost = HeavyChargeCostPerSec * DeltaTime;

	float OldValue = Stamina;
	Stamina = FMath::Clamp(Stamina - Cost, 0.0f, MaxStamina);

	if (GetWorld())
	{
		LastStaminaConsumeTime = GetWorld()->GetTimeSeconds();
	}

	// 广播 UI
	if (Stamina != OldValue)
	{
		OnStaminaChanged.Broadcast(nullptr, this, Stamina, Stamina - OldValue);
	}
	return Stamina > 0.0f;
}

bool UBMWAttributeComponent::HasEnoughStaminaToStart()
{
	// 定义一个极小值，或者根据需求定义一个“起手消耗”
	// 只有当前体力大于这个值，才允许进入蓄力状态
	return Stamina > 5.0f;
}

void UBMWAttributeComponent::AddXP(float DeltaXP)
{
	CurrentXP += DeltaXP;

	// 升级逻辑
	if (CurrentXP >= MaxXP)
	{
		CurrentXP -= MaxXP;
		Level++;
		MaxXP = MaxXP * 1.5f;

		// 升级回满状态
		Health = MaxHealth;
		Mana = MaxMana;
		Stamina = MaxStamina;

	}

	OnXPChanged.Broadcast(nullptr, this, CurrentXP, DeltaXP);
}