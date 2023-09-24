#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace :uint8
{
	ETIP_Right UMETA(DisplayName = "TurningRight"),
	ETIP_Left UMETA(DisplayName = "TurningLeft"),
	ETIP_NotTurning UMETA(DisplayName = "NotTurning"),

	ETIP_MAX UMETA(DisplayName = "DefaultMax")
};