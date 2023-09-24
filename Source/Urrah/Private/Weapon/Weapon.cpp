#include "Weapon/Weapon.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Character/PlayerCharacter.h"

#include "Net/UnrealNetwork.h"

#include "Kismet/GameplayStatics.h"

#include "Weapon/Casing.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("Skeletal Mesh")));
	SetRootComponent(SkeletalMesh);
	SkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(FName(TEXT("Pickup Widget")));
	PickUpWidget->SetupAttachment(GetRootComponent());

	PickUpCollider = CreateDefaultSubobject<UCapsuleComponent>(FName(TEXT("PickUp Collider")));
	PickUpCollider->SetupAttachment(GetRootComponent());
	PickUpCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickUpCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		PickUpCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PickUpCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		PickUpCollider->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnCapsuleBeginOverlap);
		PickUpCollider->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnCapsuleEndOverlap);
	}

}

void AWeapon::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		Player->SetOverlapingWeapon(this);
	}
}

void AWeapon::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		Player->SetOverlapingWeapon(nullptr);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::SetWeaponState(EWeaponState newState)
{
	WeaponState = newState;
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			PickUpCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			ShowPickupWidget(false);
			break;
		case EWeaponState::EWS_Dropped:
			PickUpCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
 		ShowPickupWidget(false);
		break;
	}
}

void AWeapon::ShowPickupWidget(bool bShowPickupWidget)
{
	if (PickUpWidget)
		PickUpWidget->SetVisibility(bShowPickupWidget);
}

USkeletalMeshComponent* AWeapon::GetWeaponMesh()
{
	return SkeletalMesh;
}

void AWeapon::Fire(FVector hitResult)
{
	if (ammoCapacity <= 0 && magAmmo == 0)
	{
		bHasAmmoCapacity = false;
		return;
	}

	if (FireAnim )
		SkeletalMesh->PlayAnimation(FireAnim, false);
	
	if (MuzzleFleshVFX)
	{
		FTransform spawnTransform = SkeletalMesh->GetSocketTransform(FName("MuzzleFlash"));
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFleshVFX, spawnTransform);
	}
	
	const FName ammoEjectSocketName = "AmmoEject";
	FTransform ejectTransform = SkeletalMesh->GetSocketTransform(ammoEjectSocketName, ERelativeTransformSpace::RTS_World);
	if (BulletEjectClass && GetWorld())
		ACasing* shell = GetWorld()->SpawnActor<ACasing>(BulletEjectClass, ejectTransform.GetLocation(), ejectTransform.GetRotation().Rotator());
	
	magAmmo--;
	(magAmmo == 0) ? bHasAmmo = false : bHasAmmo = true;;
}

void AWeapon::WeaponReload()
{
	if (ammoCapacity > 0)
	{
		int32 reloadAmount = 30 - magAmmo;
		ammoCapacity = FMath::Clamp(ammoCapacity - reloadAmount,0,120);
		magAmmo = FMath::Clamp(magAmmo + reloadAmount, 0, 30);
		bHasAmmo = true;
	}
}

void AWeapon::DropMagOnReload()
{
	if (SkeletalMesh)
	{
		const FName clipSocket = "Clip_Bone";
		SkeletalMesh->HideBoneByName(clipSocket, EPhysBodyOp::PBO_None);
		FVector clipSocketTransform = SkeletalMesh->GetSocketTransform(clipSocket).GetLocation();
		if (MagClass)
			GetWorld()->SpawnActor<AActor>(MagClass, clipSocketTransform, FRotator::ZeroRotator);
	}
}
