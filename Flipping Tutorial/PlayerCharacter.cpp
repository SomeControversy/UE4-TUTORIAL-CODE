// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Curves/CurveVector.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

///BASE FUNCTIONS
APlayerCharacter::APlayerCharacter()
{
	//TICK TOGGLE
	PrimaryActorTick.bCanEverTick = true;

	//CAMERA SETUP
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);

	//CHARMOVE SETUP
	Movement = GetCharacterMovement();
}

void APlayerCharacter::BeginPlay()
{
	//SUPER
	Super::BeginPlay();

	//SET PLAYER TO FUNCTION ON GROUND
	SetupGroundConditions();

	//RESET CAMERA ROLL
	currentCameraRoll = 0.0f;

	//REMOVE LATER
	Movement->MaxWalkSpeed = baseSpeed;
	Movement->MaxFlySpeed = baseSpeed;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	//SUPER
	Super::Tick(DeltaTime);

	//UPDATE CAMERA
	Camera->SetWorldRotation(FRotator(GetViewRotation().Pitch, GetViewRotation().Yaw, currentCameraRoll));
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bind Actions to Controls
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Flip", IE_Pressed, this, &APlayerCharacter::HandleOrientation);

	//Bind Movement Input to Controls
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);
}
///BASE FUNCTIONS

///MOVEMENT FUNCTIONS
void APlayerCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void APlayerCharacter::Turn(float Value)
{
	if (orientation == Normal)
	{
		AddControllerYawInput(Value);
	}
	else if (orientation == Flipped)
	{
		AddControllerYawInput(-Value);
	}
}

void APlayerCharacter::LookUp(float Value)
{
	if (orientation == Normal)
	{
		AddControllerPitchInput(Value);
	}
	else if (orientation == Flipped)
	{
		AddControllerPitchInput(-Value);
	}
}
///MOVEMENT FUNCTIONS


///HANDLERS
void APlayerCharacter::HandleOrientation()
{
	if (Movement->IsFalling())
	{
		if (orientation == Normal)
		{
			FHitResult hit;
			FCollisionQueryParams parameters;
			parameters.AddIgnoredActor(GetOwner());

			if (GetWorld()->LineTraceSingleByChannel(hit, FlipLineStart(), FlipLineEnd(), ECC_Visibility, parameters))
			{
				DrawDebugLine(GetWorld(), FlipLineStart(), FlipLineEnd(), FColor::Green, true);
				if (hit.ImpactNormal.Z <= -0.700f && hit.Distance <= 4000) //This may need to be adjusted
				{
					FTimerDelegate FlipTimerDel;
					storedVelocityMagnitude = Movement->Velocity.Size(); //Save velocity for after flip
					FlipTimerDel.BindUFunction(this, FName("FlipUp"), hit, GetVelocity(), PlayerLocation().Z);
					GetWorldTimerManager().SetTimer(FlipTimer, FlipTimerDel, GetWorld()->GetDeltaSeconds(), true);
				}
			}
			else
			{
				return;
			}
		}
		if (orientation == Flipped)
		{
			return;
		}
	}
}
//HANDLERS


