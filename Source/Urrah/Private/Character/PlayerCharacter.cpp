#include "Character/PlayerCharacter.h"
#include "Character/CombatComponent.h"
#include "Character/PlayerAnimInstance.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Weapon/Weapon.h"

#include "Net/UnrealNetwork.h"

#include "GameFramework/PlayerState.h"

#include "Kismet/KismetMathLibrary.h"

#include "PlayerController/UrrahPlayerController.h"
#include "UrrahGameMode.h"

#include "HUD/UrahhHUD.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	InitializeComponents();
}

void APlayerCharacter::InitializeComponents()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(FName(TEXT("Spring Arm")));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->bUsePawnControlRotation = true;
	Camera = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Thirdperson Camera")));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(FName(TEXT("Overhead widget")));
	OverheadWidget->SetupAttachment(GetMesh());

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(FName(TEXT("Combat Component")));
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(FName(TEXT("Dissolve Timeline")));
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)CombatComponent->Player = this;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();	

	SetHealth();

	if (HasAuthority())
		OnTakeAnyDamage.AddDynamic(this, &APlayerCharacter::ReceiveDamage);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	time += DeltaTime;

	AimOffset(DeltaTime);

	if (IsLocallyControlled() && GetCharacterMovement() && GetCharacterMovement()->Velocity != FVector::Zero())
		HeadBobb();

	//zom in out
	if (GetPlayerWeapon() && SpringArm )
	{
		float target;
		(CombatComponent->bAiming) ? target = CombatComponent->EquippedWeapon->GetWeaponZoomInAmount() : target = 0.f;
		SpringArm->SocketOffset.X = FMath::FInterpTo(SpringArm->SocketOffset.X, target, DeltaTime, CombatComponent->EquippedWeapon->GetWeaponZoomInterpSpeed());
	}
	
	if (Camera && IsLocallyControlled())
	{
		float distanceToPlayer = (GetActorLocation() - Camera->GetComponentLocation()).Size();
		if (distanceToPlayer <= acceptableCamDistance)
		{
			GetMesh()->SetVisibility(false);
			if (GetPlayerWeapon() && CombatComponent->EquippedWeapon->GetWeaponMesh())
				CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
				
		}
		else
		{
			GetMesh()->SetVisibility(true);
			if (GetPlayerWeapon() && CombatComponent->EquippedWeapon->GetWeaponMesh())
				CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

FVector APlayerCharacter::GetHitPoint()
{
	if (!CombatComponent) { return FVector::Zero(); }
	return CombatComponent->hitPoint;
}

void APlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{

	health = FMath::Clamp(health - Damage, 0.f,maxHealth);
	PlayHitMontage();
	SetHealthHUD();

	if (health == 0.f)
	{
		AUrrahGameMode* UrrahGameMode = Cast<AUrrahGameMode>(GetWorld()->GetAuthGameMode());
		if (UrrahGameMode)
		{
			UrahController = (UrahController == nullptr) ? Cast<AUrrahPlayerController>(Controller) : UrahController;
			AUrrahPlayerController* AttackerController = Cast<AUrrahPlayerController>(InstigatorController);
				UrrahGameMode->PlayerEliminated(this, UrahController, AttackerController);
		}
	}
}

//this is called on server
void APlayerCharacter::Elim()
{
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &APlayerCharacter::ElimTimerFinished, ElimDelay);
}

void APlayerCharacter::MulticastElim_Implementation()
{
	bElimmed = true;

	DisableMovementAndCollision();

	PlayDeathMontage();

	if (CombatComponent && CombatComponent->EquippedWeapon)
		CombatComponent->EquippedWeapon->SetWeaponState(EWeaponState::EWS_Dropped);
	
	Dissolve();
}

void APlayerCharacter::DisableMovementAndCollision()
{
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (UrahController) DisableInput(UrahController);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacter::Dissolve()
{
	if (DissolveMaterial)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterial, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(FName("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(FName("EmissiveStength"), 200.f);
	}

	StartDissolve();
}

void APlayerCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &APlayerCharacter::UpdateDissolve);
	if (DissolveTimeline && DissolveCurve)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();

	}
}

void APlayerCharacter::UpdateDissolve(float DissolveAmount)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(FName("Dissolve"), DissolveAmount);
	}
}

void APlayerCharacter::ElimTimerFinished()
{
	AUrrahGameMode* urahGameMode = Cast<AUrrahGameMode>(GetWorld()->GetAuthGameMode());
	if (urahGameMode)
	{
		urahGameMode->RequestRespawn(this, Controller);
	}
}

void APlayerCharacter::OnRep_Health()
{
	PlayHitMontage();
	SetHealthHUD();
	if (health == 0.f)
	{
		bElimmed = true;
		PlayDeathMontage();
	}
}

void APlayerCharacter::PlayHitMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && HitMontage)
	{
		animInstance->Montage_Play(HitMontage);
	}
}

void APlayerCharacter::PlayDeathMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && DeathMontage)
	{
		animInstance->Montage_Play(DeathMontage);
	}
}

void APlayerCharacter::SetHealthHUD()
{
	UrahController = (UrahController == nullptr) ? Cast<AUrrahPlayerController>(Controller) : UrahController;
	if (UrahController)
		UrahController->SetHUDHealth(health, maxHealth);
}

void APlayerCharacter::SetHealth()
{
	health = maxHealth;
	SetHealthHUD();
}

