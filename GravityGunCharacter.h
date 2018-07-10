#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GravityGunProject.generated.h"

class UInputComponent;
class ABaseWeapon;

/* Character class that ties together input, camera and collision for the player */
UCLASS(config=Game)
class AGravityGunCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	/* Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/* First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/* Points to the actual instance of the weapon the player is currently carrying */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Weapon)
	ABaseWeapon * WeaponActor;

	/* Points to any actor the player currently overlaps and is used to check if it can be picked up */
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Collision)
	AActor * CurrentlyOverlappedActor = nullptr;

public:
	/* Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/* Used to select the weapon class in the editor to avoid direct content references in C++ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	TSubclassOf<ABaseWeapon> WeaponClass;

	/* Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/* AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	class UAnimMontage* FireAnimation;

private:
	void PickupWeapon(ABaseWeapon * newWeapon);

	void DropWeapon();

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void BeginPlay() override;

	// Function called when collider begins overlaps with an object
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult &SweepResult);

	// Function called when collider ends overlap with an object
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex);

	/* Bound to Keyboard 'E'/Gamepad Top Face Button */
	void OnInteract();

	/* Bound to left click/left trigger */
	void OnWeaponPrimary();

	/* Bound to right click/right trigger */
	void OnWeaponSecondary();

	/* Handles moving forward/backward */
	void MoveForward(float Val);

	/* Handles strafing movement, left and right */
	void MoveRight(float Val);

	/*
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/*
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	AGravityGunCharacter();
	/* Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/* Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

