#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MediaSource.h"
#include "BMWStartMenu.generated.h"

class UMediaPlayer;

UCLASS()
class WUKONG_API UBMWStartMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BMW UI")
	static void ShowMenu(APlayerController* PC, TSubclassOf<UBMWStartMenu> MenuClass);

	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(EditAnywhere, Category = "Video Config")
	UMediaPlayer* MediaPlayer;

	UPROPERTY(EditAnywhere, Category = "Video Config")
	UMediaSource* IntroVideoSource;

protected:

	// 设置界面的控件

	// 音量滑条
	UPROPERTY(meta = (BindWidget)) class USlider* VolumeSlider;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* VolumeValueText; // 显示数字

	// 分辨率下拉框
	UPROPERTY(meta = (BindWidget)) class UComboBoxString* ResolutionCombo;

	// 返回按钮
	UPROPERTY(meta = (BindWidget)) class UButton* BackSettingsBtn;

	// 应用按钮
	UPROPERTY(meta = (BindWidget)) class UButton* ApplySettingsBtn;

	// 功能函数
	UFUNCTION() void OnSettingsBtnClicked(); // 主界面点“设置”
	UFUNCTION() void OnBackSettingsClicked(); // 设置里点“返回”

	UFUNCTION() void OnVolumeChanged(float Value); // 滑动条拖动
	UFUNCTION() void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType); // 分辨率改变
	UFUNCTION() void OnApplyClicked(); // 点应用

	// 初始化设置
	void InitSettingsValues();

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UButton* StartGameBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* SettingsBtn;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitBtn;

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnQuitClicked();

private:
	void EnterGameLevel();
};