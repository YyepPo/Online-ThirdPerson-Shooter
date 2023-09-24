// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UrahhHUD.generated.h"

USTRUCT(BlueprintType)
struct FCrosshairPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* crosshairCenter;
	 UTexture2D* crosshairTop;
	 UTexture2D* crosshairBottom;
	 UTexture2D* crosshairRight;
	 UTexture2D* crosshairLeft;
	 float CrosshairSpreadAmount;
	 FLinearColor color;
};

UCLASS()
class URRAH_API AUrahhHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	class UPlayerOverlay* playerOverlay;
	void SetOverlayWidget();

	FORCEINLINE void SetCrosshairPackage(FCrosshairPackage NewCrosshairPackage) { CrosshairPackage = NewCrosshairPackage; }
private:
	UPROPERTY(EditAnywhere, Category = "Player Stat")
		TSubclassOf<class UUserWidget> PlayerOverlayClass;

	void DrawCrosshair(UTexture2D* CrosshairTexture, FVector2D ViewportCenter,FVector2D CrosshairSpread,FLinearColor color);
	FCrosshairPackage CrosshairPackage;
	UPROPERTY(EditAnywhere)
		float maxCrosshairSpread = 16.f;
};
