#include "Weapon/Casing.h"
#include "Kismet/GameplayStatics.h"
ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;
	
	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Bulles Mesh")));
	SetRootComponent(BulletMesh);
	BulletMesh->SetNotifyRigidBodyCollision(true);
}

void ACasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	BulletMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);

	float randomImpulse = FMath::RandRange(MinImpulseAmount, MaxImpulseAmount);
	BulletMesh->AddImpulse(-GetActorForwardVector() * randomImpulse);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (DropSound)
		UGameplayStatics::PlaySoundAtLocation(this, DropSound, GetActorLocation());
	Destroy();

}
