#include "BMWPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h" 
#include "Animation/AnimMontage.h" 
#include "TimerManager.h" 
#include "Engine/Engine.h" 
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BMWAttributeComponent.h" 
#include "BMWTargetingComponent.h"
#include "BMWMainHUD.h" 
#include "BMWStartMenu.h" 
#include "BMWPauseMenu.h"
#include "BMWGameMode.h"
#include "Kismet/KismetSystemLibrary.h" // 用于球形检测
#include "DrawDebugHelpers.h" // 用于画线调试
#include "Kismet/GameplayStatics.h"

ABMWPlayerCharacter::ABMWPlayerCharacter()
{

	/* 挂载雷达                                                       */
	TargetingComp = CreateDefaultSubobject<UBMWTargetingComponent>(TEXT("TargetingComp"));

	// 设置 Character Tick
	PrimaryActorTick.bCanEverTick = true;

	// 设置角色的旋转设置 
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 配置角色移动组件
	GetCharacterMovement()->bOrientRotationToMovement = true; // 让角色朝向移动方向
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // 转身速度

	// 确保默认速度是奔跑速度
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

	// 创建弹簧臂 
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // 臂长
	CameraBoom->bUsePawnControlRotation = true;

	// 创建摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 挂在弹簧臂末端
	FollowCamera->bUsePawnControlRotation = false;

	// 创建特效
	// 创建特效组件 - 上端
	WeaponFXComp_Top = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeaponFX_Top"));
	WeaponFXComp_Top->SetupAttachment(GetMesh(), FName("FX_Staff_Top"));
	WeaponFXComp_Top->bAutoActivate = false; // 默认不开启，或者设为 true 看你需求

	// 创建特效组件 - 下端
	WeaponFXComp_Bottom = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WeaponFX_Bottom"));
	WeaponFXComp_Bottom->SetupAttachment(GetMesh(), FName("FX_Staff_Bottom"));
	WeaponFXComp_Bottom->bAutoActivate = false;
}


// BeginPlay
void ABMWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 添加输入映射上下文
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		// 初始UI加载                                                        
		/* 如果配置了开始菜单，则在游戏开始时显示并接管输入                */
		if (StartMenuClass)
		{
			UBMWStartMenu::ShowMenu(PlayerController, StartMenuClass);
		}

		// 默认重生点
		RespawnPointTransform = GetActorTransform();
	}

	// 记录原始奔跑速度
	DefaultRunSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// 如果蓝图里填了特效资产，就应用给组件
	if (WeaponFireFX)
	{
		WeaponFXComp_Top->SetAsset(WeaponFireFX);
		WeaponFXComp_Bottom->SetAsset(WeaponFireFX);

		// 激活特效
		WeaponFXComp_Top->Activate();
		WeaponFXComp_Bottom->Activate();
	}

	// 获取初始盆骨高度
	if (GetMesh())
	{
		DefaultPelvisZ = GetMesh()->GetSocketLocation(FName("pelvis")).Z - GetActorLocation().Z;
	}
}

// 移动函数
void ABMWPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (!IsAlive()) return;

	// 实时记录输入方向
	CurrentInputDirection = Value.Get<FVector2D>();
	// 检查是否允许移动
	if (!bCanMove)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 获取控制器旋转 
		const FRotator Rotation = Controller->GetControlRotation();
		// 只保留 Yaw
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// 获取前向向量
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// 获取右向向量 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// 应用移动输入
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

