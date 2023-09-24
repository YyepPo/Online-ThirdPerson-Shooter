#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class URRAH_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ACasing();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* BulletMesh;
	UPROPERTY(EditAnywhere)
		float MinImpulseAmount;
	UPROPERTY(EditAnywhere)
		float MaxImpulseAmount;
	UPROPERTY(EditAnywhere)
		USoundBase* DropSound;
};
