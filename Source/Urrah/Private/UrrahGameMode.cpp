#include "UrrahGameMode.h"

#include "Character/PlayerCharacter.h"
#include "PlayerController/UrrahPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

#include "PlayerState/UrahPlayerState.h"

AUrrahGameMode::AUrrahGameMode()
{
	bDelayedStart = true;
}

void AUrrahGameMode::BeginPlay()
{
	Super::BeginPlay();
	timeSinceLevelStarted = GetWorld()->GetTimeSeconds();
}

void AUrrahGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (MatchState == MatchState::WaitingToStart)
	{
		float timeLeft = WarmupTime - GetWorld()->GetTimeSeconds() + timeSinceLevelStarted;
		if (timeLeft <= 0)
			StartMatch();
	}
}

void AUrrahGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AUrrahPlayerController* playerController = Cast<AUrrahPlayerController>(*It);
		if (playerController)
			playerController->OnMatchStateSet(MatchState);
	}
}

void AUrrahGameMode::PlayerEliminated(APlayerCharacter* ElimminatedCharacter, AUrrahPlayerController* VictimController, AUrrahPlayerController* AttackerController)
{
	if (ElimminatedCharacter)
		ElimminatedCharacter->Elim();

	AUrahPlayerState* VictimPlayerState = (VictimController) ? Cast<AUrahPlayerState>(VictimController->PlayerState) : nullptr;
	AUrahPlayerState* AttackerPlayerState = (AttackerController) ? Cast<AUrahPlayerState>(AttackerController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
		AttackerPlayerState->AddToScore(1.f);
	
	if (VictimPlayerState && VictimPlayerState != AttackerPlayerState)
		VictimPlayerState->AddToDefeat(1);
}

void AUrrahGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset(); // Reset() detaches the character from the controller,also unposes the controller so later on we can poses to another one
		ElimmedCharacter->Destroy();
	}

	if (ElimmedController)
	{
		TArray<AActor*> PlayerStartPositions;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartPositions);
		int32 randomStart = FMath::RandRange(0, PlayerStartPositions.Num() - 1);
		AActor* PlayerStart = PlayerStartPositions[randomStart];
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStart);
	}
}