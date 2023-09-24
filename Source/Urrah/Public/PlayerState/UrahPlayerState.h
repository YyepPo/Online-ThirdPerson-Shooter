// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "UrahPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class URRAH_API AUrahPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual void OnRep_Score() override;
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
		int32 defeats = 0;
	UFUNCTION()
		void OnRep_Defeats();
	void AddToScore(float ScoreAmount);
	void AddToDefeat(int32 DefeatScoreAmount);
private:
	class APlayerCharacter* PlayerCharacter;
	class AUrrahPlayerController* PlayerController;
};
