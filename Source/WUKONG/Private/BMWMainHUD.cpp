#include "BMWMainHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "BMWPlayerCharacter.h" 
#include "BMWAttributeComponent.h"
#include "MyBMWBossCharacter.h"

void UBMWMainHUD::NativeConstruct()
{
    Super::NativeConstruct();

    // 隐藏 Boss 血条默认
    if (BossHealthContainer) BossHealthContainer->SetVisibility(ESlateVisibility::Hidden);
}

void UBMWMainHUD::InitializeHUD(ABMWBaseCharacter* Character)
{
    if (!Character) return;

    // 绑定属性组件 (血/蓝/耐/XP)
    if (UBMWAttributeComponent* AttributeComp = Character->GetAttributeComponent())
    {
        AttributeComp->OnHealthChanged.AddDynamic(this, &UBMWMainHUD::UpdateHealth);
        AttributeComp->OnManaChanged.AddDynamic(this, &UBMWMainHUD::UpdateMana);
        AttributeComp->OnStaminaChanged.AddDynamic(this, &UBMWMainHUD::UpdateStamina);
        AttributeComp->OnXPChanged.AddDynamic(this, &UBMWMainHUD::UpdateXP);

        // 防御性绑定
        AttributeComp->OnHealthChanged.RemoveDynamic(this, &UBMWMainHUD::UpdateHealth); // 先移除
        AttributeComp->OnHealthChanged.AddDynamic(this, &UBMWMainHUD::UpdateHealth);    // 再绑定

        AttributeComp->OnManaChanged.RemoveDynamic(this, &UBMWMainHUD::UpdateMana);
        AttributeComp->OnManaChanged.AddDynamic(this, &UBMWMainHUD::UpdateMana);

        AttributeComp->OnStaminaChanged.RemoveDynamic(this, &UBMWMainHUD::UpdateStamina);
        AttributeComp->OnStaminaChanged.AddDynamic(this, &UBMWMainHUD::UpdateStamina);

        AttributeComp->OnXPChanged.RemoveDynamic(this, &UBMWMainHUD::UpdateXP);
        AttributeComp->OnXPChanged.AddDynamic(this, &UBMWMainHUD::UpdateXP);

        // 初始化显示一次
        UpdateHealth(nullptr, AttributeComp, AttributeComp->GetHealth(), 0);
        UpdateMana(nullptr, AttributeComp, AttributeComp->GetMana(), 0);
        UpdateStamina(nullptr, AttributeComp, AttributeComp->GetStamina(), 0);
        UpdateXP(nullptr, AttributeComp, AttributeComp->GetXPPercent() * 1000.0f, 0); // 这里的传参有些 tricky，主要为了触发逻辑
        if (LevelText) LevelText->SetText(FText::AsNumber(AttributeComp->GetLevel()));
    }

    // 绑定棍势
    if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(Character))
    {
        // 先移除再绑定
        Player->OnFocusChanged.RemoveDynamic(this, &UBMWMainHUD::UpdateFocus);
        Player->OnFocusChanged.AddDynamic(this, &UBMWMainHUD::UpdateFocus);
        UpdateFocus(0.0f);
    }
}

// 普通属性更新
void UBMWMainHUD::UpdateHealth(AActor* Instigator, UBMWAttributeComponent* Comp, float NewVal, float Delta)
{
    if (HealthBar && Comp) HealthBar->SetPercent(NewVal / Comp->GetMaxHealth());
}

void UBMWMainHUD::UpdateMana(AActor* Instigator, UBMWAttributeComponent* Comp, float NewVal, float Delta)
{
    if (ManaBar && Comp) ManaBar->SetPercent(NewVal / Comp->GetMaxMana());
}

void UBMWMainHUD::UpdateStamina(AActor* Instigator, UBMWAttributeComponent* Comp, float NewVal, float Delta)
{
    if (StaminaBar && Comp) StaminaBar->SetPercent(NewVal / Comp->GetMaxStamina());
}

void UBMWMainHUD::UpdateXP(AActor* Instigator, UBMWAttributeComponent* Comp, float NewVal, float Delta)
{
    if (XPBar && Comp) XPBar->SetPercent(Comp->GetXPPercent());
    if (LevelText && Comp) LevelText->SetText(FText::AsNumber(Comp->GetLevel()));
}

