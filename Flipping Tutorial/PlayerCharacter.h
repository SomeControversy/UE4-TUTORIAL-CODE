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
