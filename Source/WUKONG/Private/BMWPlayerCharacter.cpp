// Fill out your copyright notice in the Description page of Project Settings.


#include "BMWPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Animation/AnimMontage.h" 
#include "TimerManager.h" 
#include "Engine/Engine.h" 

ABMWPlayerCharacter::ABMWPlayerCharacter()
{
	// 设置 Character Tick
	PrimaryActorTick.bCanEverTick = true;

	// 1. 设置角色的旋转设置 
	// 不要让控制器旋转直接带动角色旋转
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 配置角色移动组件
	GetCharacterMovement()->bOrientRotationToMovement = true; // 让角色朝向移动方向
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 转身速度

	// 确保默认速度是奔跑速度
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

	// 2. 创建弹簧臂 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // 臂长
	CameraBoom->bUsePawnControlRotation = true; 

	// 3. 创建摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 挂在弹簧臂末端
	FollowCamera->bUsePawnControlRotation = false;
}

// BeginPlay
void ABMWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 添加输入映射上下文 (Add Mapping Context)
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// 记录原始奔跑速度
	DefaultRunSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

// 移动函数
void ABMWPlayerCharacter::Move(const FInputActionValue& Value)
{
	//实时记录输入方向，供闪避逻辑使用
	CurrentInputDirection = Value.Get<FVector2D>();
	// 检查是否允许移动
	if (!bCanMove)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 1. 获取控制器旋转 
		const FRotator Rotation = Controller->GetControlRotation();
		// 2. 只保留 Yaw
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 3. 获取前向向量
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// 4. 获取右向向量 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 5. 应用移动输入
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

//对应鼠标输入
void ABMWPlayerCharacter::Look(const FInputActionValue& Value)
{
	// 1.  检查锁定状态
	// 如果处于锁定模式，直接返回，不再执行后面的视角旋转代码
	// 这样鼠标就“失效”了，视角将完全由 Tick 中的逻辑（或未来的锁敌逻辑）控制
	if (bIsLockMode)
	{
		return;
	}

	// 2. 获取输入数据
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 3. 应用旋转
		// X轴控制左右看 (Yaw)
		AddControllerYawInput(LookAxisVector.X);

		// Y轴控制上下看 (Pitch)
		AddControllerPitchInput(LookAxisVector.Y * -1.0f);
	}
}

// 绑定输入
void ABMWPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 转换成增强输入组件
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// 绑定移动 (Triggered)
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABMWPlayerCharacter::Move);
		}

		// 绑定视角 (Triggered)
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABMWPlayerCharacter::Look);
		}

		// 绑定锁定键 (Started 表示按下瞬间触发一次)
		if (LockAction)
		{
			EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::ToggleLock);
		}

		if (JumpAction)
		{
			// Started: 按下瞬间
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::JumpStarted);
			// Completed: 松开瞬间 (可选，用于控制跳跃高度)
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABMWPlayerCharacter::JumpEnded);
		}

		//  绑定闪避和模拟键M
		if (DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::DodgeStarted);
		}
		if (SimulatePerfectAction)
		{
			EnhancedInputComponent->BindAction(SimulatePerfectAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::TogglePerfectWindow);
		}

		// 绑定行走切换
		if (WalkAction)
		{
			// 只在按下瞬间触发一次 Toggle
			EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::ToggleWalking);
		}

		//绑定闪避
		if (DashAction)
		{
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::Dash);
		}

		//绑定攻击
		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::AttackInput);
		}
	}
}

