#include "BMWMinionCharacter.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "BMWAIMinionController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BMWPlayerCharacter.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BMWAttributeComponent.h"
#include "BrainComponent.h"
#include "Kismet/GameplayStatics.h"

ABMWMinionCharacter::ABMWMinionCharacter()
{
	AIControllerClass = ABMWAIMinionController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bHasAggro = false;
	bIsDead = false;
	bIsDodging = false;
	CurrentAttackDamage = 15.0f;

	// 调整胶囊体大小
	GetCapsuleComponent()->InitCapsuleSize(30.f, 80.0f);

	// 防止小怪贴脸攻击时，弹簧臂检测到小怪身体而拉近镜头
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	// ------------------------------------------

	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidget->SetRelativeLocation(FVector(0, 0, 120));

	AggroSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphereComp"));
	AggroSphereComp->SetupAttachment(RootComponent);
	AggroSphereComp->SetSphereRadius(600.0f);
	AggroSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AggroSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	AggroSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ABMWMinionCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (AggroSphereComp)
	{
		AggroSphereComp->OnComponentBeginOverlap.AddDynamic(this, &ABMWMinionCharacter::OnAggroSphereOverlap);
	}

	if (AttributeComp)
	{
		AttributeComp->OnHealthChanged.AddDynamic(this, &ABMWMinionCharacter::OnHealthChanged);
	}
}

// 拦截伤害，判断闪避
float ABMWMinionCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 如果已经死了，不做任何处理
	if (bIsDead) return 0.0f;

	// 正在闪避中 -> 闪避被打断，强制吃伤害
	if (bIsDodging)
	{
		// 重置闪避状态
		bIsDodging = false;

		// 停止当前的闪避动作
		StopAnimMontage(DodgeMontage);

		// 调试
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, TEXT("Dodge Interrupted! Hit!"));

		// 调用父类正常扣血
		return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}

	// 正常状态 -> 闪避
	// 随机数 0.0~1.0，如果小于 DodgeChance(0.3)，则闪避成功
	if (DodgeMontage && FMath::FRand() <= DodgeChance)
	{
		// 闪避成功逻辑

		bIsDodging = true;

		// 播放闪避动画
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			float Duration = PlayAnimMontage(DodgeMontage);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ABMWMinionCharacter::OnDodgeFinished);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, DodgeMontage);
		}

		// 调试
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, TEXT("Dodged!"));

		// 返回 0 伤害
		return 0.0f;
	}

	// 闪避失败 -> 正常吃伤害
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

// 闪避结束回调
void ABMWMinionCharacter::OnDodgeFinished(UAnimMontage* Montage, bool bInterrupted)
{
	// 闪避动画结束 重置状态
	if (Montage == DodgeMontage)
	{
		bIsDodging = false;
	}
}

void ABMWMinionCharacter::OnHealthChanged(AActor* InstigatorActor, UBMWAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (bIsDead) return;

	if (Delta < 0.0f)
	{
		// 触发仇恨
		if (!bHasAggro && InstigatorActor)
		{
			bHasAggro = true;
			if (ABMWAIMinionController* AICon = Cast<ABMWAIMinionController>(GetController()))
			{
				AICon->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), InstigatorActor);
				AICon->GetBlackboardComponent()->SetValueAsBool(TEXT("HasSeenPlayer"), true);
			}
		}

		// 死亡逻辑
		if (NewHealth <= 0.0f)
		{
			Die();
		}
		// 受击逻辑
		else
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance && HitMontage)
			{
				if (AnimInstance->Montage_IsPlaying(HitMontage)) return;

				if (AAIController* AICon = Cast<AAIController>(GetController()))
				{
					AICon->StopMovement();
				}
				PlayAnimMontage(HitMontage);
			}
		}
	}
}

void ABMWMinionCharacter::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	if (HealthBarWidget) HealthBarWidget->SetVisibility(false);

	if (ABMWAIMinionController* AICon = Cast<ABMWAIMinionController>(GetController()))
	{
		AICon->ClearFocus(EAIFocusPriority::Gameplay);
		AICon->StopMovement();
		if (AICon->GetBrainComponent())
		{
			AICon->GetBrainComponent()->StopLogic("Dead");
		}
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	SetLifeSpan(5.0f);
}

void ABMWMinionCharacter::PlayAttack()
{
	if (bIsDead) return;

	// 如果正在闪避 不准攻击
	if (bIsDodging) return;

	if (AttackMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && HitMontage && AnimInstance->Montage_IsPlaying(HitMontage))
		{
			return;
		}

		if (AnimInstance && !AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			PlayAnimMontage(AttackMontage);
			if (AAIController* AICon = Cast<AAIController>(GetController()))
			{
				AICon->StopMovement();
			}
		}
	}
}

void ABMWMinionCharacter::OnAggroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsDead) return;
	if (bHasAggro) return;

	if (OtherActor && OtherActor != this && OtherActor->IsA<ABMWPlayerCharacter>())
	{
		bHasAggro = true;
		ABMWAIMinionController* AICon = Cast<ABMWAIMinionController>(GetController());
		if (AICon && AICon->GetBlackboardComponent())
		{
			AICon->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), OtherActor);
			AICon->GetBlackboardComponent()->SetValueAsBool(TEXT("HasSeenPlayer"), true);
			AICon->SetFocus(OtherActor);
		}
	}
}

void ABMWMinionCharacter::ProcessHit(FHitResult HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	// 排除自己，排除已打过的
	if (!HitActor || HitActor == this || HitActors.Contains(HitActor)) return;

	HitActors.Add(HitActor);

	// 造成伤害 (数值在构造函数里设为 15.0f)
	UGameplayStatics::ApplyDamage(HitActor, CurrentAttackDamage, GetController(), this, UDamageType::StaticClass());

	// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Minion Hit!"));
}