#include "Weapon/Projectile.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

#include "DrawDebugHelpers.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Character/PlayerCharacter.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(FName(TEXT("Box Collider")));
	SetRootComponent(BoxCollider);
	BoxCollider->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BoxCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	BoxCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	BoxCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName(TEXT("Projectile Movement")));

}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
		BoxCollider->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	
	if (TrailVFX)
		ProjectileTrailVFXComponent = UGameplayStatics::SpawnEmitterAttached(TrailVFX, BoxCollider, FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ServerImpactSound(OtherActor);
	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	if (ImpactParticle) UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
}

void AProjectile::ServerImpactSound_Implementation(AActor* DamagedActor)
{
	if (DamagedActor)
	{
		USoundBase* impactSound = nullptr;
		if (DamagedActor->ActorHasTag("Player"))
			impactSound = BodyImpactSound;
		else
			impactSound = SurfaceImpactSound;

		if (impactSound)
			UGameplayStatics::PlaySoundAtLocation(this, impactSound, DamagedActor->GetActorLocation());
	}
}