//对应鼠标输入
void ABMWPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (!IsAlive()) return;

	// 检查锁定状态
	if (bIsLockMode)
	{
		return;
	}

	// 获取输入数据
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// 应用旋转
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

		/********************************************************************************/
		/* 暂停菜单                                                      */
		if (PauseAction)
		{
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::OnPauseInput);
		}
		/********************************************************************************/


		// 绑定移动
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABMWPlayerCharacter::Move);
		}

		// 绑定视角
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABMWPlayerCharacter::Look);
		}

		// 绑定锁定键
		if (LockAction)
		{
			EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::ToggleLock);
		}

		if (JumpAction)
		{
			// Started: 按下瞬间
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::JumpStarted);
			// Completed: 松开瞬间
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

		//绑定重击
		if (HeavyAttackAction)
		{
			// 按下开始蓄力
			EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::HeavyAttackStarted);
			// 松开释放攻击
			EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Completed, this, &ABMWPlayerCharacter::HeavyAttackReleased);
		}

		//绑定技能
		if (SpecialSkillAction)
		{
			EnhancedInputComponent->BindAction(SpecialSkillAction, ETriggerEvent::Started, this, &ABMWPlayerCharacter::PerformSpecialSkill);
		}
	}
}

// Tick 函数
void ABMWPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// [新增]=== 1. 锁定逻辑 ===
	if (bIsLockMode)
	{
		// 安全检查：如果目标没了，或者死了，或者跑太远了，自动解锁
		if (!LockTargetActor ||
			!Cast<ABMWBaseCharacter>(LockTargetActor)->IsAlive() ||
			GetDistanceTo(LockTargetActor) > LockDistance * 1.2f) // 给一点缓冲距离
		{
			ToggleLock(); // 自动取消
		}
		else
		{
			// --- A. 更新摄像机朝向 (让屏幕中心对准敌人) ---
			if (Controller)
			{
				// 计算“我”看向“敌人”的旋转
				// 稍微往下一点 (-20度)，因为通常是俯视视角，不想只看敌人的脚
				FVector TargetLoc = LockTargetActor->GetActorLocation();
				// 最好瞄准敌人的胸口，而不是脚底板
				TargetLoc.Z -=50.0f; // 假设中心高10.0

				FVector StartLoc = GetActorLocation();
				// 摄像机旋转基准位置可以用 ActorLocation 或 CameraBoom 位置

				FRotator LookAtRot = (TargetLoc - StartLoc).Rotation();

				// 我们可以限制一下 Pitch，防止相机钻地，或者让弹簧臂自己处理
				// LookAtRot.Pitch = FMath::Clamp(LookAtRot.Pitch, -45.0f, 45.0f);

				// 平滑插值设置给控制器
				FRotator NewCtrlRot = FMath::RInterpTo(Controller->GetControlRotation(), LookAtRot, DeltaTime, 10.0f);
				Controller->SetControlRotation(NewCtrlRot);
			}

			// --- B. 更新角色朝向 (让主角脸朝敌人) ---
			// 我们只关心水平旋转 (Yaw)，不想让主角仰头
			FVector DirToEnemy = LockTargetActor->GetActorLocation() - GetActorLocation();
			FRotator CharLookRot = DirToEnemy.Rotation();
			CharLookRot.Pitch = 0.0f;
			CharLookRot.Roll = 0.0f;

			// 平滑插值设置给 Actor
			FRotator NewActorRot = FMath::RInterpTo(GetActorRotation(), CharLookRot, DeltaTime, 10.0f);
			SetActorRotation(NewActorRot);
		}
	}

