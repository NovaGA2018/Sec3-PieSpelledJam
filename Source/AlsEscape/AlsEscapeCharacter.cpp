// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "AlsEscapeCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Classes/Components/SphereComponent.h" 

//////////////////////////////////////////////////////////////////////////
// AAlsEscapeCharacter

AAlsEscapeCharacter::AAlsEscapeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->SetupAttachment(RootComponent);
	CollectionSphere->SetSphereRadius(200.0f);
	SpeedFactor = 2.0f;
	InitialStamina = 100.0f;
	CharacterStamina = InitialStamina; 
	SpeedFactor = 2.0f;
	BaseSpeed = GetCharacterMovement()->MaxWalkSpeed;
	PrimaryActorTick.bCanEverTick = true; 

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AAlsEscapeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AAlsEscapeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AAlsEscapeCharacter::MoveRight);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AAlsEscapeCharacter::CharacterSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AAlsEscapeCharacter::StopCharacterSprint);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AAlsEscapeCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AAlsEscapeCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AAlsEscapeCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AAlsEscapeCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AAlsEscapeCharacter::OnResetVR);

}


void AAlsEscapeCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AAlsEscapeCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AAlsEscapeCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AAlsEscapeCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AAlsEscapeCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AAlsEscapeCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AAlsEscapeCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AAlsEscapeCharacter::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if (IsSprinting) {
		if (GetCurrentStamina() <= 0) {
			StopCharacterSprint();
		}
		else {
			UpdateStamina(-0.5f);
		}
	}

	if (GetCurrentStamina() < 100 && !IsSprinting) {
		UpdateStamina(0.25f);
	}

}



void AAlsEscapeCharacter::CharacterSprint() {
	GetCharacterMovement()->MaxWalkSpeed *= SpeedFactor;
	IsSprinting = true;
}

void AAlsEscapeCharacter::StopCharacterSprint() {
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	IsSprinting = false;
}

float AAlsEscapeCharacter::GetInitialStamina() {
	return InitialStamina;
}

float AAlsEscapeCharacter::GetCurrentStamina() {
	return CharacterStamina;
}

void AAlsEscapeCharacter::UpdateStamina(float StaminaUpdate)
{
	CharacterStamina = GetCurrentStamina() + StaminaUpdate;

	if (GetCurrentStamina() > 100) {
		CharacterStamina = 100;

		if (GetCurrentStamina() < 0) {
			CharacterStamina = 0;
		}
	}

}

