#include "BMWBaseCharacter.h"
#include "BMWAttributeComponent.h"
#include "Components/CapsuleComponent.h"

void ABMWBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 记下出生位置
    InitialTransform = GetActorTransform();

    if (AttributeComp)
    {
        // 绑定血量变化事件
        AttributeComp->OnHealthChanged.AddDynamic(this, &ABMWBaseCharacter::OnHealthChanged);
    }
}

// Sets default values
ABMWBaseCharacter::ABMWBaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 创建属性组件
    AttributeComp = CreateDefaultSubobject<UBMWAttributeComponent>(TEXT("AttributeComp"));

    // 初始化状态
    bIsDead = false;
}

float ABMWBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (bIsDead || !AttributeComp)
    {
        return 0.0f;
    }

    AttributeComp->ApplyHealthChange(DamageCauser, -ActualDamage);

    return ActualDamage;
}

void ABMWBaseCharacter::OnHealthChanged(AActor* InstigatorActor, UBMWAttributeComponent* OwningComp, float NewHealth, float Delta)
{
    if (Delta >= 0.0f || bIsDead)
    {
        return;
    }

    // 检查是否死亡
    if (NewHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        // 没死，播受击动画
        PlayHitReact(InstigatorActor);
    }
}

void ABMWBaseCharacter::PlayHitReact(AActor* InstigatorActor)
{
    if (!HitReactMontage) return;

    // 获取方向名 (Front, Back, Left, Right)
    FName SectionToPlay = GetHitDirection(InstigatorActor);

    // 播放蒙太奇的指定 Section
    PlayAnimMontage(HitReactMontage, 1.0f, SectionToPlay);
}

FName ABMWBaseCharacter::GetHitDirection(AActor* Attacker)
{
    if (!Attacker) return FName("Front");

    FVector ImpactDir = (Attacker->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    ImpactDir.Z = 0.0f;

    // 获取朝向
    FVector ForwardDir = GetActorForwardVector();
    FVector RightDir = GetActorRightVector();

    // 计算点乘
    // ForwardDot > 0 (前方), < 0 (后方)
    float ForwardDot = FVector::DotProduct(ForwardDir, ImpactDir);
    // RightDot > 0 (右方), < 0 (左方)
    float RightDot = FVector::DotProduct(RightDir, ImpactDir);

    // 判断逻辑 (45度角划分)
    // 0.707 是 cos(45度)
    if (ForwardDot >= 0.707f)
    {
        return FName("Front");
    }
    else if (ForwardDot <= -0.707f)
    {
        return FName("Back");
    }
    else
    {
        // 侧面
        if (RightDot > 0.0f)
        {
            return FName("Right"); // 右边被打 -> 向左歪
        }
        else
        {
            return FName("Left"); // 左边被打 -> 向右歪
        }
    }
}


// 重置函数
void ABMWBaseCharacter::ResetCharacter()
{

    // 清除隐藏尸体的定时器
    GetWorldTimerManager().ClearTimer(TimerHandle_HideCorpse);

    // 强制显示角色
    SetActorHiddenInGame(false);

    // 恢复生命值
    if (AttributeComp)
    {
        float HealAmount = AttributeComp->GetMaxHealth() - AttributeComp->GetHealth();
        AttributeComp->ApplyHealthChange(this, HealAmount);
    }

    // 复活标记
    bIsDead = false;

    // 恢复碰撞和物理
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh")); // 恢复默认Profile

  // 播放重生动画或重置蒙太奇
    StopAnimMontage(nullptr);

    // 恢复位置
    SetActorTransform(InitialTransform);
}

void ABMWBaseCharacter::Die()
{
    if (bIsDead) return;
    bIsDead = true;

    // 停止移动
    AController* MyController = GetController();
    if (MyController) MyController->StopMovement();

    // 播放动画或物理
    if (DeathMontage) PlayAnimMontage(DeathMontage);
    else {
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    }

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 隐藏逻辑
    if (bSystemHandleDeath)
    {
        GetWorldTimerManager().SetTimer(TimerHandle_HideCorpse, [this]()
            {
                this->SetActorHiddenInGame(true);
            }, 5.0f, false);
    }
    else
    {
        SetLifeSpan(5.0f);
    }
}