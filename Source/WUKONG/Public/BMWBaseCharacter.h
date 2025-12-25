// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BMWBaseCharacter.generated.h"

class UBMWAttributeComponent;
class UBehaviorTree;

UCLASS()
class WUKONG_API ABMWBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABMWBaseCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBMWAttributeComponent* AttributeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead;

	virtual void Die();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

public:	
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	UBMWAttributeComponent* GetAttributeComponent() const { return AttributeComp; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;

	UFUNCTION(BlueprintCallable, Category = "State")
	bool IsAlive() const { return !bIsDead; }
};
