#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "GravityGun.generated.h"

class UPhysicsHandleComponent;
class USplineMeshComponent;
class USoundBase;
class UMaterial;

// Delegate used to trigger cleanup of whatever was spawned for the grab/hover effect like particles, sounds, etc.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEndGrabCleanupDelegate);

/**
 *  Weapon that attracts a Target Object on Right Click and fires the Target Object on Left Click
 *  Subclassed by a Blueprint class 'BP_GravityGun' to enable easy editing of properties
 */
UCLASS(Blueprintable, HideDropdown)
class GRAVITYGUNPROJECT_API AGravityGun : public ABaseWeapon
{
	GENERATED_BODY()

private:
	// Pointer to currently grabbed object
	AActor * CurrentTargetObject = nullptr;

	// Sphere mesh that moves with target object 
	UStaticMeshComponent * HoverSphereComponent = nullptr;

protected:
	// Is the gravity gun currently grabbing something?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	bool bIsGrabbing = false;

	// Current location of the grabbed item
	FVector HandleLocation;

	// How far away to keep the grabbed item from the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	int GrabbedItemDistance = 512;

	// How fast to lerp the grabbed object to its target location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	float HandleLocationLerpAlpha = 0.5;

	// Strength of the force with which a grabbed/targeted object is pushed from primary weapon action 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	float PushForceMagnitude = 100000;

	// Used for the forcefield mesh that encloses the grabbed object while its hovering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	UStaticMesh * HoverMesh;

	// Material for the forcefield mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	UMaterialInterface * HoverMeshMaterial;

	// Used for the forcefield mesh that connects the gravity gun to the grabbed object
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	USplineMeshComponent * SplineMeshComponent;

	// Used for the actual physics interaction behavior with the target object
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	UPhysicsHandleComponent * PhysicsHandleComponent;

	// How much to offset the target object from the handle location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	FVector HandleLocationOffset;

	// How to much to offset the location the target object is grabbed by
	// The object is grabbed by default at its center which makes it completely stable,
	// an increase in offset means the target object will behave as if it swinging on the end of a handle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	FVector HandleGrabOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Gun")
	USoundBase* TargetObjectHoverSound;

	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, BlueprintCallable, Category = "Gravity Gun")
	FEndGrabCleanupDelegate OnEndGrabCleanup;

public:
	AGravityGun();

	// Begin AWeaponBase interface -------
	virtual void OnWeaponDropped() override;
	// End AWeaponBase interface -------

protected:
	// Called when a grab is ending to perform cleanup of spawned sounds, particles etc
	void EndGrabCleanup();
	
	// Trace forward from TraceComponent to find an interactible object
	void TraceForObjectToGrab();

	// Called when currently grabbed object is released
	void ReleaseGrabbedObject();

	// Begin AActor interface -------
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	// End AActor interface -------
	
	// Begin AWeaponBase interface -------
	virtual void PrimaryWeaponAction() override;

	virtual void SecondaryWeaponAction() override;
	// End AWeaponBase interface -------
};
