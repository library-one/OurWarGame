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
	SlideDuration = 1.0f; // �����̵� ���� �ð�(��);
	bIsRunning = false;
	bIsCrouching = false;
	bIsSliding = false;
	//JumpMaxHoldTime = 0.7f;
	JumpMaxCount = 1;
	// �ʱⰪ ����
	bIsJumping = false;
	bJumpPressed = false;  // ó������ ���� Ű�� ������ ����
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
	//JumpMaxCount = 1;
	//// �̹� ���� ���̸� �� �̻� �������� �ʵ��� ����
	//if (!bIsJumping && !bJumpPressed)
	//{
	//	// ������ ������ �� �ӵ��� ����
	//	JumpStartSpeed = GetCharacterMovement()->Velocity.Size();

	//	// ���ڸ����� �����ϴ� ��� �ӵ��� �⺻ �ȱ� �ӵ��� ����
	//	if (GetCharacterMovement()->Velocity.Size() == 0)
	//	{
	//		GetCharacterMovement()->JumpZVelocity = 500.0f;
	//	}
	//	if (bIsRunning)
	//	{
	//		// �޸��� ���¿��� ���� ���� �� �ӵ� ����
	//		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetSafeNormal() * RunSpeed;
	//	}
		ACharacter::Jump();  // ���� ����
		bIsJumping = true;  // ���� �� ���·� ����
		bJumpPressed = true;	
	//}
}
void AIwanttoberichCharacter::EndJump()
{
	// �����̽��ٸ� ������ �� ������ �������� ����
	bIsJumping = false;  // ���� ���� �ƴ��� ǥ��
	bJumpPressed = false;  // ���� Ű ���� ���� ����
}
void AIwanttoberichCharacter::MaintainJumpSpeed(float DeltaTime)
{
	// ���� �߿��� ���� �� ������ �ӵ��� ����
	if (bIsJumping)
	{
		//GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetSafeNormal() * JumpStartSpeed;
	}
}
void AIwanttoberichCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bIsJumping = false;  // ���� �������� ���� ���� ����
	// ���� �� �����̵� ���� Ȯ��
	if (!bIsSliding)
	{
		StartSliding();  // �޸��ٰ� ���� �� ���� �� �����̵� ����
	}
}
void AIwanttoberichCharacter::DecaySpeedAfterRunning()
{
	bIsRunning = false;

	// �ӵ��� ������ ���̱� ���� Ÿ�̸� ����
	GetWorld()->GetTimerManager().SetTimer(SpeedDecayTimerHandle, this, &AIwanttoberichCharacter::UpdateCharacterSpeed, 0.1f, true);

}
void AIwanttoberichCharacter::StartRunning()
{
	bIsRunning = true;
	UpdateCharacterSpeed();
}

void AIwanttoberichCharacter::StopRunning()
{
	GetWorld()->GetTimerManager().SetTimer(SpeedDecayTimerHandle, this, &AIwanttoberichCharacter::DecaySpeedAfterRunning, 0.7f, false);
}

void AIwanttoberichCharacter::StartCrouching()
{
	if (!bIsSliding) {
		Crouch();
		bIsCrouching = true;
		UpdateCharacterSpeed();
	}
}

void AIwanttoberichCharacter::StopCrouching()
{
	if (!bIsSliding) {
		UnCrouch();
		bIsCrouching = false;
		UpdateCharacterSpeed();
	
	}
}

void AIwanttoberichCharacter::StartSliding()
{
	// �����̵� ����: �޸��� ���̰ų� ���� ���� ��, �����̵� ���� �ƴ� ���� ����
	if (!bIsSliding && CanSlide()&& GetCharacterMovement()->MaxWalkSpeed>=600)
	{
		bIsSliding = true;
		SlideStartTime = GetWorld()->GetTimeSeconds();
		SlideInitialSpeed = GetCharacterMovement()->IsFalling() && bIsRunning ? 1500.0f : 1350.0f;
		GetCharacterMovement()->MaxWalkSpeed = SlideInitialSpeed;

		// �����̵� ���� ����
		SlideDirection = GetVelocity().GetSafeNormal();
		GetCharacterMovement()->Velocity = SlideDirection * SlideInitialSpeed;
	}
}

void AIwanttoberichCharacter::StopSliding()
{
	bIsSliding = false;
	UpdateCharacterSpeed();  // �����̵��� ������ �ӵ��� ���󺹱�
}
bool AIwanttoberichCharacter::CanSlide() const
{
	// �����̵� ����: �޸��� ���̰ų� ���߿��� ������ ���
	return !GetCharacterMovement()->IsFalling() && (bIsRunning && !bIsSliding);
}

void AIwanttoberichCharacter::HandleSliding(float DeltaTime)
{
	float ElapsedTime = GetWorld()->GetTimeSeconds() - SlideStartTime;

	// �����̵� �ð� ���
	if (ElapsedTime >= SlideDuration)
	{
		StopSliding();  // �����̵� �ð��� ������ ����
		return;
	}

	// �����̵� �� �ӵ� ����
	float Alpha = ElapsedTime / SlideDuration;
	float CurrentSpeed = FMath::Lerp(SlideInitialSpeed, SlideEndSpeed, Alpha);
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;

	// �����̵� �� ���� ����
	FVector NewVelocity = SlideDirection * CurrentSpeed;
	GetCharacterMovement()->Velocity = NewVelocity;
}

void AIwanttoberichCharacter::UpdateCharacterSpeed()
{
	float NewSpeed;
	if (bIsRunning)
	{
		NewSpeed = RunSpeed;
	}
	else if (bIsCrouching)
	{
		NewSpeed = CrouchSpeed;
	}
	else
	{
		NewSpeed = WalkSpeed;
	}

	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

	// �ӵ��� WalkSpeed�� �����ϸ� Ÿ�̸� ����
	if (FMath::IsNearlyEqual(NewSpeed, WalkSpeed, 5.0f))
	{
		GetWorld()->GetTimerManager().ClearTimer(SpeedDecayTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed set to: %f"), NewSpeed);
}