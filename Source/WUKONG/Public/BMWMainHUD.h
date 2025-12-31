#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BMWAttributeComponent.h" // 引用属性组件
#include "Components/Button.h"
#include "BMWMainHUD.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;

UCLASS()
class WUKONG_API UBMWMainHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    // 供外部调用的初始化
    UFUNCTION(BlueprintCallable)
    void InitializeHUD(ABMWBaseCharacter* Character);

    // 显示/隐藏 Boss 血条
    UFUNCTION(BlueprintCallable)
    void ShowBossHealth(AActor* BossActor);

    UFUNCTION(BlueprintCallable)
    void HideBossHealth();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // UI 控件绑定

    // 左下角状态
    UPROPERTY(meta = (BindWidget)) UProgressBar* HealthBar;
    UPROPERTY(meta = (BindWidget)) UProgressBar* ManaBar;
    UPROPERTY(meta = (BindWidget)) UProgressBar* StaminaBar;

    // 右下角棍势
    UPROPERTY(meta = (BindWidget)) UProgressBar* FocusBar; // 长条
    UPROPERTY(meta = (BindWidget)) UImage* FocusBean_1;    // 第1颗豆
    UPROPERTY(meta = (BindWidget)) UImage* FocusBean_2;    // 第2颗豆
    UPROPERTY(meta = (BindWidget)) UImage* FocusBean_3;    // 第3颗豆
    //大招提示图标 UButton* 类型
    UPROPERTY(meta = (BindWidget)) class UButton* SpecialSkillIcon;
    // 声明闪烁动画
    UPROPERTY(Transient, meta = (BindWidgetAnim)) UWidgetAnimation* UltimateReadyAnim;

    // 右上角经验
    UPROPERTY(meta = (BindWidget)) UProgressBar* XPBar;
    UPROPERTY(meta = (BindWidget)) UTextBlock* LevelText;

    // 底部 Boss 血条
    UPROPERTY(meta = (BindWidget)) UWidget* BossHealthContainer; // 用来控制显示/隐藏
    UPROPERTY(meta = (BindWidget)) UProgressBar* BossHealthBar;
    UPROPERTY(meta = (BindWidget)) UTextBlock* BossNameText;

    // 颜色配置

    UPROPERTY(EditAnywhere, Category = "UI Config")
    FLinearColor BeanColor_Level1 = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // 1阶颜色

    UPROPERTY(EditAnywhere, Category = "UI Config")
    FLinearColor BeanColor_Level2 = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // 2阶颜色

    UPROPERTY(EditAnywhere, Category = "UI Config")
    FLinearColor BeanColor_Level3 = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 3阶颜色

    UPROPERTY(EditAnywhere, Category = "UI Config")
    FLinearColor BeanInactiveColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.5f); // 暗色
    // 内部函数
    UFUNCTION() void UpdateHealth(AActor* Instigator, UBMWAttributeComponent* Comp, float NewVal, float Delta);
    UFUNCTION() void UpdateMana(AActor* Instigator, UBMWAttributeComponent* Comp, float NewVal, float Delta);
    UFUNCTION() void UpdateStamina(AActor* Instigator, UBMWAttributeComponent* Comp, float NewVal, float Delta);
    UFUNCTION() void UpdateXP(AActor* Instigator, UBMWAttributeComponent* Comp, float NewVal, float Delta);

    // 专门处理棍势逻辑
    UFUNCTION() void UpdateFocus(float NewFocusPoint);

    // 当前绑定的 Boss
    TWeakObjectPtr<AActor> CurrentBoss;
};