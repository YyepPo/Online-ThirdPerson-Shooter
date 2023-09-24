#include "PlayerController/UrrahPlayerController.h"
#include "HUD/UrahhHUD.h"

#include "HUD/PlayerOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

#include "Character/PlayerCharacter.h"

#include "PlayerState/UrahPlayerState.h"

#include "UrrahGameMode.h"
#include "Net/UnrealNetwork.h"

void AUrrahPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UrahHud = Cast<AUrahhHUD>(GetHUD());
}

void AUrrahPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetRemainingTime();
}

void AUrrahPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUrrahPlayerController, MatchState);
}


void AUrrahPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	APlayerCharacter* controllingPawn = Cast<APlayerCharacter>(InPawn);
	if (controllingPawn)
	{
		controllingPawn->SetHealth();
	}
}

void AUrrahPlayerController::SetHUDHealth(float health, float maxHealth)
{
	UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;

	const bool bIsHudValid = UrahHud &&
		UrahHud->playerOverlay &&
		UrahHud->playerOverlay->HealthBar &&
		UrahHud->playerOverlay->HealthAmount;

	if (bIsHudValid)
	{
		const float healthPercentage = health / maxHealth;
		UrahHud->playerOverlay->HealthBar->SetPercent(healthPercentage);

		FString healthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(health), FMath::CeilToInt(maxHealth));
		UrahHud->playerOverlay->HealthAmount->SetText(FText::FromString(healthText));
	}
}

void AUrrahPlayerController::SetHitCrosshair()
{
	ClientSetHitCrosshair();
}

void AUrrahPlayerController::HitTimerFinished()
{
	UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;
	if (UrahHud && UrahHud->playerOverlay && UrahHud->playerOverlay->CrosshairHitImage)
	{
		UrahHud->playerOverlay->CrosshairHitImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AUrrahPlayerController::ClientSetHitCrosshair_Implementation()
{
	UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;
	if (UrahHud && UrahHud->playerOverlay && UrahHud->playerOverlay->CrosshairHitImage)
	{
		UrahHud->playerOverlay->CrosshairHitImage->SetVisibility(ESlateVisibility::Visible);
		GetWorldTimerManager().SetTimer(HitTimerHandle, this, &AUrrahPlayerController::HitTimerFinished, .2f);
	}
}

void AUrrahPlayerController::SetHUDScore(float ScoreAmount)
{
	UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;
	if (UrahHud && UrahHud->playerOverlay && UrahHud->playerOverlay->ScoreAmount)
	{
		FString scoreText = FString::Printf(TEXT("Score: %f"), ScoreAmount);
		UrahHud->playerOverlay->ScoreAmount->SetText(FText::FromString(scoreText));
	}
}

void AUrrahPlayerController::SetHUDDefeatScore(int32 DefeatAmount)
{
	UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;
	if (UrahHud && UrahHud->playerOverlay && UrahHud->playerOverlay->DefeatScoreAmount)
	{
		FString scoreText = FString::Printf(TEXT("Score: %d"), DefeatAmount);
		UrahHud->playerOverlay->DefeatScoreAmount->SetText(FText::FromString(scoreText));
	}
}

void AUrrahPlayerController::SetHUDAmmo(int32 MagAmmo, int32 AmmoCapacity)
{
	UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;
	if (UrahHud && UrahHud->playerOverlay && UrahHud->playerOverlay->AmmoAmount)
	{
		FString scoreText = FString::Printf(TEXT("%d/%d"), MagAmmo,AmmoCapacity);
		UrahHud->playerOverlay->AmmoAmount->SetText(FText::FromString(scoreText));
	}
}

void AUrrahPlayerController::SetHUDRemainingTime(float remainingTime)
{
	UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;
	if (UrahHud && UrahHud->playerOverlay && UrahHud->playerOverlay->RemainingTime)
	{
		int32 Minutes = FMath::FloorToInt(remainingTime / 60.f);
		int32 Seconds = remainingTime - Minutes * 60.f;

		FString time = FString::Printf(TEXT("%02d:%02d"), Minutes,Seconds);
		UrahHud->playerOverlay->RemainingTime->SetText(FText::FromString(time));
	}
}

void AUrrahPlayerController::SetRemainingTime()
{
	uint32 timeSinceGameStarted = MatchTime + GetWorld()->GetTimeSeconds();
	
	//every second update the hud ,instead of doing it every frame
	if (timeSinceGameStarted != timeLeft) 
		SetHUDRemainingTime(timeSinceGameStarted);

	timeLeft = timeSinceGameStarted;
}

void AUrrahPlayerController::OnMatchStateSet(FName NewMatchState)
{
	MatchState = NewMatchState;
	if (MatchState == MatchState::InProgress)
	{
		UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;
		if (UrahHud)
		{
			UrahHud->SetOverlayWidget();
		}
	}
}

void AUrrahPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		UrahHud = (UrahHud == nullptr) ? Cast<AUrahhHUD>(GetHUD()) : UrahHud;
		if (UrahHud)
		{
			UrahHud->SetOverlayWidget();
		}
	}
}
