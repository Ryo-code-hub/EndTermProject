#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BMWSavePoint.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UArrowComponent;

UCLASS()
class WUKONG_API ABMWSavePoint : public AActor
{
	GENERATED_BODY()

public:
	ABMWSavePoint();

protected:
	// 可视化模型
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	// 触发区域
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* TriggerBox;

	// 复活点位置指示器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* SpawnPointArrow;

	// 重叠事件函数
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};