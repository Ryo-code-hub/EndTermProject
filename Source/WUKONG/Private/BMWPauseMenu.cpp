#include "BMWPauseMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h" 
#include "GameFramework/GameUserSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"


// 初始化
void UBMWPauseMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 绑定顶部导航事件
	if (Btn_Tab_Cultivation) Btn_Tab_Cultivation->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnTabChanged_Cultivation);
	if (Btn_Tab_Relics)      Btn_Tab_Relics->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnTabChanged_Relics);
	if (Btn_Tab_Equipment)   Btn_Tab_Equipment->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnTabChanged_Equipment);
	if (Btn_Tab_Inventory)   Btn_Tab_Inventory->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnTabChanged_Inventory);
	if (Btn_Tab_Journal)     Btn_Tab_Journal->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnTabChanged_Journal);
	if (Btn_Tab_Settings)    Btn_Tab_Settings->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnTabChanged_Settings);

	// 绑定设置页子菜单事件
	if (Btn_Sub_Video)  Btn_Sub_Video->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnSubMenu_Video);
	if (Btn_Sub_Audio)  Btn_Sub_Audio->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnSubMenu_Audio);
	if (Btn_Sub_Resume) Btn_Sub_Resume->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnSubMenu_Resume);
	if (Btn_Sub_Quit)   Btn_Sub_Quit->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnSubMenu_Quit);

	// 绑定退出流程事件
	if (Btn_ConfirmQuit) Btn_ConfirmQuit->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnRealQuit);
	if (Btn_CancelQuit)  Btn_CancelQuit->OnClicked.AddDynamic(this, &UBMWPauseMenu::OnCancelQuit);

	// 绑定画质调节事件
	if (Combo_Resolution) Combo_Resolution->OnSelectionChanged.AddDynamic(this, &UBMWPauseMenu::OnResolutionChanged);
	if (Combo_WindowMode) Combo_WindowMode->OnSelectionChanged.AddDynamic(this, &UBMWPauseMenu::OnWindowModeChanged);
	if (Combo_Quality)    Combo_Quality->OnSelectionChanged.AddDynamic(this, &UBMWPauseMenu::OnQualityChanged);
	if (Check_VSync)      Check_VSync->OnCheckStateChanged.AddDynamic(this, &UBMWPauseMenu::OnVSyncChanged);

	// 绑定音频调节事件
	if (Slider_MasterVolume) Slider_MasterVolume->OnValueChanged.AddDynamic(this, &UBMWPauseMenu::OnMasterVolumeChanged);
	if (Slider_MusicVolume)  Slider_MusicVolume->OnValueChanged.AddDynamic(this, &UBMWPauseMenu::OnMusicVolumeChanged);
	if (Slider_SFXVolume)    Slider_SFXVolume->OnValueChanged.AddDynamic(this, &UBMWPauseMenu::OnSFXVolumeChanged);
}

// 每次显示都会执行的逻辑
void UBMWPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// 允许 UI 接收键盘焦点
	SetIsFocusable(true);

	// 数据回显
	InitializeSettings();

	// 设置默认 UI 状态
	if (MainContentSwitcher) MainContentSwitcher->SetActiveWidgetIndex(5); // 默认显示设置页
	if (SettingsOptionSwitcher) SettingsOptionSwitcher->SetActiveWidgetIndex(0); // 默认显示画质选项
	if (QuitConfirmPopup) QuitConfirmPopup->SetVisibility(ESlateVisibility::Hidden); // 确保弹窗隐藏
}
// ------------------------------------------------------------------------
// 数据初始化逻辑
// ------------------------------------------------------------------------
void UBMWPauseMenu::InitializeSettings()
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings) return;

	// 初始化分辨率
	if (Combo_Resolution)
	{
		Combo_Resolution->ClearOptions();
		// 演示
		Combo_Resolution->AddOption(TEXT("1920x1080"));
		Combo_Resolution->AddOption(TEXT("2560x1440"));
		Combo_Resolution->AddOption(TEXT("3840x2160"));

		FIntPoint CurrentRes = UserSettings->GetScreenResolution();
		FString ResString = FString::Printf(TEXT("%dx%d"), CurrentRes.X, CurrentRes.Y);
		Combo_Resolution->SetSelectedOption(ResString);
	}

	// 初始化窗口模式
	if (Combo_WindowMode)
	{
		Combo_WindowMode->ClearOptions();
		Combo_WindowMode->AddOption(TEXT("全屏"));
		Combo_WindowMode->AddOption(TEXT("窗口"));

		EWindowMode::Type Mode = UserSettings->GetFullscreenMode();
		bool bIsFullscreen = (Mode == EWindowMode::Fullscreen || Mode == EWindowMode::WindowedFullscreen);
		Combo_WindowMode->SetSelectedIndex(bIsFullscreen ? 0 : 1);
	}

	// 初始化画质等级
	if (Combo_Quality)
	{
		Combo_Quality->ClearOptions();
		Combo_Quality->AddOption(TEXT("低"));
		Combo_Quality->AddOption(TEXT("中"));
		Combo_Quality->AddOption(TEXT("高"));
		Combo_Quality->AddOption(TEXT("影视级"));

		int32 QualityLevel = UserSettings->GetOverallScalabilityLevel();
		Combo_Quality->SetSelectedIndex(FMath::Clamp(QualityLevel, 0, 3));
	}

	// 初始化垂直同步
	if (Check_VSync)
	{
		Check_VSync->SetIsChecked(UserSettings->IsVSyncEnabled());
	}

	// 激活音频系统
	if (GlobalSoundMix)
	{
		UGameplayStatics::PushSoundMixModifier(this, GlobalSoundMix);
	}
}