// Tick 函数
void ABMWPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); 

	// === 逻辑 1：锁定模式下的强制朝向===
	if (bIsLockMode)
	{
		if (Controller)
		{
			// 获取目标旋转 (摄像机朝向)
			FRotator TargetRotation = Controller->GetControlRotation();
			TargetRotation.Pitch = 0.0f;
			TargetRotation.Roll = 0.0f;

			// 平滑插值
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f);

			// 应用旋转
			SetActorRotation(NewRotation);
		}
	}

	// === 逻辑 2：冲刺模式下的平滑减速 ===
	if (bIsDashing)
	{
		// 获取当前最大速度
		float CurrentMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;

		// 如果当前速度还大于默认速度，就慢慢减速
		if (CurrentMaxSpeed > DefaultRunSpeed)
		{
			// 平滑插值减速
			float NewSpeed = FMath::FInterpTo(CurrentMaxSpeed, DefaultRunSpeed, DeltaTime, 2.0f);
			GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
		}
		else
		{
			// 速度降回来后，强制设为默认值
			GetCharacterMovement()->MaxWalkSpeed = DefaultRunSpeed;
		}
	}
}

//ToggleLock 函数
void ABMWPlayerCharacter::ToggleLock()
{
	// 1. 切换布尔值 
	bIsLockMode = !bIsLockMode;

	// 2. 根据状态改变移动组件设置
	if (bIsLockMode)
	{
		// === 进入锁定模式 ===
		// 禁止角色自动随移动转身 
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		// === 回到自由模式 ===
		// 恢复角色自动随移动转身
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	// 调试打印：
	if (GEngine)
	{
		FString Msg = bIsLockMode ? TEXT("Locked Mode: ON") : TEXT("Locked Mode: OFF");
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, Msg);
	}
}

void ABMWPlayerCharacter::JumpStarted()
{
	// 只有在允许移动的情况下才能跳跃
	if (bCanMove)
	{
		// 调用父类ACharacter自带的跳跃函数
		Jump();

		// 起跳后，禁止水平移动控制
		bCanMove = false;

		// 调试打印
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Jump: Movement Locked"));
	}
}

void ABMWPlayerCharacter::JumpEnded()
{
	StopJumping();
}

// 落地时自动触发
void ABMWPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 1. 落地瞬间，依然禁止移动
	bCanMove = false;

	// 2. 播放落地蒙太奇
	if (LandMontage)
	{
		PlayAnimMontage(LandMontage);
	}
	else
	{
		// 保底逻辑：如果没有蒙太奇，那就直接恢复移动，防止卡死
		bCanMove = true;
	}

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Landed: Movement Restored"));
}

//  ****闪避辅助函数****
EDodgeDirection ABMWPlayerCharacter::CalculateDodgeDirection()
{
	// 如果没有输入 (0,0)，默认向后
	if (CurrentInputDirection.IsZero())
	{
		return EDodgeDirection::Backward;
	}

	// 比较 X(左右) 和 Y(前后) 的绝对值
	if (FMath::Abs(CurrentInputDirection.Y) >= FMath::Abs(CurrentInputDirection.X))
	{
		// 前后为主
		return (CurrentInputDirection.Y > 0) ? EDodgeDirection::Forward : EDodgeDirection::Backward;
	}
	else
	{
		// 左右为主
		return (CurrentInputDirection.X > 0) ? EDodgeDirection::Right : EDodgeDirection::Left;
	}
}

// 重置连段
void ABMWPlayerCharacter::ResetDodgeCount()
{
	DodgeCount = 0;
	// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Dodge Combo Reset"));
}

// 模拟完美窗口 (M键)
void ABMWPlayerCharacter::TogglePerfectWindow()
{
	bIsPerfectDodgeWindow = !bIsPerfectDodgeWindow;
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple,
		bIsPerfectDodgeWindow ? TEXT("Perfect Window: OPEN") : TEXT("Perfect Window: CLOSED"));
}
//  ****闪避辅助函数****

