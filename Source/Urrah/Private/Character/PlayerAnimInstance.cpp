#include "Character/PlayerAnimInstance.h"
#include "Character/PlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"

#include "DrawDebugHelpers.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Player = Cast<APlayerCharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	Super::NativeUpdateAnimation(DeltaTimeX);
	if(Player == nullptr)
		Player = Cast<APlayerCharacter>(TryGetPawnOwner());

	if (!Player) { return; }

	FVector	Velocity = Player->GetVelocity();
	Velocity.Z = 0;
	groundSpeed = Velocity.Size();

	UCharacterMovementComponent* MovementComponent = Player->GetCharacterMovement();
	if (MovementComponent)
	{
		bIsInAir = MovementComponent->IsFalling();
		bIsIsAccelerating = MovementComponent->GetCurrentAcceleration().Size() > 0.f ? true : false;
	}

	bIsEquipped = Player->IsWeaponEquipped();
	bIsCrouched = Player->bIsCrouched;
	bIsAiming = Player->IsAiming();
	Weapon = Player->GetPlayerWeapon();
	TurnInPlace = Player->GetTurningInPlace();

	//yaw offset
	FRotator AimRotation = Player->GetControlRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Player->GetVelocity());
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, MovementRotation).Yaw;
	
	//Leaning value
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = Player->GetActorRotation();
	const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float target = delta.Yaw / DeltaTimeX;
	const float interp = FMath::FInterpTo(Leaning, target, DeltaTimeX, 6.f);
	Leaning = FMath::Clamp(interp, -90, 90);

	AO_Yaw = Player->GetAO_Yaw();
	AO_Pitch = Player->GetAO_Pitch();

	if (bIsEquipped && Weapon && Weapon->GetWeaponMesh() && Player->GetMesh())
	{
		LeftHandTransform = Weapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"),ERelativeTransformSpace::RTS_World);
		FVector outPosition;
		FRotator outRotation;
		Player->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, outPosition,outRotation);
		LeftHandTransform.SetLocation(outPosition);
		LeftHandTransform.SetRotation(FQuat(outRotation));

		IsReloading = Player->GetIsReloading();

		if (Player->IsLocallyControlled())
		{
			bIsLocalyControlled = true;
			FTransform rightHandTransform = Player->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			RightHandRotation =	UKismetMathLibrary::FindLookAtRotation(rightHandTransform.GetLocation(), rightHandTransform.GetLocation() + (rightHandTransform.GetLocation() - Player->GetHitPoint()));
			//DrawDebugLine(GetWorld(), rightHandTransform.GetLocation(), rightHandTransform.GetLocation() + (rightHandTransform.GetLocation() - PlayerCharacter->GetHitPoint()) * -1000.f, FColor::Red);
		}
		
		bIsElimmed = Player->IsElimmed();
	}
}

void UPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}