// ------------------------------------------------------------------------
// 菜单状态管理
// ------------------------------------------------------------------------
void UBMWPauseMenu::TogglePauseMenu()
{
	if (IsInViewport())
	{
		// 菜单开启 -> 关闭
		RemoveFromParent();
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		SetupInputMode(false); // 恢复游戏控制
	}
	else
	{
		// 菜单关闭 -> 开启
		AddToViewport(999); // 确保 UI 覆盖在最上层
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		SetupInputMode(true); // 释放鼠标控制

		// 每次打开时重新同步设置，防止外部修改导致的不一致
		InitializeSettings();
		if (QuitConfirmPopup) QuitConfirmPopup->SetVisibility(ESlateVisibility::Hidden);
	}
}

// ------------------------------------------------------------------------
// 导航事件实现
// ------------------------------------------------------------------------
void UBMWPauseMenu::OnTabChanged_Cultivation() { if (MainContentSwitcher) MainContentSwitcher->SetActiveWidgetIndex(0); }
void UBMWPauseMenu::OnTabChanged_Relics() { if (MainContentSwitcher) MainContentSwitcher->SetActiveWidgetIndex(1); }
void UBMWPauseMenu::OnTabChanged_Equipment() { if (MainContentSwitcher) MainContentSwitcher->SetActiveWidgetIndex(2); }
void UBMWPauseMenu::OnTabChanged_Inventory() { if (MainContentSwitcher) MainContentSwitcher->SetActiveWidgetIndex(3); }
void UBMWPauseMenu::OnTabChanged_Journal() { if (MainContentSwitcher) MainContentSwitcher->SetActiveWidgetIndex(4); }
void UBMWPauseMenu::OnTabChanged_Settings() { if (MainContentSwitcher) MainContentSwitcher->SetActiveWidgetIndex(5); }

void UBMWPauseMenu::OnSubMenu_Video() { if (SettingsOptionSwitcher) SettingsOptionSwitcher->SetActiveWidgetIndex(0); }
void UBMWPauseMenu::OnSubMenu_Audio() { if (SettingsOptionSwitcher) SettingsOptionSwitcher->SetActiveWidgetIndex(1); }
void UBMWPauseMenu::OnSubMenu_Resume() { TogglePauseMenu(); }
void UBMWPauseMenu::OnSubMenu_Quit() { if (QuitConfirmPopup) QuitConfirmPopup->SetVisibility(ESlateVisibility::Visible); }

void UBMWPauseMenu::OnRealQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}
void UBMWPauseMenu::OnCancelQuit()
{
	if (QuitConfirmPopup) QuitConfirmPopup->SetVisibility(ESlateVisibility::Hidden);
}

// ------------------------------------------------------------------------
// 画质设置实现
// ------------------------------------------------------------------------
void UBMWPauseMenu::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	FString Left, Right;
	if (SelectedItem.Split(TEXT("x"), &Left, &Right))
	{
		int32 Width = FCString::Atoi(*Left);
		int32 Height = FCString::Atoi(*Right);

		if (Width > 0 && Height > 0)
		{
			UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
			Settings->SetScreenResolution(FIntPoint(Width, Height));
			ApplyVideoSettings();
		}
	}
}

void UBMWPauseMenu::OnWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	// 根据选中文本切换全屏/窗口模式 
	EWindowMode::Type TargetMode = (SelectedItem == TEXT("全屏")) ? EWindowMode::Fullscreen : EWindowMode::Windowed;
	Settings->SetFullscreenMode(TargetMode);
	ApplyVideoSettings();
}

void UBMWPauseMenu::OnQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	int32 Level = 2; // 默认为高

	if (SelectedItem == TEXT("低")) Level = 0;
	else if (SelectedItem == TEXT("中")) Level = 1;
	else if (SelectedItem == TEXT("高")) Level = 2;
	else if (SelectedItem == TEXT("影视级")) Level = 3;

	// 设置整体缩放等级
	Settings->SetOverallScalabilityLevel(Level);
	ApplyVideoSettings();
}

void UBMWPauseMenu::OnVSyncChanged(bool bIsChecked)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	Settings->SetVSyncEnabled(bIsChecked);
	ApplyVideoSettings();
}

void UBMWPauseMenu::ApplyVideoSettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	// 应用设置并保存至磁盘 (bCheckForCommandLineOverrides=false)
	Settings->ApplySettings(false);
}

// ------------------------------------------------------------------------
// 音频设置实现
// ------------------------------------------------------------------------
void UBMWPauseMenu::SetSoundClassVolume(USoundClass* TargetClass, float Volume)
{
	// 防止因 SoundMix 或 SoundClass 未配置导致的空指针崩溃
	if (GlobalSoundMix && TargetClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), GlobalSoundMix, TargetClass, Volume, 1.0f, 0.0f);
	}
}

void UBMWPauseMenu::OnMasterVolumeChanged(float Value)
{
	SetSoundClassVolume(SC_Master, Value);
}

void UBMWPauseMenu::OnMusicVolumeChanged(float Value)
{
	SetSoundClassVolume(SC_Music, Value);
}

void UBMWPauseMenu::OnSFXVolumeChanged(float Value)
{
	SetSoundClassVolume(SC_SFX, Value);
}

// ------------------------------------------------------------------------
// 输入模式控制
// ------------------------------------------------------------------------
void UBMWPauseMenu::SetupInputMode(bool bIsMenuOpen)
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (bIsMenuOpen)
		{
			// 菜单模式
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(this->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
		else
		{
			// 游戏模式
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = false;
		}
	}
}

FReply UBMWPauseMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Tab)
	{
		// 执行关闭逻辑
		TogglePauseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}