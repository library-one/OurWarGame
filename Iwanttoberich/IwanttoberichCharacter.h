// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "IwanttoberichCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AIwanttoberichCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
public:
	AIwanttoberichCharacter();

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);
public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	
	// Enhanced Input 액션
	//달리기 우크리기 슬라이드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RunAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlideAction;
protected:
	//달리기 , 웅크리기, 슬라이딩 변수 선언 
	bool bIsRunning;
	bool bIsCrouching;
	bool bIsSliding;
	bool bIsJumping;
	bool bJumpPressed;  // 점프 키가 눌렸는지 여부를 추적하는 변수
	//캐릭터 속도
	float WalkSpeed;
	float RunSpeed;
	float CrouchSpeed;
	float SlideSpeed;
	float SlideStartTime;
	float SlideInitialSpeed;
	float JumpStartSpeed;
	float SlideEndSpeed = 600.0f;
	//슬라이딩 지속 시간
	float SlideDuration=0.75f;
	bool bCanSlideAfterLanding;
	bool bIsOnSlope;


	//슬라이딩 타이머
	FTimerHandle SlideTimerHandle;
	//달리기 지속 타이머
	FTimerHandle SpeedDecayTimerHandle;

	//달리기, 웅크리기, 슬라이딩 변수 선언
	void StartRunning();
	void StopRunning();
	void StartCrouching();
	void StopCrouching();
	void StartSliding();
	void StopSliding();
	void HandleSliding(float DeltaTime);
	//캐릭터 속도 갱신 함수
	void UpdateCharacterSpeed();
	//캐릭터 점프 함수
	void StartJump();
	void EndJump();
	//캐릭터 슬라이딩
	void Landed(const FHitResult& Hit) override;
	void DecaySpeedAfterRunning();
	bool CanSlide() const;
	FVector SlideDirection;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
private:
	void MaintainJumpSpeed(float DeltaTime);
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface
public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

