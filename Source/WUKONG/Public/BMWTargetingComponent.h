#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BMWTargetingComponent.generated.h"

class UCameraComponent;
class UBMWMainHUD;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WUKONG_API UBMWTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBMWTargetingComponent();

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void UpdateTargetingLogic(UCameraComponent* Camera, UBMWMainHUD* HUD);

protected:
	// 内部记录当前锁定的 Boss
	UPROPERTY()
	AActor* CurrentTargetBoss;

	// 扫描参数
	float ScanDistance = 4000.0f;
};