//****核心闪避逻辑****
void ABMWPlayerCharacter::DodgeStarted()
{
	// 1. 闪避取消逻辑 (Animation Cancel)
	if (bIsAttacking)
	{
		StopAnimMontage(nullptr);
		SoftResetAttackState();

		GetWorldTimerManager().SetTimer(
			TimerHandle_ComboPreserve,
			this,
			&ABMWPlayerCharacter::ResetCombo, // 时间到了就调用彻底重置
			ComboPreserveTime,
			false

		);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Combo Preserved!"));

	}

	// 落地硬直检查
	if (!bCanMove && GetCharacterMovement()->IsMovingOnGround())
	{
		bCanMove = true;
	}

	// 如果还没恢复移动（比如在空中），则不能闪避
	if (!bCanMove) return;

	// 2. 计算方向
	EDodgeDirection Direction = CalculateDodgeDirection();
	UAnimMontage* MontageToPlay = nullptr;

	// 3. 分支判断：完美 vs 普通
	if (bIsPerfectDodgeWindow)
	{
		// === 完美闪避逻辑 (保持不变) ===
		switch (Direction)
		{
			case EDodgeDirection::Forward:
				MontageToPlay = AM_Dodge_perfect_F; break;
			case EDodgeDirection::Backward:
				MontageToPlay = AM_Dodge_perfect_B; break;
			case EDodgeDirection::Left:
				MontageToPlay = AM_Dodge_perfect_L; break;
			case EDodgeDirection::Right:
				MontageToPlay = AM_Dodge_perfect_R; break;
			default: break;
		}

		if (MontageToPlay)
		{
			PlayAnimMontage(MontageToPlay);
			DodgeCount = 0; // 完美闪避后重置
		}
	}
	else
	{
		// === 普通多段闪避逻辑  ===
		switch (Direction)
		{
			case EDodgeDirection::Forward:
				if (DodgeCount == 0)      MontageToPlay = AM_Dodge_F_1;
				else if (DodgeCount == 1) MontageToPlay = AM_Dodge_F_2; // 必须用 else if
				else                      MontageToPlay = AM_Dodge_F_3; // 2及以上播第3段
				break;

			case EDodgeDirection::Backward:
				if (DodgeCount == 0)      MontageToPlay = AM_Dodge_B_1;
				else if (DodgeCount == 1) MontageToPlay = AM_Dodge_B_2;
				else                      MontageToPlay = AM_Dodge_B_3;
				break;

			case EDodgeDirection::Left:
				if (DodgeCount == 0)      MontageToPlay = AM_Dodge_L_1;
				else if (DodgeCount == 1) MontageToPlay = AM_Dodge_L_2;
				else                      MontageToPlay = AM_Dodge_L_3;
				break;

			case EDodgeDirection::Right:
				if (DodgeCount == 0)      MontageToPlay = AM_Dodge_R_1;
				else if (DodgeCount == 1) MontageToPlay = AM_Dodge_R_2;
				else                      MontageToPlay = AM_Dodge_R_3;
				break;

			default: break;
		}

		// 播放逻辑
		if (MontageToPlay)
		{
			// 打断上一个动作，保证连闪的手感
			StopAnimMontage(nullptr);

			PlayAnimMontage(MontageToPlay);

			// 计数增加
			DodgeCount++;

			if (DodgeCount > 2)
			{
				DodgeCount = 0;
			}

			// 设置重置计时器
			GetWorldTimerManager().ClearTimer(TimerHandle_ResetDodge);
			GetWorldTimerManager().SetTimer(TimerHandle_ResetDodge, this, &ABMWPlayerCharacter::ResetDodgeCount, 1.5f, false);
		}
	}
}
//****核心闪避逻辑****


//****行走奔跑切换****
void ABMWPlayerCharacter::ToggleWalking()
{
	// 1. 状态取反 (Flip-Flop)
	bIsWalking = !bIsWalking;

	// 2. 根据状态设置速度
	if (bIsWalking)
	{
		// 进入行走模式
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; // 200

		//调试打印
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Mode: Walking"));
	}
	else
	{
		// 回到奔跑模式
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed; // 600

		// 调试打印
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Mode: Running"));
	}
}


