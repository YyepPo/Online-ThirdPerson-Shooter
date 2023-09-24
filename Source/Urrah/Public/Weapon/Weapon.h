// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DislayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "MAX")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Ar UMETA(DisplayName = "Ar"),
	EWT_Shotgun UMETA(DislayName = "Shotgun"),
	EWT_RocketLauncher UMETA(DislayName = "RocketLauncher"),
	EWT_None UMETA(DisplayName = "None"),
	EWT_MAX UMETA(DisplayName = "MAX")
};

UCLASS()
class URRAH_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void ShowPickupWidget(bool bShowPickupWidget);
	UFUNCTION()
		void SetWeaponState(EWeaponState newState);
	UFUNCTION(BlueprintCallable,BlueprintPure)
		USkeletalMeshComponent* GetWeaponMesh();
	virtual void Fire(FVector hitResult);

	UPROPERTY(EditAnywhere, Category = "Crosshair")
		class UTexture2D* CenterCrosshair;
	UPROPERTY(EditAnywhere,Category = "Crosshair")
		UTexture2D* UpCrosshair;
	UPROPERTY(EditAnywhere,Category = "Crosshair")
		UTexture2D* DownCrosshair;
	UPROPERTY(EditAnywhere,Category = "Crosshair")
		UTexture2D* RightCrosshair;
	UPROPERTY(EditAnywhere, Category = "Crosshair")
		UTexture2D* LeftCrosshair;
	UPROPERTY(EditAnywhere, Category = "Crosshair")
		class UImage* HitCrosshair;

	/// <summary>
	/// Weapon Recoil
	/// </summary>
	UPROPERTY(EditAnywhere, Category = "Fire|Recoil")
		float minPitchRecoil;
	UPROPERTY(EditAnywhere, Category = "Fire|Recoil")
		float maxPitchRecoil;
	UPROPERTY(EditAnywhere, Category = "Fire|Recoil")
		float minYawRecoil;
	UPROPERTY(EditAnywhere, Category = "Fire|Recoil")
		float maxYawRecoil;
	UPROPERTY(EditAnywhere, Category = "Fire|Recoil")
		TSubclassOf<class UCameraShakeBase> ShootCameraShake;

	void WeaponReload();
	UFUNCTION(BlueprintCallable)
		void DropMagOnReload();
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
		virtual void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		virtual void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:	
 
	UPROPERTY(EditAnywhere)
		USkeletalMeshComponent* SkeletalMesh;
	UPROPERTY(EditAnywhere)
		class UCapsuleComponent* PickUpCollider;
	UPROPERTY(EditAnywhere)
		class UWidgetComponent* PickUpWidget;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState ,VisibleAnywhere, Category = "Weapon State")
		EWeaponState WeaponState = EWeaponState::EWS_Initial;
	UFUNCTION()
		void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere)
		EWeaponType WeaponType = EWeaponType::EWT_None;


	UPROPERTY(EditAnywhere, Category = "Fire | Automatic")
		bool bIsAutomatic;
	UPROPERTY(EditAnywhere, Category = "Fire | Automatic")
		float minFireRate;
	UPROPERTY(EditAnywhere, Category = "Fire | Automatic")
		float maxFireRate;
	UPROPERTY(EditAnywhere,Category = "Fire")
		UAnimationAsset* FireAnim;
	UPROPERTY(EditAnywhere, Category = "Fire")
		UParticleSystem* MuzzleFleshVFX;
	UPROPERTY(EditAnywhere, Category = "Fire")
		TSubclassOf<class ACasing> BulletEjectClass;
	UPROPERTY(EditAnywhere, Category = "Fire")
		float weaponZoomInAmount;
	UPROPERTY(EditAnywhere, Category = "Fire")
		float weaponZoomInterpSpeed;


/// <summary>
/// Ammo
/// </summary>
	UPROPERTY(EditAnywhere, Category = "Ammo")
		int32 magAmmo = 30;
	UPROPERTY(EditAnywhere, Category = "Ammo")
		int32 ammoCapacity ;
	UPROPERTY(VisibleAnywhere, Category = "Ammo")
		bool bHasAmmo = true;
	UPROPERTY(VisibleAnywhere, Category = "Ammo")
		bool bHasAmmoCapacity = true;
	UPROPERTY(EditAnywhere, Category = "Ammo")
		TSubclassOf<AActor> MagClass;
public:
	FORCEINLINE float GetWeaponZoomInAmount() const { return weaponZoomInAmount; }
	FORCEINLINE float GetWeaponZoomInterpSpeed() const { return weaponZoomInterpSpeed; }
	FORCEINLINE bool IsAutomatic() const { return bIsAutomatic; }
	FORCEINLINE float GetFireRate() const { return FMath::RandRange(minFireRate, maxFireRate); }
	FORCEINLINE float GetPitchRecoil() const { return FMath::RandRange(minPitchRecoil, maxPitchRecoil); }
	FORCEINLINE float GetYawRecoil() const { return FMath::RandRange(minYawRecoil, maxYawRecoil); }

	FORCEINLINE float GetMagAmmo() const { return magAmmo; }
	FORCEINLINE float GetAmmoCapacity() const { return ammoCapacity; }
	FORCEINLINE bool GetHasMagAmmo() const { return bHasAmmo; }
	FORCEINLINE bool GetHasAmmoCapacity() const { return bHasAmmoCapacity; }
	FORCEINLINE TSubclassOf<AActor> GetMagClass() const { return MagClass; }
};
