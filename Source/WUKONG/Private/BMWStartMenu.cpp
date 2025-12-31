#include "BMWStartMenu.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MediaPlayer.h"
#include "BMWPlayerCharacter.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"

void UBMWStartMenu::ShowMenu(APlayerController* PC, TSubclassOf<UBMWStartMenu> MenuClass)
{
	if (!PC || !MenuClass) return;

	// 创建 UI
	UBMWStartMenu* Menu = CreateWidget<UBMWStartMenu>(PC, MenuClass);
	if (Menu)
	{
		Menu->AddToViewport(9999); // 确保在最上层

		// 马上接管输入
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(Menu->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true; // 显示鼠标
	}
}

void UBMWStartMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// 不要用 bIsFocusable = true;
	SetIsFocusable(true);

	// 绑定主菜单原有按钮
	if (StartGameBtn) StartGameBtn->OnClicked.AddDynamic(this, &UBMWStartMenu::OnStartClicked);
	if (QuitBtn) QuitBtn->OnClicked.AddDynamic(this, &UBMWStartMenu::OnQuitClicked);

	// 绑定主菜单的设置按钮
	if (SettingsBtn) SettingsBtn->OnClicked.AddDynamic(this, &UBMWStartMenu::OnSettingsBtnClicked);

	// 绑定设置界面新按钮
	if (BackSettingsBtn) BackSettingsBtn->OnClicked.AddDynamic(this, &UBMWStartMenu::OnBackSettingsClicked);
	if (ApplySettingsBtn) ApplySettingsBtn->OnClicked.AddDynamic(this, &UBMWStartMenu::OnApplyClicked);

	// 绑定设置控件 (滑条/下拉框)
	if (VolumeSlider) VolumeSlider->OnValueChanged.AddDynamic(this, &UBMWStartMenu::OnVolumeChanged);
	if (ResolutionCombo) ResolutionCombo->OnSelectionChanged.AddDynamic(this, &UBMWStartMenu::OnResolutionChanged);

	// 初始化设置数据
	InitSettingsValues();

	// 初始显示第0页 (主菜单)
	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(0);
}

void UBMWStartMenu::OnStartClicked()
{
	if (MenuSwitcher)
	{
		MenuSwitcher->SetActiveWidgetIndex(1); // 视频页
		if (MediaPlayer && IntroVideoSource)
		{
			MediaPlayer->OpenSource(IntroVideoSource);
			MediaPlayer->Play();
		}
		SetKeyboardFocus(); // 确保键盘能响应后续按键
	}
}

void UBMWStartMenu::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}

FReply UBMWStartMenu::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// 视频页按任意键进游戏
	if (MenuSwitcher && MenuSwitcher->GetActiveWidgetIndex() == 1)
	{
		EnterGameLevel();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UBMWStartMenu::EnterGameLevel()
{
	if (MediaPlayer) MediaPlayer->Close();

	if (APlayerController* PC = GetOwningPlayer())
	{

		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false; // 隐藏鼠标

		if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(PC->GetPawn()))
		{
			Player->StartGame();
		}
	}

	RemoveFromParent();
}

void UBMWStartMenu::OnSettingsBtnClicked()
{
	if (MenuSwitcher)
	{
		MenuSwitcher->SetActiveWidgetIndex(2);
	}
}

void UBMWStartMenu::OnBackSettingsClicked()
{
	if (MenuSwitcher)
	{
		// 切换回 Index 0
		MenuSwitcher->SetActiveWidgetIndex(0);
	}
}

void UBMWStartMenu::OnVolumeChanged(float Value)
{
	// 只负责更新显示的文字
	if (VolumeValueText)
	{
		VolumeValueText->SetText(FText::AsNumber(FMath::RoundToInt(Value * 100)));
	}
}

void UBMWStartMenu::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{

}

void UBMWStartMenu::OnApplyClicked()
{
	// 应用分辨率
	UGameUserSettings* Settings = GEngine->GetGameUserSettings();
	if (Settings && ResolutionCombo)
	{
		// 直接读取下拉框当前选中的文字
		FString SelectedRes = ResolutionCombo->GetSelectedOption();

		if (SelectedRes == "1920x1080") Settings->SetScreenResolution(FIntPoint(1920, 1080));
		else if (SelectedRes == "1280x720") Settings->SetScreenResolution(FIntPoint(1280, 720));
		else if (SelectedRes == "2560x1440") Settings->SetScreenResolution(FIntPoint(2560, 1440));

		// 应用画质设置
		Settings->ApplySettings(false);
	}

	// 应用音量 (最简单的命令)
	if (VolumeSlider)
	{
		// 获取滑条当前的值
		float CurrentVol = VolumeSlider->GetValue();

		FString Command = FString::Printf(TEXT("au.MasterVolume %f"), CurrentVol);

		UKismetSystemLibrary::ExecuteConsoleCommand(this, Command);
	}

	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("设置应用成功!"));
}

void UBMWStartMenu::InitSettingsValues()
{
	// 初始化分辨率下拉框
	if (ResolutionCombo)
	{
		ResolutionCombo->ClearOptions();
		ResolutionCombo->AddOption(TEXT("1920x1080"));
		ResolutionCombo->AddOption(TEXT("1280x720"));
		ResolutionCombo->AddOption(TEXT("2560x1440"));
		ResolutionCombo->SetSelectedOption(TEXT("1920x1080")); // 默认值
	}

	// 初始化音量条
	if (VolumeSlider)
	{
		VolumeSlider->SetValue(1.0f);
		if (VolumeValueText) VolumeValueText->SetText(FText::AsNumber(100));
	}
}