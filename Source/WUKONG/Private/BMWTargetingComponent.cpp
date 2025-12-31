#include "BMWTargetingComponent.h"
#include "Camera/CameraComponent.h"
#include "BMWMainHUD.h"
#include "BMWBaseCharacter.h"

UBMWTargetingComponent::UBMWTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBMWTargetingComponent::UpdateTargetingLogic(UCameraComponent* Camera, UBMWMainHUD* HUD)
{
	if (!Camera || !HUD) return;

	// 射线检测逻辑
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + (Camera->GetForwardVector() * ScanDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	// 状态机逻辑
	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->IsA(ABMWBaseCharacter::StaticClass()))
		{
			// 只有当目标改变时才调用 UI
			if (CurrentTargetBoss != HitActor)
			{
				CurrentTargetBoss = HitActor;
				HUD->ShowBossHealth(HitActor);
			}
			return;
		}
	}

	// 丢失目标逻辑
	if (CurrentTargetBoss != nullptr)
	{
		HUD->HideBossHealth();
		CurrentTargetBoss = nullptr;
	}
}


