#include "BMWMainHUD.h"
#include "Components/ProgressBar.h"
#include "BMWBaseCharacter.h"

void UBMWMainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. 获取当前玩家的角色
	if (APawn* MyPawn = GetOwningPlayerPawn())
	{
		// 2. 强转为我们的基类
		if (ABMWBaseCharacter* MyChar = Cast<ABMWBaseCharacter>(MyPawn))
		{
			// 3. 获取组件
			if (UBMWAttributeComponent* AttributeComp = MyChar->GetAttributeComponent())
			{
				// 4. 绑定委托 (AddDynamic)
				AttributeComp->OnHealthChanged.AddDynamic(this, &UBMWMainHUD::OnHealthChanged);

				// 5. 初始化：刚进游戏手动调一次，不然血条是空的或满的（取决于默认值）
				// 假设 AttributeComp 有 GetMaxHealth() 和 GetHealth()，你需要确保这俩是 public 的
				// 这里为了演示，假设你有 GetMaxHealth()
				// OnHealthChanged(nullptr, AttributeComp, AttributeComp->GetHealth(), 0.0f);
			}
		}
	}
}

void UBMWMainHUD::OnHealthChanged(AActor* InstigatorActor, UBMWAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (HealthBar && OwningComp)
	{
		// 假设你在 AttributeComponent 里公开了 MaxHealth 变量，或者写了 GetMaxHealth() 函数
		// 这里假设 MaxHealth 是 public 的或者有 Getter
		float MaxHealth = 100.0f; // 临时防崩，请替换为 OwningComp->MaxHealth;

		// 如果 MaxHealth 是 protected 的，去组件里加个 GetMaxHealth() 函数
		// float Percent = NewHealth / OwningComp->GetMaxHealth();

		float Percent = NewHealth / MaxHealth;
		HealthBar->SetPercent(Percent);
	}
}