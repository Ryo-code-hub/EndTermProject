#include "ANS_RootMotionFly.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h" 
#include "BMWPlayerCharacter.h"

void UANS_RootMotionFly::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	// 必须调用父类
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
		{
			// 切换为 Flying 模式
			// 允许 Z 轴位移生效，不再吸附地面
			Character->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		}
	}

	// 开启镜头跟随
	if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->SetCameraFollowPelvis(true);
	}
}

void UANS_RootMotionFly::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// 必须调用父类
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
		{
			// 切换为 Falling 模式
			// 动作结束，让重力接管。如果脚在地上，系统会自动切回 Walking
			Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}

	// 关闭镜头跟随
	if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(MeshComp->GetOwner()))
	{
		Player->SetCameraFollowPelvis(false);
	}
}