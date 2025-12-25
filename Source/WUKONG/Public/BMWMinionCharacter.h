// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BMWBaseCharacter.h"
#include "BMWMinionCharacter.generated.h"

class UWidgetComponent;

UCLASS()
class WUKONG_API ABMWMinionCharacter : public ABMWBaseCharacter
{
	GENERATED_BODY()
	
public:
	ABMWMinionCharacter();

protected:
	// --- 小怪特有的组件 ---
	// 这是一个 3D UI 组件，用来显示头顶的红条
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HealthBarWidget;

	// 重写死亡逻辑（小怪死了可能要给玩家加经验，或者消失得更快）
	virtual void Die() override;
};
