#include "Weapon/ProjectileWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/Projectile.h"
void AProjectileWeapon::Fire(FVector hitResult)
{
	Super::Fire(hitResult);
	
	if (!HasAuthority()) { return; }

	const FName socketName = "MuzzleFlash";
	FTransform muzzleLoc = GetWeaponMesh()->GetSocketTransform(socketName);
	FVector projectileDirection = hitResult - muzzleLoc.GetLocation();

	APawn* instigator = Cast<APawn>(GetOwner());
	if (instigator)
	{
		FActorSpawnParameters actorSpawnParameters;
		actorSpawnParameters.Owner = GetOwner();
		actorSpawnParameters.Instigator = instigator;
		if (ProjectileClass)
		{
			GetWorld()->SpawnActor<AProjectile>(ProjectileClass, muzzleLoc.GetLocation(), projectileDirection.Rotation(), actorSpawnParameters);
		}
	}
}