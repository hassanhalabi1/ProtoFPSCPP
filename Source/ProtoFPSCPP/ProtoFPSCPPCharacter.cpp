// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ProtoFPSCPPCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "DrawDebugHelpers.h"
#include "EngineGlobals.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AProtoFPSCPPCharacter

AProtoFPSCPPCharacter::AProtoFPSCPPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	//Create PhysicsHandleComponent
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

void AProtoFPSCPPCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}
void AProtoFPSCPPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	Start = FirstPersonCameraComponent->GetComponentLocation();
	End = FirstPersonCameraComponent->GetForwardVector() * RaycastDistance + GetFirstPersonCameraComponent() ->GetComponentLocation();
	FCollisionQueryParams TraceParams;
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

	if (PhysicsHandle->GetGrabbedComponent()) {
		GEngine->AddOnScreenDebugMessage(-1, 1.F, FColor::Red, FString::Printf(TEXT("You are hitting: %s"),
			*PhysicsHandle->GetGrabbedComponent()->GetName()));
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProtoFPSCPPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AProtoFPSCPPCharacter::Interact);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AProtoFPSCPPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProtoFPSCPPCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProtoFPSCPPCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProtoFPSCPPCharacter::LookUpAtRate);
}

void AProtoFPSCPPCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AProtoFPSCPPCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AProtoFPSCPPCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProtoFPSCPPCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProtoFPSCPPCharacter::Interact()
{
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f);
	if (Hit.GetComponent()) {
		PhysicsHandle->GrabComponentAtLocation(Hit.GetComponent(), Hit.BoneName, End);
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 1.F, FColor::Red, FString::Printf(TEXT("You are hitting: %s"),
				*Hit.GetActor()->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, 1.F, FColor::Red, FString::Printf(TEXT("Impact Point : %s"),
				*Hit.ImpactPoint.ToString()));
		}
	}
}