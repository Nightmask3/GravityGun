#include "GravityGunCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "BaseWeapon.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SubclassOf.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AGravityGunCharacter

AGravityGunCharacter::AGravityGunCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	// Bind the begin and end overlap collision delegates to our response functions
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AGravityGunCharacter::BeginOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AGravityGunCharacter::EndOverlap);

	// Set turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

}

void AGravityGunCharacter::PickupWeapon(ABaseWeapon * newWeapon)
{
	// Null out overlapped actor
	CurrentlyOverlappedActor = nullptr;

	WeaponActor = newWeapon;
	WeaponActor->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	// Set the camera as the trace component
	WeaponActor->SetTraceComponent(FirstPersonCameraComponent);
	WeaponActor->WeaponMesh->SetSimulatePhysics(false);
	WeaponActor->OnWeaponPickedUp();
}

void AGravityGunCharacter::DropWeapon()
{
	WeaponActor->OnWeaponDropped();
	WeaponActor->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	WeaponActor->WeaponMesh->SetSimulatePhysics(true);
	WeaponActor = nullptr;
}

#if WITH_EDITOR
void AGravityGunCharacter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	// If the weapon class to spawn is changed
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AGravityGunCharacter, WeaponClass))
	{
		if (WeaponClass)
		{
			// Spawn the weapon actor from the chosen class and bind to it
			ABaseWeapon * newWeaponActor = nullptr;
			UWorld * world = this->GetWorld();
			if (world)
			{
				newWeaponActor = world->SpawnActor<ABaseWeapon>(WeaponClass, RootComponent->GetComponentTransform());
				if (newWeaponActor)
				{
					this->PickupWeapon(newWeaponActor);
				}
			}
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AGravityGunCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	if (WeaponClass)
	{
		// Spawn the weapon actor from the chosen class and bind to it
		ABaseWeapon * newWeaponActor = nullptr;
		UWorld * world = this->GetWorld();
		if (world)
		{
			newWeaponActor = world->SpawnActor<ABaseWeapon>(WeaponClass, RootComponent->GetComponentTransform());
			if (newWeaponActor)
			{
				this->PickupWeapon(newWeaponActor);
			}
		}
	}

	Mesh1P->SetHiddenInGame(false, true);
}

void AGravityGunCharacter::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor != WeaponActor)
	{
		CurrentlyOverlappedActor = OtherActor;
	}
}

void AGravityGunCharacter::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CurrentlyOverlappedActor = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGravityGunCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("WeaponPrimary", IE_Pressed, this, &AGravityGunCharacter::OnWeaponPrimary);
	PlayerInputComponent->BindAction("WeaponSecondary", IE_Pressed, this, &AGravityGunCharacter::OnWeaponSecondary);

	// Bind interact event
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AGravityGunCharacter::OnInteract);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AGravityGunCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGravityGunCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGravityGunCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGravityGunCharacter::LookUpAtRate);
}

void AGravityGunCharacter::OnInteract()
{
	// If currently holding a weapon
	if (WeaponActor)
	{
		// Drop it
		this->DropWeapon();
	}
	// If not holding a weapon
	else
	{
		// Check if the overlapped actor is a weapon
		ABaseWeapon * weaponActorPickUp = Cast<ABaseWeapon>(CurrentlyOverlappedActor);
		if (weaponActorPickUp)
		{
			// Pick up the weapon if so
			this->PickupWeapon(weaponActorPickUp);
		}
	}
}

void AGravityGunCharacter::OnWeaponPrimary()
{
	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		if (WeaponActor)
		{
			WeaponActor->PrimaryWeaponAction();
		}
	}

	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AGravityGunCharacter::OnWeaponSecondary()
{
	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		if (WeaponActor)
		{
			WeaponActor->SecondaryWeaponAction();
		}
	}
	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AGravityGunCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGravityGunCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGravityGunCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGravityGunCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
