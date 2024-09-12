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
	
	// Enhanced Input �׼�
	//�޸��� ��ũ���� �����̵�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RunAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SlideAction;
protected:
	//�޸��� , ��ũ����, �����̵� ���� ���� 
	bool bIsRunning;
	bool bIsCrouching;
	bool bIsSliding;
	bool bIsJumping;
	bool bJumpPressed;  // ���� Ű�� ���ȴ��� ���θ� �����ϴ� ����
	//ĳ���� �ӵ�
	float WalkSpeed;
	float RunSpeed;
	float CrouchSpeed;
	float SlideSpeed;
	float SlideStartTime;
	float SlideInitialSpeed;
	float JumpStartSpeed;
	float SlideEndSpeed = 600.0f;
	//�����̵� ���� �ð�
	float SlideDuration=0.75f;
	bool bCanSlideAfterLanding;
	bool bIsOnSlope;


	//�����̵� Ÿ�̸�
	FTimerHandle SlideTimerHandle;
	//�޸��� ���� Ÿ�̸�
	FTimerHandle SpeedDecayTimerHandle;

	//�޸���, ��ũ����, �����̵� ���� ����
	void StartRunning();
	void StopRunning();
	void StartCrouching();
	void StopCrouching();
	void StartSliding();
	void StopSliding();
	void HandleSliding(float DeltaTime);
	//ĳ���� �ӵ� ���� �Լ�
	void UpdateCharacterSpeed();
	//ĳ���� ���� �Լ�
	void StartJump();
	void EndJump();
	//ĳ���� �����̵�
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

