// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

void ABaseWeapon::PrimaryWeaponAction()
{
	// Try and play the sound if specified 
	if (PrimaryActionSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PrimaryActionSound, GetActorLocation());
	}
}

void ABaseWeapon::SecondaryWeaponAction()
{
	// Try and play the sound if specified 
	if (SecondaryActionSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SecondaryActionSound, GetActorLocation());
	}
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