void APlayerCharacter::AimOffset(float deltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) { return; }

	FVector	Velocity = GetVelocity();
	Velocity.Z = 0;
	float speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	if (speed == 0 && !bIsInAir)
	{
		FRotator CurrentRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		FRotator deltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, StartingAimRotation);
		AO_Yaw = deltaRotator.Yaw;
		bUseControllerRotationYaw = true;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			turnInterpYaw = AO_Yaw;
		}
		TurnInPlace(deltaTime);
	}
	if (speed > 0 || bIsInAir)
	{
		StartingAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		AO_Yaw = 0;
		bUseControllerRotationYaw = true;
	}
	AO_Pitch = GetBaseAimRotation().Pitch;

	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270,360) to [-90,0)
		FVector2D inRange(270, 360);
		FVector2D outRange(-90, 0);
		AO_Pitch = FMath::GetMappedRangeValueClamped(inRange, outRange,AO_Pitch);
	}
}

void APlayerCharacter::TurnInPlace(float deltaTime)
{
	if (AO_Yaw > 90.f)
		TurningInPlace = ETurningInPlace::ETIP_Right;
	else if (AO_Yaw < -90.f)
		TurningInPlace = ETurningInPlace::ETIP_Left;

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		turnInterpYaw = FMath::FInterpTo(turnInterpYaw, 0.f, deltaTime, 3.f);
		AO_Yaw = turnInterpYaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		}
	}
}

#pragma region InputBindings

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputComponent) { return; }

	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &ThisClass::LookRight);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("PickupWeapon", EInputEvent::IE_Pressed, this, &ThisClass::WeaponPickup);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ThisClass::PlayerCrouch);
	PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &ThisClass::AimHold);
	PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &ThisClass::AimReleased);

	PlayerInputComponent->BindAction("FireHold", EInputEvent::IE_Pressed, this, &ThisClass::FireHold);
	PlayerInputComponent->BindAction("FireReleased", EInputEvent::IE_Released, this, &ThisClass::FireReleased);
	PlayerInputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &ThisClass::Reload);

}

void APlayerCharacter::MoveForward(float inputValue)
{
	FRotator rotation = GetControlRotation();
	FRotator YawRotation(0, rotation.Yaw, 0);
	FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, inputValue);
}

void APlayerCharacter::MoveRight(float inputValue)
{
	FRotator rotation = GetControlRotation();
	FRotator YawRotation(0, rotation.Yaw, 0);
	FVector direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
	AddMovementInput(direction, inputValue);
}

void APlayerCharacter::HeadBobb()
{
	float sinusoidal = bobbAmplitude * FMath::Sin(bobbFrequnce * time);
	if (Camera)
	{
		FVector offSet(0.f, 0.f, sinusoidal);
		Camera->AddLocalOffset(offSet);
	}
}

void APlayerCharacter::LookUp(float inputValue)
{
	AddControllerPitchInput(inputValue);
}

void APlayerCharacter::LookRight(float inputValue)
{
	AddControllerYawInput(inputValue);
}

void APlayerCharacter::Reload()
{
	ServerReload();
}

void APlayerCharacter::OnRep_Reload()
{
	if (bIsReloading)
	{
		if (CombatComponent && CombatComponent->EquippedWeapon)
			CombatComponent->Reload();
	}
}

void APlayerCharacter::ServerReload_Implementation()
{
	if (bHoldingFireButton) { return; }
	bIsReloading = true;
	if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetAmmoCapacity() != 0)
		CombatComponent->Reload();
}

void APlayerCharacter::WeaponPickup()
{
	if (CombatComponent )
		(HasAuthority()) ? CombatComponent->EquippWeapon(OverlappingWeapon) : ServerEquippedButtonPressed();
}
//rpc reliable
void APlayerCharacter::ServerEquippedButtonPressed_Implementation()
{
	if (CombatComponent)CombatComponent->EquippWeapon(OverlappingWeapon);
}

void APlayerCharacter::PlayerCrouch()
{
	(bIsCrouched) ? UnCrouch() : Crouch();
}

void APlayerCharacter::AimHold()
{
	if (CombatComponent)CombatComponent->SetIsAiming(true);
}

void APlayerCharacter::AimReleased()
{
	if (CombatComponent)CombatComponent->SetIsAiming(false);
}

void APlayerCharacter::FireHold()
{
	bHoldingFireButton = true;

	if (bHoldingFireButton && GetPlayerWeapon() && !bIsReloading)
		CombatComponent->FireBehaviour(bHoldingFireButton);

}

void APlayerCharacter::FireReleased()
{
	bHoldingFireButton = false;
	if (!bHoldingFireButton && GetPlayerWeapon())
		CombatComponent->FireBehaviour(bHoldingFireButton);
}

#pragma endregion

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APlayerCharacter, OverlappingWeapon,COND_OwnerOnly);
	DOREPLIFETIME(APlayerCharacter, health);
	DOREPLIFETIME(APlayerCharacter, bIsReloading);
}

bool APlayerCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

void APlayerCharacter::SetOverlapingWeapon(AWeapon* newWeapon)
{
	if (OverlappingWeapon)
		OverlappingWeapon->ShowPickupWidget(false);

	OverlappingWeapon = newWeapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* lastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (lastWeapon)
	{
		lastWeapon->ShowPickupWidget(false);
	}
}

bool APlayerCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

void APlayerCharacter::PlayFireMontage(bool isAiming)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && FireMontage)
	{
		animInstance->Montage_Play(FireMontage);
		FName sectionName = "";
		(isAiming) ? sectionName = "Hip" : sectionName = "Aim";
		animInstance->Montage_JumpToSection(sectionName, FireMontage);
	}
}

AWeapon* APlayerCharacter::GetPlayerWeapon()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
		return CombatComponent->EquippedWeapon;

	return nullptr;
}
