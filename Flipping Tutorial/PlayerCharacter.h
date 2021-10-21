// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UENUM()
enum PlayerOrientation
{
	Normal,
	Flipped
};

UCLASS()
class VERTIGO_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

		//Built-in functionality
protected:
	virtual void BeginPlay() override;

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Custom Code
protected:

	///MOVEMENT FUNCTIONS (EXPOSED)
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	///MOVEMENT FUNCTIONS (EXPOSED)

	///FLIPPING FUNCTIONS (EXPOSED)
	UFUNCTION()
		void FlipUp(FHitResult hit, FVector initalVelocity, float jumpZ);
	//UFUNCTION()
	//void FlipDown(FHitResult hit);
	///FLIPPING FUNCTIONS (EXPOSED)

	///INSTANCES (EXPOSED)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UCameraComponent* Camera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UCurveFloat* z_bottomCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UCharacterMovementComponent* Movement;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<PlayerOrientation> orientation;
	///INSTANCES (EXPOSED)

	///SPEED VARIABLES (EXPOSED)
	float baseSpeed = 2000;
	///SPEED VARIABLES (EXPOSED)

	///FLIPPING VARIABLES (EXPOSED)
	float minFlipSpeed = 2000; //Around 2000
	float flipCurve = 2400.0f;
	///FLIPPING VARIABLES (EXPOSED)


private:
	///FLIPPING FUNCTIONS (HIDDEN)
	void HandleOrientation();

	void SetupGroundConditions();
	void SetFlipConditions();
	void SetupCeilingConditions();

	FVector FlipLineStart();
	FVector FlipLineEnd();
	///FLIPPING FUNCTIONS (HIDDEN)

	///GETTER FUNCTIONS (HIDDEN)
	FVector PlayerLocation();
	FRotator PlayerRotation();
	///GETTER FUNCTIONS (HIDDEN)

	///TIMERS (HIDDEN)
	FTimerHandle FlipTimer;
	///TIMERS (HIDDEN)

	///FLIPPING VARIABLES (HIDDEN)
	float currentCameraRoll;
	float storedVelocityMagnitude;
	FVector originalDistance;
	///FLIPPING VARIABLES (HIDDEN)

};