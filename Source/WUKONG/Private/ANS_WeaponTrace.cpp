// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_WeaponTrace.h"
#include "Kismet/KismetSystemLibrary.h"
#include "BMWPlayerCharacter.h"

void UANS_WeaponTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(MeshComp->GetOwner()))
		{
			Player->StartWeaponCollision();
		}
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
		EDrawDebugTrace::ForDuration,
		HitResults,
		true
	);

	if (bHit)
	{
		if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(OwnerActor))
		{
			for (const FHitResult& Hit : HitResults)
			{
				Player->ProcessHit(Hit);
			}
		}
	}
}

void UANS_WeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ABMWPlayerCharacter* Player = Cast<ABMWPlayerCharacter>(MeshComp->GetOwner()))
		{
			Player->EndWeaponCollision();
		}
	}
}