// 奔跑体力处理
	bool bIsMoving = GetVelocity().Size() > 0.1f;
	bool bIsRunning = GetVelocity().Size() > WalkSpeed + 10.0f;

	if (AttributeComp)
	{
		AttributeComp->HandleSprintingStamina(bIsMoving, bIsRunning, DeltaTime);
		// 如果没体力了，强制切回行走
		if (AttributeComp->GetStamina() <= 0.0f && bIsRunning)
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			bIsWalking = true;
		}
	}

	// 雷达扫描
	if (TargetingComp)
	{
		TargetingComp->UpdateTargetingLogic(FollowCamera, MainHUDInstance);
	}
	/********************************************************************************/

	// 锁定模式下的强制朝向
	if (bIsLockMode)
	{
		if (Controller)
		{
			// 获取目标旋转
			FRotator TargetRotation = Controller->GetControlRotation();
			TargetRotation.Pitch = 0.0f;
			TargetRotation.Roll = 0.0f;

			// 平滑插值
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f);

			// 应用旋转
			SetActorRotation(NewRotation);
		}
	}

	// 冲刺模式下的平滑减速
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

	// 蓄力逻辑
	if (bIsCharging)
	{
		/* 逻辑: 蓄力持续消耗体力与自动释放                                        */
		if (AttributeComp)
		{
			// 尝试扣除本帧体力，返回 false 表示体力耗尽
			bool bHasStamina = AttributeComp->ConsumeHeavyChargeStamina(DeltaTime);

			// 如果体力耗尽，强制自动释放重击
			if (!bHasStamina)
			{
				HeavyAttackReleased();
				return; // 退出当前 Tick 的蓄力逻辑，避免执行后续代码
			}
		}
		/********************************************************************************/
		/*  蓄力时同步棍势到UI                                                */
		float CalculatedFocus = CurrentChargeTime / ChargeTimePerStage;
		CurrentFocusPoint = FMath::Clamp(CalculatedFocus, 0.0f, MaxFocusPoint);
		OnFocusChanged.Broadcast(CurrentFocusPoint); // 广播UI
		/********************************************************************************/

		//记录计算前的阶数
		int OldStage = CurrentChargeStage;

		// 累加时间
		CurrentChargeTime += DeltaTime;

		// 计算当前阶数
		if (CurrentChargeTime >= ShortPressThreshold)
		{
			// 时间越长，阶数越高
			if (CurrentChargeTime >= ChargeTimePerStage * 3) CurrentChargeStage = 3;
			else if (CurrentChargeTime >= ChargeTimePerStage * 2) CurrentChargeStage = 2;
			else if (CurrentChargeTime >= ChargeTimePerStage * 1) CurrentChargeStage = 1;
		}
		// 处理转向
		HandleChargeRotation(DeltaTime);

		// 蓄力时特效
		if (CurrentChargeStage >= 1)
		{
			// 强制开启特效
			if (WeaponFXComp_Top && !WeaponFXComp_Top->IsActive()) WeaponFXComp_Top->Activate();
			if (WeaponFXComp_Bottom && !WeaponFXComp_Bottom->IsActive()) WeaponFXComp_Bottom->Activate();

			// 根据当前蓄到的阶数变色
			FLinearColor TempColor = FocusColor_Level1; // 默认1阶白

			if (CurrentChargeStage == 2) TempColor = FocusColor_Level2;      // 2阶金
			else if (CurrentChargeStage >= 3) TempColor = FocusColor_Level3; // 3阶红

			// 应用颜色
			SetWeaponFireColor(TempColor);
		}

		// 蓄力进阶音效
		if (CurrentChargeStage > OldStage)
		{
			USoundBase* SoundToPlay = nullptr;

			switch (CurrentChargeStage)
			{
				case 1: SoundToPlay = SFX_FocusLevelUp_1; break; // 升到1阶
				case 2: SoundToPlay = SFX_FocusLevelUp_1; break; // 升到2阶
				case 3: SoundToPlay = SFX_FocusLevelUp_1; break; // 升到3阶
				case 4: SoundToPlay = SFX_FocusLevelUp_2; break; // 升到4阶
				default: break;
			}

			if (SoundToPlay)
			{
				UGameplayStatics::PlaySound2D(this, SoundToPlay);
			}
		}
	}


	// 镜头高度跟随逻辑

	float TargetOffsetZ = 0.0f; // 默认偏移是 0

	if (bFollowPelvisZ && GetMesh())
	{
		// 获取当前盆骨的世界高度
		float CurrentPelvisZ = GetMesh()->GetSocketLocation(FName("pelvis")).Z;

		// 获取当前胶囊体的世界高度
		float CapsuleZ = GetActorLocation().Z;

		float HeightDifference = (CurrentPelvisZ - CapsuleZ) - DefaultPelvisZ;

		// 只在向上飞时跟随，或者上下都跟随
		TargetOffsetZ = HeightDifference;
	}

	// 平滑应用给弹簧臂 (插值)
	float NewOffsetZ = FMath::FInterpTo(CameraBoom->TargetOffset.Z, TargetOffsetZ, DeltaTime, 10.0f);
	CameraBoom->TargetOffset.Z = NewOffsetZ;

	// 在 Tick 函数的镜头逻辑里
	if (bFollowPelvisZ && GetMesh())
	{
		float CurrentPelvisZ = GetMesh()->GetSocketLocation(FName("Pelvis")).Z; // 记得改骨骼名
		float CapsuleZ = GetActorLocation().Z;
		float HeightDifference = (CurrentPelvisZ - CapsuleZ) - DefaultPelvisZ;

		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red,
		//	FString::Printf(TEXT("Diff: %f | BoomZ: %f"), HeightDifference, CameraBoom->TargetOffset.Z));

		TargetOffsetZ = HeightDifference;
	}
}

