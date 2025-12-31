#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Sound/SoundBase.h" 
#include "AN_Footstep.generated.h"

UCLASS()
class WUKONG_API UAN_Footstep : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FName SocketName = FName("ball_l_01");

	// 声音音量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float VolumeMultiplier = 1.0f;

	//  不同材质的声音资产
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* Sound_Default;


public:
	// 重写通知函数
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
