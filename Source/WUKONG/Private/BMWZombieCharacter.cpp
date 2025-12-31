#include "BMWZombieCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

ABMWZombieCharacter::ABMWZombieCharacter()
{
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.0f);
	if (AggroSphereComp)
	{
		AggroSphereComp->SetSphereRadius(400.0f);
	}
}