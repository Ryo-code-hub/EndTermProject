#include "AN_Footstep.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/PhysicsSettings.h"

void UAN_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetWorld()) return;

	// 确定射线检测的起点和终点
	FVector Start = MeshComp->GetSocketLocation(SocketName);
	FVector End = Start + FVector(0, 0, -150.0f); // 向下探测 50cm

	// 射线参数
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true; // 必须要求返回物理材质
	QueryParams.AddIgnoredActor(MeshComp->GetOwner()); // 忽略自己

	FHitResult HitResult;

	// 发射射线
	bool bHit = MeshComp->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	// 判断材质并播放声音
	if (bHit)
	{
		USoundBase* SoundToPlay = Sound_Default; // 默认声音

		// 播放声音
		if (SoundToPlay)
		{
			UGameplayStatics::PlaySoundAtLocation(MeshComp->GetWorld(), SoundToPlay, HitResult.Location, VolumeMultiplier);
		}
	}
}