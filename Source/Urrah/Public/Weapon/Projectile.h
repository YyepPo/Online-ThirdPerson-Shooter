// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class URRAH_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
		float damage;

	UFUNCTION(Client, Unreliable)
		void ServerImpactSound(AActor* DamagedActor);
private:	
	UPROPERTY(VisibleAnywhere)
		class UBoxComponent* BoxCollider;
	UPROPERTY(VisibleAnywhere)
		class UProjectileMovementComponent* ProjectileMovement;
		class UParticleSystemComponent* ProjectileTrailVFXComponent;
	UPROPERTY(EditAnywhere)
		class UParticleSystem* TrailVFX;
	UPROPERTY(EditAnywhere)
		UParticleSystem* ImpactParticle;
	UPROPERTY(EditAnywhere)
		USoundBase* BodyImpactSound;
	UPROPERTY(EditAnywhere)
		USoundBase* SurfaceImpactSound;
};
