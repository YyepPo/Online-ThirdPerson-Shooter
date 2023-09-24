// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerOverlay.generated.h"

/**
 * 
 */
UCLASS()
class URRAH_API UPlayerOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
		class UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
		class UTextBlock* HealthAmount;
	UPROPERTY(meta = (BindWidget))
		class UImage* CrosshairHitImage;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* ScoreAmount;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* DefeatScoreAmount;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* AmmoAmount;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* RemainingTime;
};
