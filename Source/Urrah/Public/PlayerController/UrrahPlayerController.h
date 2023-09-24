#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UrrahPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class URRAH_API AUrrahPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void SetHUDHealth(float health, float maxHealth);
	void SetHUDScore(float ScoreAmount);
	void SetHUDDefeatScore(int32 DefeatAmount);
	void SetHUDAmmo(int32 MagAmmo,int32 AmmoCapacity);
	void SetHUDRemainingTime(float remainingTime);
	void SetHitCrosshair();

	void OnMatchStateSet(FName NewMatchState);
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	UFUNCTION(Client, Unreliable)
		void ClientSetHitCrosshair();
	void SetRemainingTime();

	UFUNCTION()
		void OnRep_MatchState();
private:
	class AUrahhHUD* UrahHud;

	FTimerHandle HitTimerHandle;
	void HitTimerFinished();
	
	float MatchTime = 120.f;
	uint32 timeLeft = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;
};
