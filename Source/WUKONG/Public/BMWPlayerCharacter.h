//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BMWBaseCharacter.h" // 你的父类
#include "InputActionValue.h" // 增强输入需要的值类型
#include "BMWPlayerCharacter.generated.h"


// 前置声明
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UNiagaraComponent;
class UNiagaraSystem;

// 定义闪避方向枚举
UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	Forward UMETA(DisplayName = "Forward"),
	Backward UMETA(DisplayName = "Backward"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UCLASS()
class WUKONG_API ABMWPlayerCharacter : public ABMWBaseCharacter
{
	GENERATED_BODY()

public:
	ABMWPlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// === 组件 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// === 输入配置 ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SimulatePerfectAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	// === 状态变量 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsLockMode = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bCanMove = true;

	bool bIsWalking = false;
	bool bIsDashing = false;

	// 攻击连招相关
	int32 ComboIndex = 0;
	bool bIsAttacking = false;
	bool bHasSavedAttack = false;
	bool bReadyForNextCombo = false;

	// === 战斗参数 ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
	float RunSpeed = 650.0f;

	// 默认跑动速度 
	float DefaultRunSpeed = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
	float WalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
	float DashMaxSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
	float DashCooldownTime = 1.0f;

	// === 资源 ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	UAnimMontage* LandMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
	float LandHardThreshold = 900.0f;

	// === 蒙太奇资产 ===

//前闪
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_perfect_F; // 完美前闪

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_F_1; // 普通前闪 1

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_F_2; // 普通前闪 2

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_F_3; // 普通前闪 3

	//后闪
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_perfect_B; // 完美后闪

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_B_1; // 普通后闪 1

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_B_2; // 普通后闪 2

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_B_3; // 普通后闪 3

	//左闪
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_perfect_L; // 完美左闪

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_L_1; // 普通左闪 1

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_L_2; // 普通左闪 2

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_L_3; // 普通左闪 3

	//右闪
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_perfect_R; // 完美右闪

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_R_1; // 普通右闪 1

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_R_2; // 普通右闪 2

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Dodge_R_3; // 普通右闪 3

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	TArray<UAnimMontage*> AttackMontages;

	// === 内部变量 ===
	FVector2D CurrentInputDirection;
	int32 DodgeCount = 0;
	bool bIsPerfectDodgeWindow = false;
	FTimerHandle TimerHandle_ResetDodge;
	FTimerHandle TimerHandle_DashCooldown;

	// 攻击判定用的数组
	TArray<AActor*> HitActors;

protected:
	// === 函数 ===
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ToggleLock();
	void JumpStarted();
	void JumpEnded();
	virtual void Landed(const FHitResult& Hit) override;
	void ToggleWalking();
	void Dash();
	void StopDashing();
	void DodgeStarted();
	void TogglePerfectWindow();
	EDodgeDirection CalculateDodgeDirection();
	void ResetDodgeCount();

	// 攻击函数
	void AttackInput();
	void PerformComboAttack();

public:
	// === Notify 调用的函数 ===
	UFUNCTION(BlueprintCallable)
	void ContinueCombo();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	void StartWeaponCollision();
	void EndWeaponCollision();
	void ProcessHit(FHitResult HitResult);



protected:
	//连招保留计时器句柄
	FTimerHandle TimerHandle_ComboPreserve;

	//  连招保留窗口时间 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ComboPreserveTime = 1.5f;

protected:
	// 软重置：只重置状态
	void SoftResetAttackState();


	//重击
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

	// === 蓄力攻击参数 ===

	// 每一阶蓄力需要的时间 (秒)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HeavyAttack")
	float ChargeTimePerStage = 2.0f;

	// 短按判定的阈值 (小于这个时间算普通重击)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|HeavyAttack")
	float ShortPressThreshold = 0.2f;

	// === 蓄力状态变量 ===
	bool bIsCharging = false;       // 是否正在蓄力
	float CurrentChargeTime = 0.0f; // 当前蓄力了多久
	int32 CurrentChargeStage = 0;   // 当前阶数 (0=普通, 1, 2, 3)

	// === 蒙太奇资源 ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Charge_Loop;    // 蓄力循环动作

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Heavy_Normal;   // 0阶：短按重击

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Heavy_Stage1;   // 1阶

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Heavy_Stage2;   // 2阶

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Heavy_Stage3;   // 3阶

protected:
	// 按下右键：开始蓄力
	void HeavyAttackStarted();

	// 松开右键：释放攻击
	void HeavyAttackReleased();

	//  辅助：处理蓄力时的转向
	void HandleChargeRotation(float DeltaTime);

	//武器特效
protected:
	// 武器特效资产
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|VFX")
	UNiagaraSystem* WeaponFireFX;

	// 武器上端的特效组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|VFX")
	UNiagaraComponent* WeaponFXComp_Top;

	// 武器下端的特效组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|VFX")
	UNiagaraComponent* WeaponFXComp_Bottom;

public:
	// 用于后续切换形态的函数
	UFUNCTION(BlueprintCallable)
	void SetWeaponFireColor(FLinearColor NewColor);



	//技能和棍势
protected:
	//超级重击技能输入 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SpecialSkillAction;

	// 超级重击蒙太奇 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Montage")
	UAnimMontage* AM_Heavy_Special;


	// 当前棍势点数 (0.0 ~ 4.0)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Focus")
	float CurrentFocusPoint = 0.0f;

	// 最大棍势
	float MaxFocusPoint = 4.0f;

	//特效颜色配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|VFX")
	FLinearColor FocusColor_Level1 = FLinearColor(50.f, 50.f, 50.f); // 白/银 (高亮)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|VFX")
	FLinearColor FocusColor_Level2 = FLinearColor(50.f, 40.f, 0.f);  // 金色

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|VFX")
	FLinearColor FocusColor_Level3 = FLinearColor(50.f, 0.f, 0.f);   // 红色

public:
	// 供 Notify 调用的函数：增加棍势
	// Amount = 0.34 (约等于1/3)
	UFUNCTION(BlueprintCallable)
	void AddFocus(float Amount);

protected:
	//更新特效状态 (根据点数变色)
	void UpdateFocusVFX();

	// 消耗所有棍势 
	void ConsumeAllFocus();
	//  专门用于关闭特效的函数
	UFUNCTION(BlueprintCallable)
	void TurnOffFocusVFX();

	//  释放大招
	void PerformSpecialSkill();

	//控制技能镜头
protected:
	//是否开启镜头跟随盆骨高度 (在飞起动作时开启)
	bool bFollowPelvisZ = false;

	//记录盆骨的标准高度 (站立时盆骨离地的高度，用于计算差值)
	float DefaultPelvisZ = 0.0f;

public:
	// 供 Notify 调用：开启/关闭跟随
	UFUNCTION(BlueprintCallable)
	void SetCameraFollowPelvis(bool bEnable);
};







