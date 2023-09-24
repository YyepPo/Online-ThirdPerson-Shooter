#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetOverheadText(FString newText)
{
	if (OverheadText)	OverheadText->SetText(FText::FromString(newText));
}

void UOverheadWidget::ShowPlayerNetRole(APawn* Player)
{
	ENetRole netRole = Player->GetLocalRole();
	FString textToDisplay;
	//APlayerState* playerState = Player->GetPlayerState();
	//if (playerState)
	//{
	//	textToDisplay = playerState->GetPlayerName();
	//	SetOverheadText(textToDisplay);

	switch (netRole)
	{
		case ROLE_None:
			textToDisplay = "Network Role NONE";
			break;
		case ROLE_SimulatedProxy:
			textToDisplay = "SimulatedProxy";
			break;
		case ROLE_AutonomousProxy:
			textToDisplay = "AutonomousProxy";
			break;
		case ROLE_Authority:
			textToDisplay = "AuthorityProxy";
			break;
	}
	SetOverheadText(textToDisplay);
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}