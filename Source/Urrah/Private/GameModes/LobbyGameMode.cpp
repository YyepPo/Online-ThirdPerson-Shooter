#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 numberOfPlayers =	GameState.Get()->PlayerArray.Num();
	if (numberOfPlayers == 1)
	{
		if (GetWorld())
		{
			bUseSeamlessTravel = true;
			GetWorld()->ServerTravel(FString("/Game/Maps/Level1?listen"));
		}
		
	}
}