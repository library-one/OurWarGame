// Copyright Epic Games, Inc. All Rights Reserved.

#include "IwanttoberichCharacter.h"
#include "IwanttoberichProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AIwanttoberichCharacter

AIwanttoberichCharacter::AIwanttoberichCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	//변수 초기화
	WalkSpeed = 600.f;
	RunSpeed = 1000.f;
	CrouchSpeed = 300.f;
	SlideSpeed = 1200.f;
	SlideDuration = 1.5f; // 슬라이딩 지속 시간(초);

	bIsRunning = false;
	bIsCrouching = false;
	bIsSliding = false;

	//기본 캐릭터 속도 설정 
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

}

void AIwanttoberichCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////// Input

void AIwanttoberichCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIwanttoberichCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIwanttoberichCharacter::Look);
		
		// 달리기 액션
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AIwanttoberichCharacter::StopRunning);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AIwanttoberichCharacter::StartRunning);

		// 웅크리기 액션
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AIwanttoberichCharacter::StartCrouching);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AIwanttoberichCharacter::StopCrouching);

		// 슬라이딩 액션
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AIwanttoberichCharacter::StartSliding);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AIwanttoberichCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AIwanttoberichCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AIwanttoberichCharacter::StartRunning(const FInputActionValue& Value)
{
	bIsRunning = true;
	UpdateCharacterSpeed();
}

void AIwanttoberichCharacter::StopRunning(const FInputActionValue& Value)
{
	bIsRunning = false;
	UpdateCharacterSpeed();
}

void AIwanttoberichCharacter::StartCrouching(const FInputActionValue& Value)
{
	if (!bIsSliding) {
		bIsCrouching = true;
		UpdateCharacterSpeed();
		Crouch();
	}
}

void AIwanttoberichCharacter::StopCrouching(const FInputActionValue& Value)
{
	if (!bIsSliding) {
		bIsCrouching = true;
		UpdateCharacterSpeed();
		UnCrouch();
	}
}

void AIwanttoberichCharacter::StartSliding()
{
	if (bIsRunning && GetCharacterMovement()->Velocity.Size() > 600.f) // 일정 속도 이상일 때만 슬라이딩
	{
		bIsSliding = true;
		UpdateCharacterSpeed();

		// 일정 시간 후 슬라이딩 종료
		GetWorld()->GetTimerManager().SetTimer(SlideTimerHandle, this, &AIwanttoberichCharacter::StopSliding, SlideDuration, false);
	}
}

void AIwanttoberichCharacter::StopSliding()
{
	bIsSliding = false;
	UpdateCharacterSpeed();
}

void AIwanttoberichCharacter::UpdateCharacterSpeed()
{
	float NewSpeed;
	if (bIsSliding)
	{
		GetCharacterMovement()->MaxWalkSpeed = SlideSpeed;
		NewSpeed = SlideSpeed;
	}
	else if (bIsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		NewSpeed = RunSpeed;
	}
	else if (bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		NewSpeed = CrouchSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		NewSpeed = CrouchSpeed;
	}
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed set to: %f"), NewSpeed);

}
