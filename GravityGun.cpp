#include "GravityGun.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

AGravityGun::AGravityGun()
{
	PhysicsHandleComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandleComponent"));

	SplineMeshComponent = CreateDefaultSubobject<USplineMeshComponent>(TEXT("SplineMeshComponent"));
	SplineMeshComponent->SetupAttachment(RootComponent);
	SplineMeshComponent->SetVisibility(false);

	// Gravity Gun requires tick
	PrimaryActorTick.bCanEverTick = true;
}


void AGravityGun::TraceForObjectToGrab()
{
	// Trace to find an object that can be grabbed
	if (TraceComponent)
	{
		bool bBlockingHit = false;
		// Get trace location from trace component
		FVector traceStartLocation = TraceComponent->GetComponentLocation();
		FVector traceEndLocation = traceStartLocation + TraceComponent->GetForwardVector() * WeaponRange;

		UWorld * thisWorld = this->GetWorld();

		if (thisWorld)
		{
			TArray<TEnumAsByte<EObjectTypeQuery>> acceptedObjectTypes;
			// EObjectTypeQuery == ECollisionChannel 
			acceptedObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery2); // World Dynamic
			acceptedObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery4); // Physics Body

			TArray<AActor *> actorsToIgnore;
			FHitResult outHitResult;

			EDrawDebugTrace::Type drawDebugType = bShouldDebugTraces ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None;
			bBlockingHit = UKismetSystemLibrary::LineTraceSingleForObjects((UObject *)thisWorld, traceStartLocation, traceEndLocation, acceptedObjectTypes, false, actorsToIgnore, drawDebugType, outHitResult, true);

			// If trace encountered an object, grab it
			if (bBlockingHit)
			{
				// If the object was WorldDynamic and not simulating physics, set it to do so
				outHitResult.Component->SetSimulatePhysics(true);
				PhysicsHandleComponent->GrabComponentAtLocation(outHitResult.Component.Get(), NAME_None, outHitResult.Actor->GetActorLocation() + HandleGrabOffset);
				bIsGrabbing = true;
				HandleLocation = outHitResult.Location;
				CurrentTargetObject = outHitResult.Actor.Get();
			}
		}
	}
}

void AGravityGun::ReleaseGrabbedObject()
{
	// Call cleanup for sound effects/particles etc spawned for hover/grab effect
	this->EndGrabCleanup();

	bIsGrabbing = false;
	PhysicsHandleComponent->ReleaseComponent();
	CurrentTargetObject = nullptr;
	SplineMeshComponent->SetVisibility(false);
	if (HoverSphereComponent)
	{
		HoverSphereComponent->SetVisibility(false);
	}
}

void AGravityGun::BeginPlay()
{
	Super::BeginPlay();

	if (HoverMesh)
	{
		UStaticMeshComponent * sphereComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("HoverSphereComponent"));
		sphereComponent->RegisterComponent();
		sphereComponent->SetStaticMesh(HoverMesh);
		sphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HoverSphereComponent = sphereComponent;
		HoverSphereComponent->SetWorldScale3D(FVector(3.0f));
		HoverSphereComponent->SetVisibility(false);
		if (HoverMeshMaterial)
		{
			HoverSphereComponent->SetMaterial(0, HoverMeshMaterial);
		}
		// Do not attach as we want it to move independently
	}
}

void AGravityGun::EndGrabCleanup()
{
	// Call all cleanup functions bound to the delegate
	OnEndGrabCleanup.Broadcast();
	// Cleanup all delegate bindings to prevent any hanging references
	OnEndGrabCleanup.Clear();
}

void AGravityGun::Tick(float DeltaSeconds)
{
	// If currently grabbing something, update its position as the parent actor moves
	if (bIsGrabbing)
	{
		AActor * attachParent = this->GetAttachParentActor();
		if (attachParent)
		{
			HandleLocation = attachParent->GetActorLocation() + this->TraceComponent->GetForwardVector() * GrabbedItemDistance;

			// Get old location
			FVector oldLocation;
			FRotator oldRotation;
			PhysicsHandleComponent->GetTargetLocationAndRotation(oldLocation, oldRotation);

			// Find and set new location of grabbed object
			FVector newLocation = FMath::Lerp(oldLocation, HandleLocation + HandleLocationOffset, HandleLocationLerpAlpha);
			PhysicsHandleComponent->SetTargetLocation(newLocation);

			// Set start and end locations of spline in local space
			SplineMeshComponent->SetVisibility(true);
			SplineMeshComponent->SetStartPosition(SplineMeshComponent->GetComponentTransform().InverseTransformPosition(WeaponMesh->GetSocketLocation(TEXT("Muzzle"))));
			SplineMeshComponent->SetEndPosition(SplineMeshComponent->GetComponentTransform().InverseTransformPosition(CurrentTargetObject->GetActorLocation()));

			HoverSphereComponent->SetVisibility(true);
			HoverSphereComponent->SetWorldLocation(CurrentTargetObject->GetActorLocation());
		}
	}
}

