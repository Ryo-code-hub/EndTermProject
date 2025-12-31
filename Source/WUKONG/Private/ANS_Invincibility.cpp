#include "ANS_Invincibility.h"
#include "BMWPlayerCharacter.h" 

void UANS_Invincibility::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(MeshComp->GetOwner()))
		{
			// 开启无敌
			Player->SetInvincible(true);
		}
	}
}

void UANS_Invincibility::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(MeshComp->GetOwner()))
		{
			// 关闭无敌
			Player->SetInvincible(false);
		}
	}
}