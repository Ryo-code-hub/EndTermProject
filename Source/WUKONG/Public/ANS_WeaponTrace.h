#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_WeaponTrace.generated.h"

UCLASS()
class WUKONG_API UANS_WeaponTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// 武器上的插槽名字
	UPROPERTY(EditAnywhere, Category = "Trace")
	FName SocketStart = FName("TraceStart");

	UPROPERTY(EditAnywhere, Category = "Trace")
	FName SocketEnd = FName("TraceEnd");

	// 检测半径
	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceRadius = 15.0f;

	// 伤害数值
	UPROPERTY(EditAnywhere, Category = "Trace")
	float DamageAmount = 10.0f;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	// 记录本次攻击命中的Actor 防止重复造成伤害
	UPROPERTY()
	TArray<AActor*> HitActors;
};