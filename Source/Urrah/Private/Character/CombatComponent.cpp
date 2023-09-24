#include "Character/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "Character/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "PlayerController/UrrahPlayerController.h"
#include "Character/InteractWithCrosshairInterface.h"
#include "TimerManager.h"
#include "Camera/CameraShakeBase.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();	
	if(Player && Player->HasAuthority())
		SpawnDefaultWeapon();

	if (Player)
		Player->GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

void UCombatComponent::SpawnDefaultWeapon()
{
	if (StartingWeaponClass)
	{
		AWeapon* startingWeapon = GetWorld()->SpawnActor<AWeapon>(StartingWeaponClass);
		EquippWeapon(startingWeapon);
	}
}

void UCombatComponent::ClientSpawnDefaultWeapon_Implementation()
{
	if (Player)
		SpawnDefaultWeapon();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	deltaTimeTracker += DeltaTime;

	if (Player && Player->IsLocallyControlled()) 
	{
		FHitResult hitResult;
		LineTraceUnderCrosshair(hitResult);
		hitPoint = hitResult.ImpactPoint;
		SetUpCrosshair(DeltaTime);		
	}
}
 
void UCombatComponent::SetUpCrosshair(float DeltaTime)
{
	if (!Player || !Player->Controller) { return; }
	Controller = Controller == nullptr ? Cast<AUrrahPlayerController>(Player->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<AUrahhHUD>(Controller->GetHUD()): HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				crosshairPackage.crosshairCenter = EquippedWeapon->CenterCrosshair;
				crosshairPackage.crosshairTop = EquippedWeapon->UpCrosshair;
				crosshairPackage.crosshairBottom = EquippedWeapon->DownCrosshair;
				crosshairPackage.crosshairRight = EquippedWeapon->RightCrosshair;
				crosshairPackage.crosshairLeft = EquippedWeapon->LeftCrosshair;
			}
			else
			{
				crosshairPackage.crosshairCenter = nullptr;
				crosshairPackage.crosshairTop = nullptr;
				crosshairPackage.crosshairBottom = nullptr;
				crosshairPackage.crosshairRight = nullptr;
				crosshairPackage.crosshairLeft = nullptr;
			}

			//clamp range values [0,players max speed amount] to [0,1]
			FVector2D walkRange(0, Player->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D walkOutRange(.2f, 1.f);

			//get players velocity
			FVector velocity = Player->GetVelocity();
			velocity.Z = 0;

			crosshairVelocityFactor = FMath::GetMappedRangeValueClamped(walkRange, walkOutRange, velocity.Size());
			
			//jump factor
			if (Player->GetCharacterMovement()->IsFalling())
				crosshairAirFactor = FMath::FInterpTo(crosshairAirFactor, 2.25f, DeltaTime, 2.25f);
			else
				crosshairAirFactor = FMath::FInterpTo(crosshairAirFactor, 0.f, DeltaTime, 20.f);

			//aim factor
			if (bAiming)
				crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, -.3f , DeltaTime, 4.f);
			else
				crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, 0.f , DeltaTime, 4.f);

			crosshairFireFactor = FMath::FInterpTo(crosshairFireFactor, 0.f , DeltaTime, 4.f);
			
			crosshairPackage.CrosshairSpreadAmount = 
				crosshairVelocityFactor + crosshairAirFactor + crosshairAimFactor + crosshairFireFactor;
									
			HUD->SetCrosshairPackage(crosshairPackage);
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::EquippWeapon(AWeapon* equippedWeapon)
{
	if (equippedWeapon == nullptr || Player == nullptr) { return; }
	EquippedWeapon = equippedWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* handSocket = Player->GetMesh()->GetSocketByName(FName(TEXT("WeaponSocket")));
	if (handSocket) handSocket->AttachActor(EquippedWeapon, Player->GetMesh());

	EquippedWeapon->SetOwner(Player);
	Player->GetCharacterMovement()->bOrientRotationToMovement = false;
	Player->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_WeaponEquipped()
{
	if (EquippedWeapon && Player)
	{
		Player->GetCharacterMovement()->bOrientRotationToMovement = false;
		Player->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::SetIsAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetIsAiming(bIsAiming);
	if (Player)	Player->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? aimingWalkSpeed : walkSpeed;
}

void UCombatComponent::ServerSetIsAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Player)	Player->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? aimingWalkSpeed : walkSpeed;
}

