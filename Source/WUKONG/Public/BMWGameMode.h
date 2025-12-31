#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BMWGameMode.generated.h"

UCLASS()
class WUKONG_API ABMWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	// 玩家死亡时调用此函数
	void OnPlayerDied(AController* Controller);

protected:
	// 复活倒计时
	UFUNCTION()
	void RespawnTimerElapsed(AController* Controller);
};