// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_WeaponTrace.generated.h"

UCLASS()
class WUKONG_API UANS_WeaponTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	// ÎäÆ÷ÉÏµÄ²å²ÛÃû×Ö
	UPROPERTY(EditAnywhere, Category = "Trace")
	FName SocketStart = FName("TraceStart");

	UPROPERTY(EditAnywhere, Category = "Trace")
	FName SocketEnd = FName("TraceEnd");

	// ¼ì²â°ë¾¶
	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceRadius = 15.0f;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};