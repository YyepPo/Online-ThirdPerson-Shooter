#include "HUD/UrahhHUD.h"
#include "HUD/PlayerOverlay.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"

void AUrahhHUD::BeginPlay()
{
	Super::BeginPlay();
}

void AUrahhHUD::SetOverlayWidget()
{
	APlayerController* owningPlayer = GetOwningPlayerController();
	if (!owningPlayer) { return; }

	playerOverlay = CreateWidget<UPlayerOverlay>(owningPlayer, PlayerOverlayClass);
	if (!playerOverlay) { return; }

	playerOverlay->AddToViewport();
	playerOverlay->CrosshairHitImage->SetVisibility(ESlateVisibility::Hidden);
}

void AUrahhHUD::DrawHUD()
{
	Super::DrawHUD();
	
	FVector2D viewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		//get center of the screen
		GEngine->GameViewport->GetViewportSize(viewportSize);
		FVector2D viewportCenter(viewportSize.X / 2.f, viewportSize.Y / 2.f);

		float spreadScale = maxCrosshairSpread * CrosshairPackage.CrosshairSpreadAmount;

		if (CrosshairPackage.crosshairCenter)
		{
			FVector2D spread(0.f,0.f);
			DrawCrosshair(CrosshairPackage.crosshairCenter, viewportCenter, spread,CrosshairPackage.color);
		}
		if (CrosshairPackage.crosshairTop)
		{
			FVector2D spread(0.f, -spreadScale);
			DrawCrosshair(CrosshairPackage.crosshairTop, viewportCenter, spread, CrosshairPackage.color);
		}
		if (CrosshairPackage.crosshairBottom)
		{
			FVector2D spread(0.f,spreadScale);
			DrawCrosshair(CrosshairPackage.crosshairBottom, viewportCenter, spread, CrosshairPackage.color);
		}
		if (CrosshairPackage.crosshairRight)
		{
			FVector2D spread(spreadScale,0.f);
			DrawCrosshair(CrosshairPackage.crosshairRight, viewportCenter, spread, CrosshairPackage.color);
		}
		if (CrosshairPackage.crosshairLeft)
		{
			FVector2D spread(-spreadScale,0.f);
			DrawCrosshair(CrosshairPackage.crosshairLeft, viewportCenter, spread, CrosshairPackage.color);
		}
	}
}

void AUrahhHUD::DrawCrosshair(UTexture2D* CrosshairTexture, FVector2D ViewportCenter, FVector2D CrosshairSpread, FLinearColor color)
{
	if (!CrosshairTexture) { return; }
	//Get texture's width and height,these are for how much we are going to move the texture to center it
	const float width = CrosshairTexture->GetSizeX();
	const float height = CrosshairTexture->GetSizeY();

	//center of the screen (-) left by texture width divided by 2 ,center of the screen (-) up by texture height divided by 2
	const FVector2D TextureDrawPoint(ViewportCenter.X - (width / 2.f) + CrosshairSpread.X,
		ViewportCenter.Y - (height / 2.f) + CrosshairSpread.Y);
	                                                                                             //think this these two 1.f downbelow as alignmet in widgetblueprint
	DrawTexture(CrosshairTexture, TextureDrawPoint.X, TextureDrawPoint.Y, width, height, 0.f, 0.f, 1.f, 1.f, color);
}
