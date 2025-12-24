// Fill out your copyright notice in the Description page of Project Settings.


#include "BMWBaseCharacter.h"
#include "BMWAttributeComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values
ABMWBaseCharacter::ABMWBaseCharacter()
{
    // 开启 Tick，如果不需要可以设为 false 优化性能
    PrimaryActorTick.bCanEverTick = true;

    // 1. 创建属性组件
    // "AttributeComp" 是这个组件在编辑器里的名字
    AttributeComp = CreateDefaultSubobject<UBMWAttributeComponent>(TEXT("AttributeComp"));

    // 初始化状态
    bIsDead = false;
}

float ABMWBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // 1. 调用父类逻辑（一定要保留）
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 2. 如果已经死了，或者没有属性组件，直接返回
    if (bIsDead || !AttributeComp)
    {
        return 0.0f;
    }

    // 3. 核心：调用属性组件来扣血
    // 注意：ApplyHealthChange 这个函数需要在 AttributeComponent 里实现（传入负数代表扣血）
    // 这里的 EventInstigator 是造成伤害的人（比如悟空打怪，Instigator就是悟空的控制器）
    if (AttributeComp->ApplyHealthChange(DamageCauser, -ActualDamage))
    {
        // 4. 如果扣血成功，检查是否死亡
        // 假设你在组件里写了一个 GetHealth() 函数
        if (AttributeComp->GetHealth() <= 0.0f)
        {
            Die();
        }
    }

    return ActualDamage;
}

void ABMWBaseCharacter::Die()
{
    // 防止重复死亡
    if (bIsDead) return;

    bIsDead = true;

    // --- 死亡后的通用处理 ---

    // 1. 停止控制（除了旋转，禁止移动）
    AController* MyController = GetController();
    if (MyController)
    {
        MyController->StopMovement();
        // 如果是怪，通常会在这里 Detach AI Controller
    }

    // 2. 物理表现：开启布娃娃系统 (Ragdoll)
    // 获取角色的网格体，开启物理模拟
    GetMesh()->SetSimulatePhysics(true);
    // 这里的 CollisionProfileName 需要根据你的项目设置，通常是 "Ragdoll"
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

    // 3. 关闭胶囊体碰撞（避免尸体挡路）
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 4. 设置生命周期（比如 5 秒后尸体消失）
    SetLifeSpan(5.0f);
}