// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BMWBaseCharacter.h" // 你的父类
#include "InputActionValue.h" // 增强输入需要的值类型
#include "BMWPlayerCharacter.generated.h"

// 定义闪避方向枚举
UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	Forward UMETA(DisplayName = "Forward"),
	Backward UMETA(DisplayName = "Backward"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class WUKONG_API ABMWPlayerCharacter : public ABMWBaseCharacter
{
	GENERATED_BODY()

public:
	// 构造函数
	ABMWPlayerCharacter();

protected:
	// 相当于蓝图的 Event BeginPlay
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;


	// 专门用于绑定输入的函数
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//  锁定状态标记
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsLockMode = false;

	//  是否允许移动
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bCanMove = true; // 默认是 true，允许移动

	//行走输入动作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;

	//  记录当前是否处于行走模式
	bool bIsWalking = false;

	//定义行走和奔跑的速度值，方便在编辑器调整
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
	float RunSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
	float WalkSpeed = 200.0f;

	


protected:
	/** 摄像机吊杆 (弹簧臂) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** 跟随摄像机 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** 输入映射上下文 (IMC) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** 移动输入动作 (IA_Move) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** 视角移动动作 (IA_Look - 对应鼠标) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	//  锁定输入动作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockAction;

	// 跳跃输入动作 (IA_Jump)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	//切换行走状态 (Toggle)
	void ToggleWalking();

	// 落地蒙太奇资源
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	UAnimMontage* LandMontage;

protected:
	/** 移动函数，对应蓝图里的 IA_Move Triggered */
	void Move(const FInputActionValue& Value);

	/** 视角函数，对应蓝图里的 IA_Look/MouseXY */
	void Look(const FInputActionValue& Value);

	// 切换锁定状态的函数
	void ToggleLock();

	//  跳跃开始 (按下空格)
	void JumpStarted();

	//  跳跃结束 (松开空格)
	void JumpEnded();

	//  落地事件 (复写系统自带的Landed函数)
	virtual void Landed(const FHitResult& Hit) override;

protected:
	//闪避输入 (空格)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	//  模拟完美闪避窗口 (M键)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SimulatePerfectAction;

	//  记录当前的输入方向 (X, Y)，在 Move 函数里更新
	FVector2D CurrentInputDirection;

	// 闪避连段计数
	int32 DodgeCount = 0;

	// 是否处于完美闪避窗口
	bool bIsPerfectDodgeWindow = false;

	//连段重置定时器句柄
	FTimerHandle TimerHandle_ResetDodge;

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

protected:
	//  闪避主逻辑
	void DodgeStarted();

	//  开启/关闭完美窗口 (测试用)
	void TogglePerfectWindow();

	// 辅助函数：计算闪避方向
	EDodgeDirection CalculateDodgeDirection();

	//辅助函数：重置连段
	void ResetDodgeCount();


	//****实现冲刺****
	protected:
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		UInputAction* DashAction;

		// 冲刺最大速度
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
		float DashMaxSpeed = 1200.0f;


		//冲刺冷却时间 (秒)
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Settings")
		float DashCooldownTime = 1.0f;

		// 冷却计时器句柄 
		FTimerHandle TimerHandle_DashCooldown;

		//冲刺持续时间

		//冲刺状态标记
		bool bIsDashing = false;

		// 默认跑动速度 
		float DefaultRunSpeed = 600.0f;

protected:
	//冲刺函数
	void Dash();

	// 冲刺结束
	void StopDashing();
};
