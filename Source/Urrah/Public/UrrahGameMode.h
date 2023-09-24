// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "UrrahGameMode.generated.h"

UCLASS()
class URRAH_API AUrrahGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AUrrahGameMode();
	virtual void Tick(float DeltaSeconds) override;

	virtual void PlayerEliminated(class APlayerCharacter* ElimminatedCharacter, class AUrrahPlayerController* VictimController, AUrrahPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter,AController* ElimmedController);
protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;
private:
	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;
	float timeSinceLevelStarted;
};