// 棍势条逻辑
void UBMWMainHUD::UpdateFocus(float NewFocusPoint)
{
    // NewFocusPoint 范围 0.0 ~ 4.0
    // 整数部分 = 豆子数，小数部分 = 进度条

    int32 BeansCount = FMath::FloorToInt(NewFocusPoint);
    float BarProgress = NewFocusPoint - BeansCount;

    if (BeansCount >= 4)
    {
        BeansCount = 4;
        BarProgress = 1.0f; // 条也满了
    }

    // 更新长条
    if (FocusBar) FocusBar->SetPercent(BarProgress);

    // 更新豆子颜色 
    // 确定当前最高激活颜色
    FLinearColor CurrentActiveColor = BeanColor_Level1; // 默认白色
    if (BeansCount >= 2) CurrentActiveColor = BeanColor_Level2; // 到了2阶全变金
    if (BeansCount >= 3) CurrentActiveColor = BeanColor_Level3; // 到了3阶全变红

    // 应用这个颜色给所有激活中的豆子
    if (FocusBean_1) FocusBean_1->SetColorAndOpacity(BeansCount >= 1 ? CurrentActiveColor : BeanInactiveColor);
    if (FocusBean_2) FocusBean_2->SetColorAndOpacity(BeansCount >= 2 ? CurrentActiveColor : BeanInactiveColor);
    if (FocusBean_3) FocusBean_3->SetColorAndOpacity(BeansCount >= 3 ? CurrentActiveColor : BeanInactiveColor);
    // ========================================================
    // ===  大招图标逻辑 (显示 + 闪烁动画) ===
    // ========================================================

    if (SpecialSkillIcon)
    {
        // 设定阈值
        const bool bIsReady = (NewFocusPoint >= 4.0f);

        // 如果大招就绪 (>= 4.0)
        if (bIsReady)
        {
            // 防抖检查
            // 只有当图标当前还是隐藏状态时才执行显示+播放动画。
            // 如果它已经是 Visible ，说明动画已经在播放中，不打断
            if (SpecialSkillIcon->GetVisibility() != ESlateVisibility::Visible)
            {
                SpecialSkillIcon->SetVisibility(ESlateVisibility::Visible); // 显示图标

                if (UltimateReadyAnim)
                {
                    // 播放动画
                    // 参数: 动画指针, 开始时间, 循环次数(0=无限循环), 播放模式
                    PlayAnimation(UltimateReadyAnim, 0.0f, 0);
                }
            }
        }
        // 如果大招未就绪
        else
        {
            if (SpecialSkillIcon->GetVisibility() != ESlateVisibility::Hidden)
            {
                SpecialSkillIcon->SetVisibility(ESlateVisibility::Hidden); // 隐藏图标

                if (UltimateReadyAnim)
                {
                    StopAnimation(UltimateReadyAnim); // 停止动画
                }
            }
        }
    }

}

// Boss 血条更新
void UBMWMainHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 如果 Boss 血条可见，且有 Boss 引用，每帧更新它的血量
    if (BossHealthContainer && BossHealthContainer->GetVisibility() == ESlateVisibility::Visible)
    {
        if (CurrentBoss.IsValid())
        {
            if (UBMWAttributeComponent* BossComp = CurrentBoss->FindComponentByClass<UBMWAttributeComponent>())
            {
                if (BossHealthBar) BossHealthBar->SetPercent(BossComp->GetHealth() / BossComp->GetMaxHealth());
            }
        }
        else
        {
            HideBossHealth(); // Boss 没了，隐藏血条
        }
    }
}

void UBMWMainHUD::ShowBossHealth(AActor* BossActor)
{
    // 记录当前 Boss 引用
    CurrentBoss = BossActor;

    // 显示血条容器
    if (BossHealthContainer)
    {
        BossHealthContainer->SetVisibility(ESlateVisibility::Visible);
    }

    // 设置 Boss 名字 (这是修改的核心部分)
    if (BossNameText)
    {
        // 尝试把传入的 Actor 转换成我们需要用的 Boss 类
        if (AMyBMWBossCharacter* Boss = Cast<AMyBMWBossCharacter>(BossActor))
        {
            // 如果转换成功，读取你在蓝图里填写的中文名 BossDisplayName
            BossNameText->SetText(Boss->BossDisplayName);
        }
        else
        {
            // 保底逻辑：如果不是在这个类体系下的 Boss，或者转换失败，显示 Actor 名字方便调试
            BossNameText->SetText(FText::FromString(BossActor->GetName()));
        }
    }
}
void UBMWMainHUD::HideBossHealth()
{
    if (BossHealthContainer) BossHealthContainer->SetVisibility(ESlateVisibility::Hidden);
    CurrentBoss = nullptr;
}