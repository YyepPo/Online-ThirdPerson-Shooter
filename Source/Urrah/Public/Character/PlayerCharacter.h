#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterTypes/TurningInPlace.h"
#include "InteractWithCrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "PlayerCharacter.generated.h"

class AWeapon;

UCLASS()
class URRAH_API APlayerCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void Elim();
	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim();
protected:
	void AimOffset(float deltaTime);
	UFUNCTION()
		void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void PlayHitMontage();
	void PlayDeathMontage();

	void DisableMovementAndCollision();
	void Dissolve();
private:	

	class AUrrahPlayerController* UrahController;

#pragma region Input bindings
	//movement
	void MoveForward(float inputValue);
	void MoveRight(float inputValue);
	void LookUp(float inputValue);
	void LookRight(float inputValue);

	void WeaponPickup();
	void PlayerCrouch();

	void AimHold();
	void AimReleased();
	
	void FireHold();
	void FireReleased();
	void Reload();
#pragma endregion

#pragma region Components
	void InitializeComponents();

	UPROPERTY(VisibleAnywhere)
		class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere)
		class UCameraComponent* Camera;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
		class UWidgetComponent* OverheadWidget;
	UPROPERTY(VisibleAnywhere)
		class UCombatComponent* CombatComponent;
#pragma endregion

	/*
	// Head Bobb
	*/
	void HeadBobb();
	UPROPERTY(EditAnywhere, Category = "Head Bobb")
		float bobbAmplitude;
	UPROPERTY(EditAnywhere, Category = "Head Bobb")
		float bobbFrequnce;
	float time;


	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		AWeapon* OverlappingWeapon;
	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* lastWeapon);

	UFUNCTION(Server,Reliable)
		void ServerEquippedButtonPressed();

	/// <summary>
	/// Aim offset 
	/// </summary>
	float AO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	void TurnInPlace(float deltaTime);
	float turnInterpYaw;

	bool bHoldingFireButton;
	UPROPERTY(EditAnywhere, Category = "Combat")
		class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere)
		float acceptableCamDistance;

	UPROPERTY(EditAnywhere = "Montage")
		UAnimMontage* HitMontage;
	UPROPERTY(EditAnywhere = "Montage")
		UAnimMontage* DeathMontage;

	/// <summary>
	/// Health
	/// </summary>
	UFUNCTION()
		void OnRep_Health();
	void SetHealthHUD();
	void ElimTimerFinished();

	UPROPERTY(EditAnywhere, Category = "Health")
		float maxHealth = 100.f;
	UPROPERTY(VisibleAnywhere, Category = "Health", ReplicatedUsing = OnRep_Health)
		float health;
	
	bool bElimmed = false;
	FTimerHandle ElimTimer;
	UPROPERTY(EditAnywhere, Category = "Health")
		float ElimDelay = 3.f;

	/*
	* Dissolve Timeline
	*/
	UPROPERTY(VisibleAnywhere,Category = "Health|Dissolve")
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;
	UPROPERTY(EditAnywhere,Category = "Health|Dissolve")
		UCurveFloat* DissolveCurve;

	void StartDissolve();
	UFUNCTION()
		void UpdateDissolve(float DissolveAmount);

	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	UPROPERTY(EditAnywhere, Category = "Health|Dissolve")
		UMaterialInstance* DissolveMaterial;

	UPROPERTY(ReplicatedUsing = OnRep_Reload)
	bool bIsReloading;
	UFUNCTION()
		void OnRep_Reload();
	UPROPERTY(EditAnywhere, Category = "Health|Dissolve")
		UAnimMontage* ReloadMontage;
	UFUNCTION(Server, Reliable)
		void ServerReload();
public:
	bool IsWeaponEquipped();
	UFUNCTION(BlueprintCallable)
		AWeapon* GetPlayerWeapon();
	void SetOverlapingWeapon(AWeapon* newWeapon);

	bool IsAiming();
	void PlayFireMontage(bool isAiming);

	FORCEINLINE float  GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float  GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace  GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetMaxHealth() const { return maxHealth; }
	FORCEINLINE float GetCurrentHealth() const { return health; }
	void SetHealth();

	FVector GetHitPoint();

	FORCEINLINE bool GetIsReloading() const { return bIsReloading; }
	UFUNCTION(BlueprintCallable)
		FORCEINLINE void SetIsReloading(bool newIsReloading) { bIsReloading = newIsReloading; }
};
