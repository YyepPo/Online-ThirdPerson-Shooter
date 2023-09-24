#include "Weapon/ProjectileAR.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "PlayerController/UrrahPlayerController.h"
#include "Components/SlateWrapperTypes.h"

void AProjectileAR::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ownerCharacter = Cast<ACharacter>(GetOwner());
	if (ownerCharacter)
	{
		controller = ownerCharacter->GetController();
		if (!controller) { return; }
		
		OwningPlayerController = Cast<AUrrahPlayerController>(ownerCharacter->GetController());
		if (OwningPlayerController && OtherActor->ActorHasTag("Player"))
			OwningPlayerController->SetHitCrosshair();
		
		UGameplayStatics::ApplyDamage(OtherActor, damage, controller, this, UDamageType::StaticClass());

	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

