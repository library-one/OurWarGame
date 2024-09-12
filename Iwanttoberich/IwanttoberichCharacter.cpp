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
	SlideDuration = 1.0f; // 슬라이딩 지속 시간(초);
	bIsRunning = false;
	bIsCrouching = false;
	bIsSliding = false;
	//JumpMaxHoldTime = 0.7f;
	JumpMaxCount = 1;
	// 초기값 설정
	bIsJumping = false;
	bJumpPressed = false;  // 처음에는 점프 키가 눌리지 않음
	JumpStartSpeed = 0.0f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
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

void AIwanttoberichCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsJumping) {
		MaintainJumpSpeed(DeltaTime);
	}

	//슬라이딩 여부에 따라 시간 전달
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
void AIwanttoberichCharacter::StartJump()
{
	//JumpMaxCount = 1;
	//// 이미 점프 중이면 더 이상 점프하지 않도록 설정
	//if (!bIsJumping && !bJumpPressed)
	//{
	//	// 점프를 시작할 때 속도를 저장
	//	JumpStartSpeed = GetCharacterMovement()->Velocity.Size();

	//	// 제자리에서 점프하는 경우 속도를 기본 걷기 속도로 설정
	//	if (GetCharacterMovement()->Velocity.Size() == 0)
	//	{
	//		GetCharacterMovement()->JumpZVelocity = 500.0f;
	//	}
	//	if (bIsRunning)
	//	{
	//		// 달리기 상태에서 점프 시작 시 속도 유지
	//		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetSafeNormal() * RunSpeed;
	//	}
		ACharacter::Jump();  // 점프 실행
		bIsJumping = true;  // 점프 중 상태로 설정
		bJumpPressed = true;	
	//}
}
void AIwanttoberichCharacter::EndJump()
{
	// 스페이스바를 떼었을 때 점프를 끝내도록 설정
	bIsJumping = false;  // 점프 중이 아님을 표시
	bJumpPressed = false;  // 점프 키 눌림 상태 해제
}
void AIwanttoberichCharacter::MaintainJumpSpeed(float DeltaTime)
{
	// 점프 중에는 시작 시 설정된 속도를 유지
	if (bIsJumping)
	{
		//GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetSafeNormal() * JumpStartSpeed;
	}
}
void AIwanttoberichCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	bIsJumping = false;  // 착지 시점에서 점프 상태 해제
	// 착지 후 슬라이딩 조건 확인
	if (!bIsSliding)
	{
		StartSliding();  // 달리다가 점프 후 착지 시 슬라이딩 시작
	}
}
void AIwanttoberichCharacter::DecaySpeedAfterRunning()
{
	bIsRunning = false;

	// 속도를 서서히 줄이기 위한 타이머 설정
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
	// 슬라이딩 조건: 달리기 중이거나 점프 착지 후, 슬라이딩 중이 아닐 때만 실행
	if (!bIsSliding && CanSlide()&& GetCharacterMovement()->MaxWalkSpeed>=600)
	{
		bIsSliding = true;
		SlideStartTime = GetWorld()->GetTimeSeconds();
		SlideInitialSpeed = GetCharacterMovement()->IsFalling() && bIsRunning ? 1500.0f : 1350.0f;
		GetCharacterMovement()->MaxWalkSpeed = SlideInitialSpeed;

		// 슬라이딩 방향 설정
		SlideDirection = GetVelocity().GetSafeNormal();
		GetCharacterMovement()->Velocity = SlideDirection * SlideInitialSpeed;
	}
}

void AIwanttoberichCharacter::StopSliding()
{
	bIsSliding = false;
	UpdateCharacterSpeed();  // 슬라이딩이 끝나면 속도를 원상복구
}
bool AIwanttoberichCharacter::CanSlide() const
{
	// 슬라이딩 조건: 달리기 중이거나 공중에서 착지한 경우
	return !GetCharacterMovement()->IsFalling() && (bIsRunning && !bIsSliding);
}

void AIwanttoberichCharacter::HandleSliding(float DeltaTime)
{
	float ElapsedTime = GetWorld()->GetTimeSeconds() - SlideStartTime;

	// 슬라이딩 시간 계산
	if (ElapsedTime >= SlideDuration)
	{
		StopSliding();  // 슬라이딩 시간이 끝나면 종료
		return;
	}

	// 슬라이딩 중 속도 감소
	float Alpha = ElapsedTime / SlideDuration;
	float CurrentSpeed = FMath::Lerp(SlideInitialSpeed, SlideEndSpeed, Alpha);
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;

	// 슬라이딩 중 방향 고정
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

	// 속도가 WalkSpeed에 도달하면 타이머 정지
	if (FMath::IsNearlyEqual(NewSpeed, WalkSpeed, 5.0f))
	{
		GetWorld()->GetTimerManager().ClearTimer(SpeedDecayTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed set to: %f"), NewSpeed);
}