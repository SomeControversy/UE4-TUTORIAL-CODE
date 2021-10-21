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