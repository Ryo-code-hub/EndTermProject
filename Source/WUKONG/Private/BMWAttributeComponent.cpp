#include "BMWAttributeComponent.h"

UBMWAttributeComponent::UBMWAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxHealth = 100.0f;
	Health = MaxHealth;
}

bool UBMWAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	// 如果已经死了（血量为0）且还在扣血，就忽略，防止鞭尸
	// 或者如果 Delta 是 0，也不处理
	if (Health <= 0.0f && Delta < 0.0f)
	{
		return false;
	}

	// 记录修改前的血量
	float OldHealth = Health;

	// 修改血量：使用 Clamp 防止血量超过上限或低于 0
	Health = FMath::Clamp(Health + Delta, 0.0f, MaxHealth);

	// 计算实际变化量（因为可能被 Clamp 截断了）
	float ActualDelta = Health - OldHealth;

	// --- 3. 广播给 UI (关键) ---
	// 告诉所有人：血量变了！UI 快更新！
	// (Instigator, Component本身, 新血量, 变化量)
	if (ActualDelta != 0.0f)
	{
		OnHealthChanged.Broadcast(InstigatorActor, this, Health, ActualDelta);
	}

	return ActualDelta != 0;
}

float UBMWAttributeComponent::GetHealth() const
{
	return Health;
}