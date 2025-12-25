#include "BMWMinionCharacter.h"
#include "Components/WidgetComponent.h"
#include "BMWAIMinionController.h"
#include "BMWAttributeComponent.h"

ABMWMinionCharacter::ABMWMinionCharacter()
{
	// 1. 设置 AI 控制器
	// 这行代码的意思是：只要这个怪生出来，默认就用你写的那个 AI 控制器来接管大脑
	AIControllerClass = ABMWAIMinionController::StaticClass();

	// 2. 极为重要：设置自动接管
	// 意思是：无论你是把怪拖进场景(Placed)，还是用刷怪笼生成(Spawned)，AI 都会立刻接管
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 3. 初始化头顶血条
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	// 默认设置为“屏幕空间”，这样血条会始终朝向摄像机
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	// 设置相对位置（大概在头顶）
	HealthBarWidget->SetRelativeLocation(FVector(0, 0, 180));
}

void ABMWMinionCharacter::Die()
{
	// 1. 先隐藏血条（死人不需要血条）
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}

	// 2. 执行父类的死亡逻辑（布娃娃、停止移动等）
	Super::Die();

	// 3. (可选) 给玩家加经验的逻辑可以写在这里
	// Cast<ABMWPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn())->AddExp(100);
}