///FLIPPING 
void APlayerCharacter::FlipUp(FHitResult hit, FVector initialVelocity, float jumpZ)
{
	///How flipping should work:
	//Player Camera should interpolate around PlayerLocation(); 
	//Set the player to walk on the ceiling and create a custom movement mode
	//All of this must be completely dynamic, and should work on any valid surface that is 1. Upside down & 2. Within the acceptable range of tilt (Use surface normals for this)
	// Also player shouldn't be able to flip on surfaces that are too close to flip onto


	//BASIC LINE INFORMATION
	FVector bottomPoint = FVector(hit.ImpactPoint.X, hit.ImpactPoint.Y, jumpZ); //Orange
	float flipHeight = FVector::Dist(hit.ImpactPoint, bottomPoint);
	FVector lowerMidPoint = FVector(hit.ImpactPoint.X, hit.ImpactPoint.Y, jumpZ + flipHeight / 4); //Yellow
	FVector upperMidPoint = FVector(hit.ImpactPoint.X, hit.ImpactPoint.Y, hit.ImpactPoint.Z - flipHeight / 4); //Green
	FVector topPoint = hit.ImpactPoint; //Blue

	//DRAW LINE POUINTS
	DrawDebugPoint(GetWorld(), bottomPoint, 10, FColor::Orange, false);
	DrawDebugPoint(GetWorld(), lowerMidPoint, 10, FColor::Yellow, false);
	DrawDebugPoint(GetWorld(), upperMidPoint, 10, FColor::Green, false);
	DrawDebugPoint(GetWorld(), topPoint, 10, FColor::Blue, false);

	/*
		//OFFSET POINT (This is for later when curving the flip)
		float circleRadius = flipHeight / 2;
		if ((PlayerLocation() - bottomPoint).Normalize(1.0f))
			originalDistance = (PlayerLocation() - bottomPoint).GetSafeNormal(1.0f);
		FVector offsetBottomPoint = bottomPoint + (originalDistance * circleRadius);

		//CAMERA ORBIT (For later when curving the camera path)
		FVector arcPoint = offsetBottomPoint + FVector(0.0f, 0.0f, flipHeight/2); //Purple
	*/
	if (flipHeight >= 400) //Tested - Maybe make a variable
	{
		if (PlayerLocation() == upperMidPoint) //The player is at the ceiling and is finished flipping
		{
			currentCameraRoll = 180;
			Movement->Velocity = storedVelocityMagnitude * Camera->GetForwardVector();

			//CONFIGURE PLAYER TO WALK ON CEILING 
			SetupCeilingConditions();

			GetWorldTimerManager().ClearTimer(FlipTimer);
			return;
		}
		else if ((bottomPoint.X - PlayerLocation().X) < (flipHeight / 2) && (bottomPoint.Y - PlayerLocation().Y) < (flipHeight / 2)) //Player is at offset point 
		{
			Movement->Velocity = FVector::ZeroVector;
			SetFlipConditions();

			if (PlayerLocation().Z < lowerMidPoint.Z) //Smoothing into flip 
			{
				FVector newFlipLocation = FMath::VInterpConstantTo(PlayerLocation(), lowerMidPoint, GetWorld()->GetDeltaSeconds(), minFlipSpeed + (initialVelocity.Size() / 3));
				SetActorLocation(newFlipLocation);
			}
			else //Moving directly upwards
			{
				float betweenPercent = (PlayerLocation().Z - lowerMidPoint.Z) / (upperMidPoint.Z - lowerMidPoint.Z);
				currentCameraRoll = FMath::Lerp(0, 180, betweenPercent);
				Camera->SetRelativeLocation(FVector(0, 0, FMath::Lerp(60, -60, betweenPercent)));

				//MOVE PLAYER UPWARDS 
				FVector NewLocation = FMath::VInterpConstantTo(PlayerLocation(), FVector(hit.ImpactPoint.X, hit.ImpactPoint.Y, hit.ImpactPoint.Z - flipHeight / 4), GetWorld()->GetDeltaSeconds(), minFlipSpeed / 3);
				SetActorLocation(NewLocation, true);
			}
		}
		else
		{
			//INTERPOLATE PLAYER TO STARTING POINT 
			FVector NewVelocity = FMath::VInterpConstantTo(GetVelocity(), FVector::ZeroVector, GetWorld()->GetDeltaSeconds(), flipCurve);
			FVector NewLocation = FMath::VInterpConstantTo(PlayerLocation(), bottomPoint, GetWorld()->GetDeltaSeconds(), minFlipSpeed + (FMath::Abs(initialVelocity.Size() - NewVelocity.Size()) / 3));

			//SET NEW POSITION AND VELOCITY
			SetActorLocation(NewLocation, true, 0, ETeleportType::None);
			Movement->Velocity = NewVelocity;

			/// [NewLocation] is affected by the player's velocity, and has a base speed [minFlipSpeed] that is increased by a third of the player's inital velocity
			/// [flipCurve] affects how fast the velocity decreases, which affects how fast the player turns toward the point. Higher flipcurve means smaller curves, and vice versa.
		}
	}
	else
	{
		return;
	}
}

void APlayerCharacter::SetupGroundConditions()
{
	//SET PLAYER ORIENTATION
	orientation = Normal;

	//SET PLAYER MOVEMENT MODE
	Movement->SetMovementMode(EMovementMode::MOVE_Walking);

	//NORMALIZE GRAVITY
	Movement->GravityScale = 1;

	//ENABLE PLAYER INPUT
	GetController()->SetIgnoreMoveInput(false);
}

void APlayerCharacter::SetFlipConditions()
{
	//DISABLE GRAVITY 
	Movement->GravityScale = 0;

	//DISABLE PLAYER INPUT
	GetController()->SetIgnoreMoveInput(true);
}

void APlayerCharacter::SetupCeilingConditions()
{
	//SET PLAYER ORIENTATION
	orientation = Flipped;

	//SET PLAYER MOVEMENT MODE
	Movement->SetMovementMode(EMovementMode::MOVE_Flying);

	//INVERT GRAVITY
	Movement->GravityScale = -1;

	//ENABLE PLAYER INPUT
	GetController()->SetIgnoreMoveInput(false);
}

FVector APlayerCharacter::FlipLineStart()
{
	//SHORT TERM STORAGE VARIABLES
	FVector playerViewpointLocation;
	FRotator playerViewpointRotation;

	//GET LOCATION OF PLAYER VIEWPOINT
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(playerViewpointLocation, playerViewpointRotation);

	//RETURN VIEWPOINT LOCATION
	return(playerViewpointLocation);
}

FVector APlayerCharacter::FlipLineEnd()
{
	//SHORT TERM STORAGE VARIABLES
	FVector playerViewpointLocation;
	FRotator playerViewpointRotation;

	//GET PLAYER VIEWPOINT
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(playerViewpointLocation, playerViewpointRotation);

	//RETURN VIEWPOINT MULTIPLIED BY DISTANCE
	return(playerViewpointLocation + playerViewpointRotation.Vector() * 4000);
}
///FLIPPING 


///GETTERS
FVector APlayerCharacter::PlayerLocation()
{
	return GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
}

FRotator APlayerCharacter::PlayerRotation()
{
	return GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorRotation();
}
///GETTERS

