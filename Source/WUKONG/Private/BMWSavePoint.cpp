#include "BMWSavePoint.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "BMWPlayerCharacter.h"

ABMWSavePoint::ABMWSavePoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// 创建模型组件
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	// 阻挡玩家
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));

	// 创建触发框
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(MeshComp);
	// 设只检测重叠 不阻挡
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	// 设置默认大小
	TriggerBox->SetBoxExtent(FVector(200.f, 200.f, 100.f));

	// 创建复活点箭头
	SpawnPointArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnPointArrow"));
	SpawnPointArrow->SetupAttachment(MeshComp);
	// 默认放在土地庙前方一点的位置
	SpawnPointArrow->SetRelativeLocation(FVector(200.f, 0.f, 50.f));

	// 绑定事件
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABMWSavePoint::OnOverlap);
}

// 实现重叠逻辑
void ABMWSavePoint::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 检查是不是玩家
	if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(OtherActor))
	{
		// 获取箭头组件的世界变换
		FTransform SpawnTrans = SpawnPointArrow->GetComponentTransform();

		// 更新玩家的重生点
		Player->SetRespawnTransform(SpawnTrans);

		// 调试
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Save Point Activated!"));
	}
}