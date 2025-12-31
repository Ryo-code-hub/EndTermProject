#include "BMWDeerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

ABMWDeerCharacter::ABMWDeerCharacter()
{
	// 设置移动速度
	GetCharacterMovement()->MaxWalkSpeed = 800.0f;

	// 调整胶囊体
	GetCapsuleComponent()->InitCapsuleSize(40.f, 95.0f);

	// 确保忽略摄像机
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// 调整仇恨范围
	if (AggroSphereComp)
	{
		AggroSphereComp->SetSphereRadius(800.0f);
	}
}