// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileAR.generated.h"

/**
 * 
 */
UCLASS()
class URRAH_API AProjectileAR : public AProjectile
{
	GENERATED_BODY()

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	class ACharacter* ownerCharacter;
	class AController* controller;
	class AUrrahPlayerController* OwningPlayerController;
};
