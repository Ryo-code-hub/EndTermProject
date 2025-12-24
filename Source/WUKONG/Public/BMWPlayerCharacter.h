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

	// === 蒙太奇资产

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
	// 软重置：只重置状态（让你可以动），但不重置 ComboIndex
	void SoftResetAttackState();
};







