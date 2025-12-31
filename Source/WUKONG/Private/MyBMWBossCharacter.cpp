#include "MyBMWBossCharacter.h"
#include "BMWAttributeComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyBMWBossAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BrainComponent.h"
#include <BMWPlayerCharacter.h>

AMyBMWBossCharacter::AMyBMWBossCharacter()
{
	// 初始化组件
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);

	// 设置 Boss 体型
	GetCapsuleComponent()->InitCapsuleSize(45.f, 100.0f);

	// 初始化变量
	bIsPhaseTwo = false;
	CurrentAttackDamage = 20.0f;

	// 指定 AI Controller
	AIControllerClass = AMyBMWBossAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMyBMWBossCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 绑定血量变化事件
	if (AttributeComp)
	{
		AttributeComp->OnHealthChanged.AddDynamic(this, &AMyBMWBossCharacter::OnHealthChanged);
	}
}

void AMyBMWBossCharacter::OnHealthChanged(AActor* InstigatorActor, UBMWAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	/* FString Msg = FString::Printf(TEXT("当前血量: %f / %f (比例: %f)"), NewHealth, AttributeComp->MaxHealth, NewHealth / AttributeComp->MaxHealth);
	 GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, Msg);*/
	 // 死亡判定
	if (NewHealth <= 0.0f)
	{
		Die();
		return;
	}

	// 转阶段判定
	if (!bIsPhaseTwo && AttributeComp->MaxHealth > 0)
	{
		float HealthPercent = NewHealth / AttributeComp->MaxHealth;

		if (HealthPercent <= PhaseTwoThreshold)
		{
			EnterPhaseTwo();
		}
	}

	if (Delta < 0.0f && HitMontage)
	{
		// 如果正在放技能， Boss 是霸体，不打断
		// GetMesh()->GetAnimInstance()->Montage_IsPlaying(nullptr);

		if (!bIsPhaseTwo)
		{
			PlayAnimMontage(HitMontage);
		}
	}
}
void AMyBMWBossCharacter::EnterPhaseTwo()
{
	// 防止重复触发
	if (bIsPhaseTwo)
		return;

	// 设置标志位
	bIsPhaseTwo = true;

	// 播放变身动画，并获取动画长度
	float Duration = 0.0f;
	Duration = PlayAnimMontage(PhaseTransitionMontage);
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->ClearFocus(EAIFocusPriority::Gameplay);
	}


	// 冻结玩家
	// 获取玩家控制器
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->SetIgnoreMoveInput(true); // 禁止移动
		//PC->SetIgnoreLookInput(true); // 禁止转头

	}
	// 设置定时器
	float WaitTime = (Duration > 0.0f) ? Duration : 0.1f;

	GetWorldTimerManager().SetTimer(TimerHandle_ResumeFight, this, &AMyBMWBossCharacter::ResumeFight, WaitTime, false);

}

void AMyBMWBossCharacter::Die()
{
	if (GetCapsuleComponent()->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		return;

	// 停止 AI 逻辑
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		// 停止脑子
		if (AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->StopLogic("Dead");
		}

		// 停止身体移动
		AICon->StopMovement();

		// 清除注视焦点
		AICon->ClearFocus(EAIFocusPriority::Gameplay);

		DetachFromControllerPendingDestroy();
	}

	// 物理表现
	if (DeathMontage)
	{
		// 播放蒙太奇
		PlayAnimMontage(DeathMontage);

		// 确保网格体还可见，并且不做物理模拟
		GetMesh()->SetSimulatePhysics(false);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(15.0f);
}

void AMyBMWBossCharacter::ResumeFight()
{
	// 获取玩家控制器
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		// 解除冻结
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);
	}
	// UE_LOG(LogTemp, Warning, TEXT("Boss Phase 2 Fight Start!"));
}

void AMyBMWBossCharacter::ProcessHit(FHitResult HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor || HitActor == this || HitActors.Contains(HitActor)) return;

	HitActors.Add(HitActor);

	// 造成伤害 (数值在构造函数里设为 20.0f)
	UGameplayStatics::ApplyDamage(HitActor, CurrentAttackDamage, GetController(), this, UDamageType::StaticClass());
}