//****Dash****
void ABMWPlayerCharacter::Dash()
{
	// 1. 检查冷却和移动状态
	if (!bCanMove || bIsDashing)
	{
		return;
	}

	// 2. 瞬间提速
	GetCharacterMovement()->MaxWalkSpeed = DashMaxSpeed;

	// 3. 标记状态
	bIsDashing = true;

	// 4. 设置冷却/结束计时器
	// 1.0秒后强制重置状态
	GetWorldTimerManager().SetTimer(TimerHandle_DashCooldown, this, &ABMWPlayerCharacter::StopDashing, DashCooldownTime, false);

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Speed Boost!"));
}

void ABMWPlayerCharacter::StopDashing()
{
	// 冷却结束，允许下次冲刺
	bIsDashing = false;
}



//****attacking****
// 1. 玩家按下左键
void ABMWPlayerCharacter::AttackInput()
{
	if (!bCanMove && !bIsAttacking) return;

	if (bIsAttacking)
	{
		// 情况A：还没到 ContinueCombo 点，先存着
		if (!bReadyForNextCombo)
		{
			bHasSavedAttack = true;
		}
		// 情况B：
		else
		{
			PerformComboAttack();
		}
	}
	else
	{
		PerformComboAttack();
	}
}

// 2. 执行攻击动作 (核心)
void ABMWPlayerCharacter::PerformComboAttack()
{
	if (AttackMontages.Num() == 0) return;

	// 确保 Index 不越界 (虽然逻辑上会重置，但加个保险)
	if (AttackMontages.IsValidIndex(ComboIndex))
	{
		// 1. 锁死状态
		bIsAttacking = true;
		bCanMove = false; // 攻击时不能移动(硬直)
		bHasSavedAttack = false; // 消耗掉缓存
		bReadyForNextCombo = false;

		// 2. 播放对应的蒙太奇
		PlayAnimMontage(AttackMontages[ComboIndex]);

		// 3. 准备下一段的下标
		// 如果当前是 0，下一次就是 1。如果是 3 (数组长度4)，下一次变成 0
		ComboIndex++;
		if (ComboIndex >= AttackMontages.Num())
		{
			ComboIndex = 0;
		}
	}
}

// 3. 连招检测 (将被 AnimNotify 调用)
// 就像接力棒：动画播到这一帧，问一下C++：“玩家刚才按键了吗？”
void ABMWPlayerCharacter::ContinueCombo()
{
	if (bHasSavedAttack)
	{
		PerformComboAttack();
	}
	else
	{
		bReadyForNextCombo = true;
	}
}

// 4. 重置连招 (将被 AnimNotify 或 BlendOut 调用)
void ABMWPlayerCharacter::ResetCombo()
{
	ComboIndex = 0;
	bIsAttacking = false;
	bHasSavedAttack = false;
	bReadyForNextCombo = false;
	bCanMove = true; // 恢复移动
}

void ABMWPlayerCharacter::StartWeaponCollision()
{
	// 每次挥刀开始时，清空受击者名单
	HitActors.Empty();
}

void ABMWPlayerCharacter::EndWeaponCollision()
{
	// 可以在这里做一些收尾工作，比如重置特效等
	HitActors.Empty();
}

void ABMWPlayerCharacter::ProcessHit(FHitResult HitResult)
{
	AActor* HitActor = HitResult.GetActor();

	// 1. 如果打到的是自己，忽略
	if (!HitActor || HitActor == this) return;

	// 2. 如果这个人这一刀已经打过了，忽略
	if (HitActors.Contains(HitActor)) return;

	// 3. 记录这个人
	HitActors.Add(HitActor);

	// 4. 应用伤害 (这里先打印，后面再接伤害系统)
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("Hit Enemy: %s"), *HitActor->GetName());
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, DebugMsg);
	}

	// TODO: UGameplayStatics::ApplyDamage(...)
	// 播放打击音效、特效、顿帧(HitStop)等

}

//闪避保留连击
void ABMWPlayerCharacter::SoftResetAttackState()
{
	// 重置攻击状态标记
	bIsAttacking = false;
	bHasSavedAttack = false;
	bReadyForNextCombo = false; 

	// 恢复移动能力 
	bCanMove = true;

}