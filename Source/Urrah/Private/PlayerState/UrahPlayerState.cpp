#include "PlayerState/UrahPlayerState.h"
#include "Character/PlayerCharacter.h"
#include "PlayerController/UrrahPlayerController.h"
#include "Net/UnrealNetwork.h"

void AUrahPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUrahPlayerState, defeats);
}

void AUrahPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	PlayerCharacter = (PlayerCharacter == nullptr) ? Cast<APlayerCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = (PlayerController == nullptr) ? Cast<AUrrahPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDScore(Score);
		}
	}
}

void AUrahPlayerState::OnRep_Defeats()
{
	PlayerCharacter = (PlayerCharacter == nullptr) ? Cast<APlayerCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = (PlayerController == nullptr) ? Cast<AUrrahPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			//update defats text
			PlayerController->SetHUDDefeatScore(defeats);
		}
	}
}

void AUrahPlayerState::AddToScore(float ScoreAmount)
{
	Score += ScoreAmount;
	PlayerCharacter = (PlayerCharacter == nullptr) ? Cast<APlayerCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = (PlayerController == nullptr) ? Cast<AUrrahPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			//update score text
			PlayerController->SetHUDScore(Score);
		}
	}
}

void AUrahPlayerState::AddToDefeat(int32 DefeatScoreAmount)
{
	defeats += DefeatScoreAmount;
	PlayerCharacter = (PlayerCharacter == nullptr) ? Cast<APlayerCharacter>(GetPawn()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = (PlayerController == nullptr) ? Cast<AUrrahPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			//update defats text
			PlayerController->SetHUDDefeatScore(defeats);
		}
	}
}