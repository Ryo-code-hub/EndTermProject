#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BMWAttributeComponent.h"
#include "BMWMainHUD.generated.h"

class UProgressBar;

UCLASS()
class WUKONG_API UBMWMainHUD : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, UBMWAttributeComponent* OwningComp, float NewHealth, float Delta);
};