void AGravityGun::OnWeaponDropped()
{
	// Drop any currently grabbed objects
	this->ReleaseGrabbedObject();
}

void AGravityGun::PrimaryWeaponAction()
{
	// If grabbing something currently apply force to grabbed item
	if (bIsGrabbing)
	{
		// Apply impulse to push it forwards
		UPrimitiveComponent * targetMesh = nullptr;
		targetMesh = Cast<UPrimitiveComponent>(CurrentTargetObject->GetComponentByClass(UPrimitiveComponent::StaticClass()));
		targetMesh->AddImpulse(TraceComponent->GetForwardVector() * PushForceMagnitude, NAME_None, true);

		// Release the object 
		this->ReleaseGrabbedObject();

	}
	// Apply force to any item found when tracing forward
	else
	{
		this->TraceForObjectToGrab();
		// If an object was found, apply force and release it
		if (CurrentTargetObject)
		{
			UPrimitiveComponent * targetMesh = nullptr;
			targetMesh = Cast<UPrimitiveComponent>(CurrentTargetObject->GetComponentByClass(UPrimitiveComponent::StaticClass()));
			targetMesh->AddImpulse(TraceComponent->GetForwardVector() * PushForceMagnitude, NAME_None, true);

			this->ReleaseGrabbedObject();
		}

	}

	FTransform WeaponMuzzleTransform = WeaponMesh->GetSocketTransform(TEXT("Muzzle"), RTS_World);
	// Try to spawn the particle if specified - Relies on weapon skeletal mesh having a socket called "Muzzle" created
	{
		UParticleSystemComponent* beamParticle = UGameplayStatics::SpawnEmitterAtLocation(this->GetWorld(), PrimaryActionParticleSystem, WeaponMuzzleTransform, true);

		FVector beamTarget = TraceComponent->GetComponentLocation() + TraceComponent->GetForwardVector() * WeaponRange;
		// Set the target location so the beam will correctly land at the area the player is firing at in the reticule 
		beamParticle->SetVectorParameter(TEXT("Target"), beamTarget);
	}

	Super::PrimaryWeaponAction();
}

void AGravityGun::SecondaryWeaponAction()
{
	// If not currently grabbing anything
	if (bIsGrabbing == false)
	{
		this->TraceForObjectToGrab();

		// If grab was successful
		if (bIsGrabbing)
		{
			// Spawn the hover sound effect 
			if (TargetObjectHoverSound != nullptr)
			{
				// AudioComponent is needed to play/stop/pause the sound effect as it is looping and not a fire-once-and-forget
				UAudioComponent * audioComponent = UGameplayStatics::SpawnSoundAttached(TargetObjectHoverSound, CurrentTargetObject->GetRootComponent(), NAME_None, CurrentTargetObject->GetActorLocation(), CurrentTargetObject->GetActorRotation());
				audioComponent->Play();
				// Register the Stop function of the AudioComponent for cleanup later
				OnEndGrabCleanup.AddDynamic(audioComponent, &UAudioComponent::Stop);

				if (HoverSphereComponent)
				{
					HoverSphereComponent->SetVisibility(true);
				}
			}
		}
	}
	// If currently grabbing something, release it
	else
	{
		this->ReleaseGrabbedObject();
	}
	
	FTransform WeaponMuzzleTransform = WeaponMesh->GetSocketTransform(TEXT("Muzzle"), RTS_World);
	// Try to spawn the particle if specified - Relies on weapon skeletal mesh having a socket called "Muzzle" created
	{
		UGameplayStatics::SpawnEmitterAtLocation(this->GetWorld(), SecondaryActionParticleSystem, WeaponMuzzleTransform, true);
	}

	Super::SecondaryWeaponAction();
}

