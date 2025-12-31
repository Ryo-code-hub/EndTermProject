#include "ANS_WeaponTrace.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "BMWPlayerCharacter.h"
#include "BMWMinionCharacter.h"
#include "BMWBaseCharacter.h"
#include "MyBMWBossCharacter.h"

void UANS_WeaponTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);


	if (MeshComp && MeshComp->GetOwner())
	{
		AActor* Owner = MeshComp->GetOwner();

		if (ABMWPlayerCharacter* P = Cast<ABMWPlayerCharacter>(Owner))      P->StartWeaponCollision();
		else if (ABMWMinionCharacter* M = Cast<ABMWMinionCharacter>(Owner)) M->StartWeaponCollision();
		else if (AMyBMWBossCharacter* B = Cast<AMyBMWBossCharacter>(Owner)) B->StartWeaponCollision();
	}
}

void UANS_WeaponTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp) return;
	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	FVector StartLoc = MeshComp->GetSocketLocation(SocketStart);
	FVector EndLoc = MeshComp->GetSocketLocation(SocketEnd);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerActor);

	TArray<FHitResult> HitResults;

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		MeshComp,
		StartLoc,
		EndLoc,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		HitResults,
		true
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();

			if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(OwnerActor))
			{
				Player->ProcessHit(Hit);
			}
			// 如果是小怪
			else if (ABMWMinionCharacter* Minion = Cast<ABMWMinionCharacter>(OwnerActor))
			{
				Minion->ProcessHit(Hit);
			}
			// 如果是 Boss
			else if (AMyBMWBossCharacter* Boss = Cast<AMyBMWBossCharacter>(OwnerActor))
			{
				Boss->ProcessHit(Hit);
			}

			// 基础过滤
			if (HitActor && !HitActors.Contains(HitActor))
			{
				// 严谨的阵营伤害过滤

				bool bIsPlayerAttacking = OwnerActor->IsA<ABMWPlayerCharacter>();
				bool bTargetIsPlayer = HitActor->IsA<ABMWPlayerCharacter>();

				bool bIsMinionAttacking = OwnerActor->IsA<ABMWMinionCharacter>();
				bool bTargetIsMinion = HitActor->IsA<ABMWMinionCharacter>(); // 鹿、僵尸、哥布林都是 BMWMinionCharacter 的子类

				// 玩家不打玩家
				if (bIsPlayerAttacking && bTargetIsPlayer) continue;

				// 小怪不打小怪
				if (bIsMinionAttacking && bTargetIsMinion) continue;

				// 伤害生效
				HitActors.Add(HitActor);

				UGameplayStatics::ApplyDamage(
					HitActor,
					DamageAmount,
					OwnerActor->GetInstigatorController(),
					OwnerActor,
					UDamageType::StaticClass()
				);

				if (ABMWBaseCharacter* Char = Cast<ABMWBaseCharacter>(OwnerActor))
				{
					if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(OwnerActor))
					{
						Player->ProcessHit(Hit);
					}
					else if (ABMWMinionCharacter* Minion = Cast<ABMWMinionCharacter>(OwnerActor))
					{
						Minion->ProcessHit(Hit); // 需要在小怪里实现这个函数
					}
				}

				if (GEngine)
				{
					FString AttackerName = OwnerActor->GetName();
					//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("%s Hit: %s"), *AttackerName, *HitActor->GetName()));
				}
			}
		}
	}
}

void UANS_WeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);


	if (MeshComp && MeshComp->GetOwner())
	{
		AActor* Owner = MeshComp->GetOwner();

		if (ABMWPlayerCharacter* P = Cast<ABMWPlayerCharacter>(Owner))      P->EndWeaponCollision();
		else if (ABMWMinionCharacter* M = Cast<ABMWMinionCharacter>(Owner)) M->EndWeaponCollision();
		else if (AMyBMWBossCharacter* B = Cast<AMyBMWBossCharacter>(Owner)) B->EndWeaponCollision();
	}
}