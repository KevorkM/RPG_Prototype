// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainAnimInstance.generated.h"

class APawn;

/**
 * 
 */
UCLASS()
class RPG_PROTOTYPE_API UMainAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	
	UFUNCTION(BlueprintCallable, Category = "AnimationProperties")
	void UpdateAnimationProperties();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Movement")
		float movementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
		bool bIsInTheAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
		APawn* Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class AHeroChara* Hero;
};
