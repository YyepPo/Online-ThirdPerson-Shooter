#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/UrahhHUD.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class URRAH_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class APlayerCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquippWeapon(AWeapon* equippedWeapon);
	void Reload();
	UFUNCTION(BlueprintCallable)
		void SpawnMagAtPlayersHand();
protected:
	virtual void BeginPlay() override;

	void SetIsAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
		void ServerSetIsAiming(bool bIsAiming);
	
	UFUNCTION()
		void OnRep_WeaponEquipped();

	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& hitTarget);
	UFUNCTION(NetMulticast, Reliable)
		void MultiCastFire(const FVector_NetQuantize& hitTarget);

	UFUNCTION(Client, Reliable)
		void ClientSpawnDefaultWeapon();
	void SpawnDefaultWeapon();
	void LineTraceUnderCrosshair(FHitResult& hitResult);

	UFUNCTION(BlueprintCallable)
		void UpdateAmmoHUD();
private:	
	UPROPERTY()
		class APlayerCharacter* Player;
	class AUrrahPlayerController* Controller;
	
	float deltaTimeTracker;
	float pitchRecoil;
	UPROPERTY(ReplicatedUsing = OnRep_WeaponEquipped)
		AWeapon* EquippedWeapon;
	UPROPERTY(EditAnywhere, Category = "Weapon")
		TSubclassOf<AWeapon> StartingWeaponClass;

	//walk & aiming speed
	UPROPERTY(EditAnywhere, Category = "Move & Walk Speed")
		float walkSpeed;
	UPROPERTY(EditAnywhere, Category = "Move & Walk Speed")
		float aimingWalkSpeed;

	//fire behaviour
	UPROPERTY(Replicated)
		bool bAiming;
	bool bFireButtonPressed;
	void FireBehaviour(bool bPressed);
	FTimerHandle FireTimerHandle;
	UFUNCTION()
		void FireTimerStart();
	UFUNCTION()
		void FireTimerEnd();
	void Fire();
	FVector impactPoint;
	bool bCanFire = true;

	/*
	*HUD AND CROSSHAIR
	*/
	class AUrahhHUD* HUD;
	void SetUpCrosshair(float DeltaTime);
	FCrosshairPackage crosshairPackage;
	float crosshairVelocityFactor;
	float crosshairAirFactor;
	float crosshairAimFactor;
	float crosshairFireFactor;
	FVector hitPoint;	


	/*
	*Reload
	*/
	UPROPERTY(EditAnywhere, Category = "Reload")
		UAnimMontage* ReloadAnim;
};
