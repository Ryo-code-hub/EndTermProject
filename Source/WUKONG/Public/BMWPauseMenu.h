#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "BMWPauseMenu.generated.h"

class UButton;
class UWidgetSwitcher;
class USoundClass;
class USoundMix;

UCLASS()
class WUKONG_API UBMWPauseMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;
	// 只执行一次的初始化函数
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "BMW System")
	void TogglePauseMenu();

protected:
	// ========================================================================
	// UI 绑定区 (BindWidget)
	// C++ 变量名必须与 UMG 蓝图中的控件名严格一致，否则无法绑定
	// ========================================================================

	//  监听键盘按键 (Tab/Esc)
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// --- 顶部导航栏 (Tab 切换) ---
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Tab_Cultivation; // 修行页面
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Tab_Relics;      // 根器页面
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Tab_Equipment;   // 披挂页面
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Tab_Inventory;   // 行囊页面
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Tab_Journal;     // 游记页面
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Tab_Settings;    // 设置页面

	// 核心内容切换器，用于在不同功能页面间切换
	UPROPERTY(meta = (BindWidget)) UWidgetSwitcher* MainContentSwitcher;

	// 设置页面组件
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Sub_Video;   // 画质子菜单
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Sub_Audio;   // 音频子菜单
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Sub_Resume;  // 继续游戏按钮
	UPROPERTY(meta = (BindWidget)) UButton* Btn_Sub_Quit;    // 唤起退出弹窗按钮

	// 设置选项切换器：用于在画质和音频详情页间切换
	UPROPERTY(meta = (BindWidget)) UWidgetSwitcher* SettingsOptionSwitcher;

	// 退出确认弹窗
	UPROPERTY(meta = (BindWidget)) UWidget* QuitConfirmPopup; // 弹窗容器
	UPROPERTY(meta = (BindWidget)) UButton* Btn_ConfirmQuit;  // 确认退出
	UPROPERTY(meta = (BindWidget)) UButton* Btn_CancelQuit;   // 取消退出

	// 画质调节控件
	// 通过 GameUserSettings 接口直接控制渲染参数
	UPROPERTY(meta = (BindWidget)) UComboBoxString* Combo_Resolution;  // 分辨率
	UPROPERTY(meta = (BindWidget)) UComboBoxString* Combo_WindowMode;  // 窗口模式
	UPROPERTY(meta = (BindWidget)) UComboBoxString* Combo_Quality;     // 画质等级
	UPROPERTY(meta = (BindWidget)) UCheckBox* Check_VSync;             // 垂直同步

	// 音频调节控件
	UPROPERTY(meta = (BindWidget)) USlider* Slider_MasterVolume;       // 主音量
	UPROPERTY(meta = (BindWidget)) USlider* Slider_MusicVolume;        // 音乐音量
	UPROPERTY(meta = (BindWidget)) USlider* Slider_SFXVolume;          // 音效音量

	// ========================================================================
	// 资源配置区 (EditDefaultsOnly)
	// 通过暴露音频类接口，实现逻辑代码与具体音频资产的解耦
	// ========================================================================

	UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
	USoundMix* GlobalSoundMix; // 全局混音器，用于动态调整音量

	UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
	USoundClass* SC_Master;    // 主音量 SoundClass

	UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
	USoundClass* SC_Music;     // 音乐 SoundClass

	UPROPERTY(EditDefaultsOnly, Category = "Audio Config")
	USoundClass* SC_SFX;       // 音效 SoundClass

private:
	// ========================================================================
	// 内部逻辑处理函数
	// ========================================================================

	// 导航回调
	UFUNCTION() void OnTabChanged_Cultivation();
	UFUNCTION() void OnTabChanged_Relics();
	UFUNCTION() void OnTabChanged_Equipment();
	UFUNCTION() void OnTabChanged_Inventory();
	UFUNCTION() void OnTabChanged_Journal();
	UFUNCTION() void OnTabChanged_Settings();

	UFUNCTION() void OnSubMenu_Video();
	UFUNCTION() void OnSubMenu_Audio();
	UFUNCTION() void OnSubMenu_Resume();
	UFUNCTION() void OnSubMenu_Quit();

	// 弹窗逻辑
	UFUNCTION() void OnRealQuit();
	UFUNCTION() void OnCancelQuit();

	// 画质设置回调
	UFUNCTION() void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION() void OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION() void OnQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION() void OnVSyncChanged(bool bIsChecked);

	// 音频设置回调
	UFUNCTION() void OnMasterVolumeChanged(float Value);
	UFUNCTION() void OnMusicVolumeChanged(float Value);
	UFUNCTION() void OnSFXVolumeChanged(float Value);

	// 辅助功能
	void InitializeSettings(); // 读取本地配置并回显到 UI
	void ApplyVideoSettings(); // 应用并保存画质更改
	void SetSoundClassVolume(USoundClass* TargetClass, float Volume); // 安全音量设置封装
	void SetupInputMode(bool bIsMenuOpen); // 输入模式管理
};