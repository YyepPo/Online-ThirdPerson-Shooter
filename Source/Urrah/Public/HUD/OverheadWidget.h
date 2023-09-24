// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class URRAH_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

public:
	void SetOverheadText(FString newText);
	UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* Player);
private:
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* OverheadText;

};
