#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

class USkeletalMeshComponent;
class UParticleSystem;

UCLASS(Abstract)
class GRAVITYGUNPROJECT_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	// Allows weapon mesh to be set in editor 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int WeaponRange = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bShouldDebugTraces = false;

protected:
	// If the weapon behavior requires a trace, this component is set by the weapon owner to specify the starting location and direction for the trace 
	USceneComponent * TraceComponent;

	/* These are triggered after their respective actions are performed*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USoundBase* PrimaryActionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UParticleSystem * PrimaryActionParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USoundBase* SecondaryActionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UParticleSystem * SecondaryActionParticleSystem;

public:
	ABaseWeapon();

	// Called when a weapon is picked up by a character
	virtual void OnWeaponPickedUp() {};

	// Called when a weapon is dropped by a character
	virtual void OnWeaponDropped() {};
	
	// Meant to be overridden by subclasses for functionality on Left Click/Trigger
	virtual void PrimaryWeaponAction();

	// Meant to be overridden by subclasses for functionality on Right Click/Trigger
	virtual void SecondaryWeaponAction();

	FORCEINLINE void SetTraceComponent(USceneComponent * newTraceComponent) { TraceComponent = newTraceComponent; }

	FORCEINLINE USceneComponent * GetTraceComponent() { return TraceComponent; }

protected:

	// Begin AActor interface ------
	virtual void BeginPlay() override;
	// End AActor interface ------
	
};