void UCombatComponent::FireBehaviour(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed) Fire();

}

void UCombatComponent::FireTimerStart()
{
	if (Player && EquippedWeapon)
		Player->GetWorldTimerManager().SetTimer(FireTimerHandle, this,&UCombatComponent::FireTimerEnd,EquippedWeapon->GetFireRate());

}

void UCombatComponent::FireTimerEnd()
{
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon && EquippedWeapon->IsAutomatic())
		Fire();
}

void UCombatComponent::Fire()
{
	if (bCanFire)
	{
		bCanFire = false;

		ServerFire(hitPoint);
		FireTimerStart();

		if (Player && Controller && EquippedWeapon && EquippedWeapon->GetHasMagAmmo())
		{
			Player->AddControllerPitchInput(EquippedWeapon->GetPitchRecoil());
			Player->AddControllerYawInput(EquippedWeapon->GetYawRecoil());
			Controller->ClientStartCameraShake(EquippedWeapon->ShootCameraShake);
			crosshairFireFactor += .2f;
			Controller = Controller == nullptr ? Cast<AUrrahPlayerController>(Player->Controller) : Controller;
			if (Controller)Controller->SetHUDAmmo(EquippedWeapon->GetMagAmmo(), EquippedWeapon->GetAmmoCapacity());
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& traceHitTarget)
{
	MultiCastFire(traceHitTarget);
}

void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& traceHitTarget)
{
	if (Player && EquippedWeapon && EquippedWeapon->GetHasMagAmmo())
	{
		Player->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(traceHitTarget);
	}
}

void UCombatComponent::LineTraceUnderCrosshair(FHitResult& hitResult)
{
	FVector2D viewportSize;
	if (GEngine && GEngine->GameViewport)
		GEngine->GameViewport->GetViewportSize(viewportSize);
	FVector2D crosshairPossition(viewportSize.X / 2,viewportSize.Y / 2);
	FVector crosshairWorldPosition;
	FVector crosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		crosshairPossition,
		crosshairWorldPosition,
		crosshairWorldDirection);
	if (bScreenToWorld)
	{
		FVector traceStartPos = crosshairWorldPosition;
	
		if (Player)
		{
			float distanceToPlayer = (Player->GetActorLocation() - traceStartPos).Size();
			traceStartPos += crosshairWorldDirection * (distanceToPlayer + 100.f);
		}

		FVector traceEndPos = (traceStartPos + crosshairWorldDirection * 8000.f);

		bool bHasHit = GetWorld()->LineTraceSingleByChannel(hitResult, traceStartPos, traceEndPos, ECollisionChannel::ECC_Visibility);
		if (bHasHit)
		{
			if (!hitResult.GetActor()) {return;}
			
			if (hitResult.GetActor()->Implements<UInteractWithCrosshairInterface>()) crosshairPackage.color = FLinearColor::Red;
			else crosshairPackage.color = FLinearColor::White;
		}
	}

}

void UCombatComponent::Reload()
{
	if (EquippedWeapon) EquippedWeapon->WeaponReload();
	if (Player && Player->GetMesh())
	{
		UAnimInstance* animInstance = Player->GetMesh()->GetAnimInstance();
		if (animInstance && ReloadAnim) animInstance->Montage_Play(ReloadAnim);
	}
}

void UCombatComponent::UpdateAmmoHUD()
{
	if (Controller && EquippedWeapon)
		Controller->SetHUDAmmo(EquippedWeapon->GetMagAmmo(), EquippedWeapon->GetAmmoCapacity());
}

void UCombatComponent::SpawnMagAtPlayersHand()
{
	if (Player && Player->GetMesh() && EquippedWeapon)
	{
		AActor* mag = GetWorld()->SpawnActor(EquippedWeapon->GetMagClass());
		if (mag)
		{
			mag->DisableComponentsSimulatePhysics();
			const USkeletalMeshSocket* handLSocket = Player->GetMesh()->GetSocketByName(FName(TEXT("MagazineSocket")));
			if(handLSocket)
				handLSocket->AttachActor(mag, Player->GetMesh());
		}
	}
}