//ToggleLock 函数
void ABMWPlayerCharacter::ToggleLock()
{
	// 情况A：如果已经锁定了，那就解锁
	if (bIsLockMode)
	{
		bIsLockMode = false;
		LockTargetActor = nullptr; // 清空目标
		GetCharacterMovement()->bOrientRotationToMovement = true; // 恢复自由跑

		/*if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Lock: OFF"));*/
	}
	// 情况B：还没锁定，尝试找人
	else
	{
		AActor* FoundTarget = FindBestLockTarget();

		if (FoundTarget)
		{
			bIsLockMode = true;
			LockTargetActor = FoundTarget; // 记下这个人
			GetCharacterMovement()->bOrientRotationToMovement = false; // 进入平移模式

			/*if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Locked: %s"), *FoundTarget->GetName()));*/
		}
		else
		{
			// 没找到人，锁定失败，保持自由模式
			/*if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Silver, TEXT("No Target Found!"));*/
		}
	}
}

void ABMWPlayerCharacter::JumpStarted()
{
	if (!IsAlive()) return;
	/********************************************************************************/
    /* 检查体力                                                      */
	if (AttributeComp && !AttributeComp->TryConsumeActionStamina(20.0f)) return;
	/********************************************************************************/

	// 只有在允许移动的情况下才能跳跃
	if (bCanMove)
	{
		// 调用父类ACharacter自带的跳跃函数
		Jump();

		// 起跳后，禁止水平移动控制
		bCanMove = false;

		//// 调试打印
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Jump: Movement Locked"));
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

	// 获取落地前一瞬间的垂直速度
	float FallingSpeed = -GetCharacterMovement()->GetLastUpdateVelocity().Z;
	// 播放落地蒙太奇
	if (FallingSpeed >= LandHardThreshold)
	{
		PlayAnimMontage(LandMontage);
	}
	else
	{
		bCanMove = true;
	}

	/*if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Landed: Movement Restored"));*/
}

//  闪避辅助函数
EDodgeDirection ABMWPlayerCharacter::CalculateDodgeDirection()
{
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
}

// 模拟完美窗口 (M键)
void ABMWPlayerCharacter::TogglePerfectWindow()
{
	bIsPerfectDodgeWindow = !bIsPerfectDodgeWindow;
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple,
	//	bIsPerfectDodgeWindow ? TEXT("Perfect Window: OPEN") : TEXT("Perfect Window: CLOSED"));
}
//  闪避辅助函数

// 核心闪避逻辑
void ABMWPlayerCharacter::DodgeStarted()
{
	if (!IsAlive()) return;
	/********************************************************************************/
	/* 检查体力                                                    */
	if (AttributeComp && !AttributeComp->TryConsumeActionStamina(25.0f)) return;
	/********************************************************************************/

	// 闪避取消逻辑
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
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Combo Preserved!"));

	}

	// 落地硬直检查
	if (!bCanMove && GetCharacterMovement()->IsMovingOnGround())
	{
		bCanMove = true;
	}

	// 如果还没恢复移动则不能闪避
	if (!bCanMove) return;

	// 计算方向
	EDodgeDirection Direction = CalculateDodgeDirection();
	UAnimMontage* MontageToPlay = nullptr;

	// 分支判断
	if (bIsPerfectDodgeWindow)
	{
		// 完美闪避逻辑
		switch (Direction)
		{
			case EDodgeDirection::Forward:
				MontageToPlay = AM_Dodge_perfect_F; break;
			case EDodgeDirection::Backward:
				MontageToPlay = AM_Dodge_perfect_B; break;
			case EDodgeDirection::Left:
				MontageToPlay = AM_Dodge_perfect_L;  break;
			case EDodgeDirection::Right:
				MontageToPlay = AM_Dodge_perfect_R;  break;
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
		// 普通多段闪避逻辑
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
			// 打断上一个动作
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
// 核心闪避逻辑


// 行走奔跑切换
void ABMWPlayerCharacter::ToggleWalking()
{
	if (!IsAlive()) return;
	// 状态取反
	bIsWalking = !bIsWalking;

	// 根据状态设置速度
	if (bIsWalking)
	{
		// 进入行走模式
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; // 100

		// 调试打印
		/*if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Mode: Walking"));*/
	}
	else
	{
		// 回到奔跑模式
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed; // 550

		// 调试打印
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Mode: Running"));
	}
}


// Dash
void ABMWPlayerCharacter::Dash()
{
	if (!IsAlive()) return;
	// 检查冷却和移动状态
	if (!bCanMove || bIsDashing)
	{
		return;
	}

	// 瞬间提速
	GetCharacterMovement()->MaxWalkSpeed = DashMaxSpeed;

	// 标记状态
	bIsDashing = true;

	// 设置冷却/结束计时器
	// 1.0秒后强制重置状态
	GetWorldTimerManager().SetTimer(TimerHandle_DashCooldown, this, &ABMWPlayerCharacter::StopDashing, DashCooldownTime, false);

	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Speed Boost!"));
}

void ABMWPlayerCharacter::StopDashing()
{
	// 冷却结束，允许下次冲刺
	bIsDashing = false;
}



// attacking
// 玩家按下左键
void ABMWPlayerCharacter::AttackInput()
{
	if (!IsAlive()) return;

	/* 逻辑: 攻击前检查并扣除体力                                              */
	// 如果组件存在且扣除失败(体力不足)，直接返回，不执行攻击
	if (AttributeComp && !AttributeComp->TryConsumeLightAttackStamina())
	{
		return;
	}

	if (!bCanMove && !bIsAttacking) return;

	if (bIsAttacking)
	{
		// 还没到 ContinueCombo 点
		if (!bReadyForNextCombo)
		{
			bHasSavedAttack = true;
		}
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

// 执行攻击动作
void ABMWPlayerCharacter::PerformComboAttack()
{
	if (!IsAlive()) return;

	if (AttackMontages.Num() == 0) return;

	// 确保 Index 不越界
	if (AttackMontages.IsValidIndex(ComboIndex))
	{
		// 锁死状态
		CurrentAttackDamage = 15.0f;
		bIsAttacking = true;
		bCanMove = false; // 攻击时不能移动
		bHasSavedAttack = false; // 消耗掉缓存
		bReadyForNextCombo = false;

		// 播放对应的蒙太奇
		PlayAnimMontage(AttackMontages[ComboIndex]);

		// 准备下一段的下标
		ComboIndex++;
		if (ComboIndex >= AttackMontages.Num())
		{
			ComboIndex = 0;
		}
	}
}

// 连招检测
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

// 重置连招
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
	// 每次挥刀开始时清空受击者名单
	HitActors.Empty();
}

void ABMWPlayerCharacter::EndWeaponCollision()
{
	HitActors.Empty();
}

void ABMWPlayerCharacter::ProcessHit(FHitResult HitResult)
{
	AActor* HitActor = HitResult.GetActor();

	// 如果打到的是自己，忽略
	if (!HitActor || HitActor == this) return;

	// 如果这个人这一刀已经打过了，忽略
	if (HitActors.Contains(HitActor)) return;

	// 记录这个人
	HitActors.Add(HitActor);

	// [修改] 使用 CurrentAttackDamage 变量
	UGameplayStatics::ApplyDamage(
		HitActor,
		CurrentAttackDamage, 
		GetController(),
		this,
		UDamageType::StaticClass()
	);

	AddFocus(0.2f);
	// 应用伤害
	if (GEngine)
	{
		/*FString DebugMsg = FString::Printf(TEXT("Hit Enemy: %s"), *HitActor->GetName());
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, DebugMsg);*/
	}
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


//重击逻辑
void ABMWPlayerCharacter::HeavyAttackStarted()
{
	if (!IsAlive()) return;
	// 状态检查
	if (bIsAttacking || !bCanMove || GetCharacterMovement()->IsFalling()) return;

	/* 逻辑: 蓄力起手检查                                                      */
	// 检查是否有最低限度的体力来开始蓄力
	if (AttributeComp && !AttributeComp->HasEnoughStaminaToStart())
	{
		return; // 体力不足，无法起手
	}

	// 进入蓄力状态
	bIsCharging = true;
	bCanMove = false; // 禁止移动
	bIsAttacking = true; // 视为攻击状态，防止被其他逻辑打断

	// 获取当前棍势等级 (向下取整)
	int32 StartLevel = FMath::FloorToInt(CurrentFocusPoint);

	// 限制
	if (StartLevel > 3) StartLevel = 3;

	// 设置初始时间
	CurrentChargeTime = StartLevel * ChargeTimePerStage;

	// 设置初始阶数
	CurrentChargeStage = StartLevel;

	// 播放蓄力循环动画
	if (AM_Charge_Loop)
	{
		PlayAnimMontage(AM_Charge_Loop);
	}
	/*if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("Charge Start Level: %d"), StartLevel));*/
}


void ABMWPlayerCharacter::HandleChargeRotation(float DeltaTime)
{
	if (!IsAlive()) return;
	// 锁定模式 -> 始终朝向敌人/摄像机前方
	if (bIsLockMode)
	{
		if (Controller)
		{
			FRotator TargetRotation = Controller->GetControlRotation();
			TargetRotation.Pitch = 0.0f;
			TargetRotation.Roll = 0.0f;
			// 平滑转向
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f);
			SetActorRotation(NewRotation);
		}
	}
	// 自由模式 -> 朝向输入方向 (WASD)
	else
	{
		if (!CurrentInputDirection.IsZero() && Controller)
		{
			const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
			const FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			FVector TargetDirection = (ForwardVector * CurrentInputDirection.Y) + (RightVector * CurrentInputDirection.X);
			TargetDirection.Normalize();

			FRotator TargetRot = TargetDirection.Rotation();

			// 平滑转向
			FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.0f); // 5.0 转得慢一点，有蓄力的厚重感
			SetActorRotation(NewRot);
		}
	}
}

void ABMWPlayerCharacter::HeavyAttackReleased()
{
	if (!IsAlive()) return;
	if (!bIsCharging) return;

	// 结束蓄力状态
	bIsCharging = false;
	// bIsAttacking 保持为 true

	// 停止蓄力循环动画
	StopAnimMontage(AM_Charge_Loop);

	// 根据阶数选择蒙太奇
	UAnimMontage* MontageToPlay = nullptr;

	switch (CurrentChargeStage)
	{
		case 0: MontageToPlay = AM_Heavy_Normal; 
			CurrentAttackDamage = 25.0f; 
			break;
		case 1: MontageToPlay = AM_Heavy_Stage1; 
			CurrentAttackDamage = 40.0f; 
			break;
		case 2: MontageToPlay = AM_Heavy_Stage2; 
			CurrentAttackDamage = 55.0f; 
			break;
		case 3: MontageToPlay = AM_Heavy_Stage3; 
			CurrentAttackDamage = 80.0f;
			break;
		default: MontageToPlay = AM_Heavy_Stage3;
			CurrentAttackDamage = 80.0f; 
			break;
	}

	// 播放攻击
	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
		ConsumeAllFocus();
	}
	else
	{
		ResetCombo();
	}
}

//特效改变函数
void ABMWPlayerCharacter::SetWeaponFireColor(FLinearColor NewColor)
{
	if (WeaponFXComp_Top)
	{
		WeaponFXComp_Top->SetNiagaraVariableLinearColor(FString("FireColor"), NewColor);
	}
	if (WeaponFXComp_Bottom)
	{
		WeaponFXComp_Bottom->SetNiagaraVariableLinearColor(FString("FireColor"), NewColor);
	}
}

void ABMWPlayerCharacter::AddFocus(float Amount)
{
	// 增加点数并限制最大值
	CurrentFocusPoint = FMath::Clamp(CurrentFocusPoint + Amount, 0.0f, MaxFocusPoint);

	// 更新特效显示
	UpdateFocusVFX();

	/********************************************************************************/
    /* 同步棍势到UI                                                      */
	OnFocusChanged.Broadcast(CurrentFocusPoint);
	/********************************************************************************/

	// 调试打印
	// if (GEngine) GEngine->AddOnScreenDebugMessage(50, 2.f, FColor::White, FString::Printf(TEXT("Focus: %.2f"), CurrentFocusPoint));
}

void ABMWPlayerCharacter::UpdateFocusVFX()
{
	// 获取当前整数层级 (0, 1, 2, 3, 4)
	int32 CurrentLevel = FMath::FloorToInt(CurrentFocusPoint);


	// 音效播放逻辑
	// 只有当 当前等级 > 上次等级 时才播放升级音效
	if (CurrentLevel > LastFocusLevel)
	{
		USoundBase* SoundToPlay = nullptr;

		switch (CurrentLevel)
		{
			case 1: SoundToPlay = SFX_FocusLevelUp_1; break; // 0->1
			case 2: SoundToPlay = SFX_FocusLevelUp_1; break; // 1->2
			case 3: SoundToPlay = SFX_FocusLevelUp_1; break; // 2->3
			case 4: SoundToPlay = SFX_FocusLevelUp_2; break; // 3->4
			default: break;
		}

		if (SoundToPlay)
		{
			UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, GetActorLocation());
		}
	}

	//更新历史记录供下一帧对比
	LastFocusLevel = CurrentLevel;

	// 0层：关闭特效
	if (CurrentLevel < 1)
	{
		if (WeaponFXComp_Top) WeaponFXComp_Top->Deactivate();
		if (WeaponFXComp_Bottom) WeaponFXComp_Bottom->Deactivate();
		return;
	}

	// >=1层：开启特效
	if (WeaponFXComp_Top) WeaponFXComp_Top->Activate();
	if (WeaponFXComp_Bottom) WeaponFXComp_Bottom->Activate();

	// 根据层级设置颜色
	FLinearColor TargetColor = FocusColor_Level1; // 1级白

	if (CurrentLevel == 2) TargetColor = FocusColor_Level2; // 2级金
	else if (CurrentLevel >= 3) TargetColor = FocusColor_Level3; // 3级/4级红

	SetWeaponFireColor(TargetColor);
}

void ABMWPlayerCharacter::TurnOffFocusVFX()
{
	if (WeaponFXComp_Top) WeaponFXComp_Top->Deactivate();
	if (WeaponFXComp_Bottom) WeaponFXComp_Bottom->Deactivate();
	UpdateFocusVFX();
}

void ABMWPlayerCharacter::PerformSpecialSkill()
{
	if (!IsAlive()) return;
	// 检查条件
	if (bIsAttacking || !bCanMove || GetCharacterMovement()->IsFalling()) return;

	// 检查棍势是否满 4 豆
	if (CurrentFocusPoint < 4.0f)
	{
		/*if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Need 4 Focus Points!"));
		return;*/
	}

	// 播放大招
	if (AM_Heavy_Special)
	{
		CurrentAttackDamage = 120.0f;
		bIsAttacking = true;
		bCanMove = false;

		PlayAnimMontage(AM_Heavy_Special);

		// 清空棍势
		ConsumeAllFocus();
	}
}

//开关镜头移位
void ABMWPlayerCharacter::SetCameraFollowPelvis(bool bEnable)
{
	bFollowPelvisZ = bEnable;
}

void ABMWPlayerCharacter::ConsumeAllFocus()
{
	// 清空棍势数值
	CurrentFocusPoint = 0.0f;

	// 重置等级记录 
	LastFocusLevel = 0;

	/* 清空UI显示                                                        */
	OnFocusChanged.Broadcast(0.0f);
}


/********************************************************************************/
/* 独立逻辑实现区域                                                    */

// 游戏启动逻辑
void ABMWPlayerCharacter::StartGame()
{
	if (MainHUDClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			// 创建HUD
			MainHUDInstance = CreateWidget<UBMWMainHUD>(PC, MainHUDClass);
			if (MainHUDInstance)
			{
				MainHUDInstance->AddToViewport();
				// 初始化连接，传入 self 指针
				MainHUDInstance->InitializeHUD(this);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("=== Wukong Game Started! ==="));
}

// 暂停输入处理
void ABMWPlayerCharacter::OnPauseInput()
{
	if (!PauseMenuInstance && PauseMenuClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PauseMenuInstance = CreateWidget<UBMWPauseMenu>(PC, PauseMenuClass);
		}
	}

	if (PauseMenuInstance)
	{
		PauseMenuInstance->TogglePauseMenu();
	}
}
/********************************************************************************/


void ABMWPlayerCharacter::SetRespawnTransform(FTransform NewTransform)
{
	RespawnPointTransform = NewTransform;
}

void ABMWPlayerCharacter::ResetCharacter()
{
	// 调用基类恢复血量、状态
	Super::ResetCharacter();

	// 玩家传送到记录的重生点
	SetActorTransform(RespawnPointTransform);

	// 恢复输入和移动
	bCanMove = true;
	bIsAttacking = false;
	bIsLockMode = false;

	// 恢复控制权
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetCinematicMode(false, false, false);
		EnableInput(PC);
	}

	// 播放重生特效/音效
}


