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
	//���� �ʱ�ȭ
	WalkSpeed = 600.f;
	RunSpeed = 1000.f;
	CrouchSpeed = 300.f;
	SlideSpeed = 1200.f;
	SlideDuration = 0.75f; // �����̵� ���� �ð�(��);
	bIsRunning = false;
	bIsCrouching = false;
	bIsSliding = false;
	bIsJumping = false;
	JumpStartSpeed = 0.0f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	//�⺻ ĳ���� �ӵ� ���� 
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

void AIwanttoberichCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsJumping) {
		MaintainJumpSpeed(DeltaTime);
	}

	//�����̵� ���ο� ���� �ð� ����
	if (bIsSliding) {
		HandleSliding(DeltaTime);
	}
}

//////////////////////////////////////////////////////////////////////////// Input

void AIwanttoberichCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AIwanttoberichCharacter::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AIwanttoberichCharacter::EndJump);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIwanttoberichCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIwanttoberichCharacter::Look);
		
		// �޸��� �׼�
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AIwanttoberichCharacter::StopRunning);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AIwanttoberichCharacter::StartRunning);

		// ��ũ���� �׼�
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AIwanttoberichCharacter::StartCrouching);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AIwanttoberichCharacter::StopCrouching);

		// �����̵� �׼�
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
void AIwanttoberichCharacter::StartJump()
{
	if (!bIsJumping)
	{
		// ���� ���� �� ���� �ӵ� ����
		JumpStartSpeed = GetCharacterMovement()->Velocity.Size();
		bIsJumping = true;

		// ������ �����մϴ�
		ACharacter::Jump();
	}
}
void AIwanttoberichCharacter::EndJump()
{
	if (bIsJumping)
	{
		// ������ ������ �ӵ� ����
		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetSafeNormal() * JumpStartSpeed;
		bIsJumping = false;
	}
}
void AIwanttoberichCharacter::MaintainJumpSpeed(float DeltaTime)
{
	if (GetCharacterMovement()->IsFalling())
	{
		// ���� �� �ӵ� ����
		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetSafeNormal() * JumpStartSpeed;
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
		Crouch();
		bIsCrouching = true;
		UpdateCharacterSpeed();
		
	}
}

void AIwanttoberichCharacter::StopCrouching(const FInputActionValue& Value)
{
	if (!bIsSliding) {
		UnCrouch();
		bIsCrouching = false;
		UpdateCharacterSpeed();
	
	}
}

void AIwanttoberichCharacter::StartSliding()
{
	if (CanSlide())
	{
		bIsSliding = true;
		SlideStartTime = GetWorld()->GetTimeSeconds();
		SlideInitialSpeed = GetCharacterMovement()->IsFalling() ? 1300.0f : 1200.0f;
		GetCharacterMovement()->MaxWalkSpeed = SlideInitialSpeed;

		// �����̵� ���� ����
		SlideDirection = GetVelocity().GetSafeNormal();
		GetCharacterMovement()->Velocity = SlideDirection * SlideInitialSpeed;
	}
}

void AIwanttoberichCharacter::StopSliding()
{
	bIsSliding = false;
	UpdateCharacterSpeed();
}

void AIwanttoberichCharacter::HandleSliding(float DeltaTime)
{
	float ElapsedTime = GetWorld()->GetTimeSeconds() - SlideStartTime;

	// �����̵� �ð� ���
	if (ElapsedTime >= SlideDuration)
	{
		StopSliding();
		return;
	}

	// �����̵� �� �ӵ� ����
	float Alpha = ElapsedTime / SlideDuration;
	float CurrentSpeed = FMath::Lerp(SlideInitialSpeed, SlideEndSpeed, Alpha);
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;

	// ���� ���� (�����̵� �߿��� ���� ��ȯ �Ұ�)
	FVector NewVelocity = SlideDirection * CurrentSpeed;
	GetCharacterMovement()->Velocity = NewVelocity;
}


void AIwanttoberichCharacter::UpdateCharacterSpeed()
{
	float NewSpeed;
	if (bIsRunning)
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
		NewSpeed = WalkSpeed;
	}
	
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

	UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed set to: %f"), NewSpeed);

}

void AIwanttoberichCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	//���� �� �����̵� �ߵ�
	if (bIsSliding)
	{
		StartSliding();
	}
}
bool AIwanttoberichCharacter::CanSlide() const
{
	return GetCharacterMovement()->IsFalling() || GetCharacterMovement()->MaxWalkSpeed > 600.0f;
}
