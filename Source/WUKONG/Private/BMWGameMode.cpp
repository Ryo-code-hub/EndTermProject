#include "BMWGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "BMWBaseCharacter.h"
#include "BMWPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"

void ABMWGameMode::OnPlayerDied(AController* Controller)
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		// 使用 PC 指针来调用
		PC->SetCinematicMode(true, false, false);

		// 开启 5 秒倒计时
		FTimerHandle TimerHandle_Respawn;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnTimerElapsed", Controller);

		GetWorldTimerManager().SetTimer(TimerHandle_Respawn, Delegate, 5.0f, false);

		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("You Died! Respawning in 5s..."));
	}
}

void ABMWGameMode::RespawnTimerElapsed(AController* Controller)
{
	if (Controller)
	{
		// 复活玩家
		if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(Controller->GetPawn()))
		{
			Player->ResetCharacter();
		}

		// 重置所有小怪 (核心逻辑)
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABMWBaseCharacter::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			// 排除玩家自己，只重置 AI
			if (Actor != Controller->GetPawn())
			{
				if (ABMWBaseCharacter* Enemy = Cast<ABMWBaseCharacter>(Actor))
				{
					Enemy->ResetCharacter();
				}
			}
		}

		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("World Reset!"));
	}
}