void ABMWPlayerCharacter::Die()
{
	// 如果已经死了，就不重复执行
	if (!IsAlive()) return;

	Super::Die(); // 执行基类的播放动画、物理逻辑

	// 通知 GameMode
	if (ABMWGameMode* GM = Cast<ABMWGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnPlayerDied(GetController());
	}
}

// 球形检测
AActor* ABMWPlayerCharacter::FindBestLockTarget()
{
	if (!Controller) return nullptr;

	TArray<AActor*> FoundActors;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this); // 忽略自己

	// 获取周围所有的 Pawn (包括小怪)
	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		LockDistance,
		TArray<TEnumAsByte<EObjectTypeQuery>>(), 
		ABMWBaseCharacter::StaticClass(), 
		ActorsToIgnore,
		FoundActors
	);

	AActor* BestTarget = nullptr;
	float ClosestAngle = LockFieldOfView; // 初始设为最大角度限制

	// 获取摄像机的位置和朝向
	FVector CamLoc;
	FRotator CamRot;
	Controller->GetPlayerViewPoint(CamLoc, CamRot);
	FVector CamFwd = CamRot.Vector();

	// 遍历筛选
	for (AActor* Target : FoundActors)
	{
		ABMWBaseCharacter* Enemy = Cast<ABMWBaseCharacter>(Target);

		if (!Enemy || !Enemy->IsAlive() || Enemy->IsHidden()) continue;

		// 计算方向向量
		FVector DirToTarget = (Enemy->GetActorLocation() - CamLoc).GetSafeNormal();

		// 计算角度
		// DotProduct = 1.0 (正前方), 0.0 (侧面), -1.0 (背后)
		float Dot = FVector::DotProduct(CamFwd, DirToTarget);

		// 将点乘转换为角度
		float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));

		if (Angle < ClosestAngle)
		{
			FHitResult Hit;
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				Hit,
				CamLoc,
				Enemy->GetActorLocation(),
				ECC_Visibility
			);

			if (!bHit || Hit.GetActor() == Enemy)
			{
				ClosestAngle = Angle;
				BestTarget = Enemy;
			}
		}
	}

	return BestTarget;
}

float ABMWPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsInvincible)
	{
		return 0.0f; // 伤害归零
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}