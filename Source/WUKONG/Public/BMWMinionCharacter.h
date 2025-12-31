#pragma once

#include "CoreMinimal.h"
#include "BMWBaseCharacter.h"
#include "BMWMinionCharacter.generated.h"

class UWidgetComponent;
class USphereComponent;
class UAnimMontage;

UCLASS()
class WUKONG_API ABMWMinionCharacter : public ABMWBaseCharacter
{
	GENERATED_BODY()

public:
	ABMWMinionCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	bool bHasAggro;

	// 闪避状态标记
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsDodging;

	// 闪避概率
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float DodgeChance = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* HitMontage;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	//UAnimMontage* DeathMontage;

	// 闪避蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	UAnimMontage* DodgeMontage;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttack();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	USphereComponent* AggroSphereComp;

	virtual void PostInitializeComponents() override;
	virtual void Die() override;

	// 重写受击核心函数
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void OnAggroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, UBMWAttributeComponent* OwningComp, float NewHealth, float Delta) override;

	// 蒙太奇结束回调
	UFUNCTION()
	void OnDodgeFinished(UAnimMontage* Montage, bool bInterrupted);

public:
	// 处理攻击命中
	void ProcessHit(FHitResult HitResult);

protected:
	// 记录已打中的人，防止一刀多段伤害
	TArray<AActor*> HitActors;

public:
	// 开启/关闭检测 (供 Notify 调用)
	void StartWeaponCollision() { HitActors.Empty(); }
	void EndWeaponCollision() { HitActors.Empty(); }
};