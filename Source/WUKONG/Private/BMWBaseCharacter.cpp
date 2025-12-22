// Fill out your copyright notice in the Description page of Project Settings.


#include "BMWBaseCharacter.h"

// Sets default values
ABMWBaseCharacter::ABMWBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABMWBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABMWBaseCharacter::Die()
{
}

float ABMWBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return 0.0f;
}

// Called every frame
void ABMWBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABMWBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

