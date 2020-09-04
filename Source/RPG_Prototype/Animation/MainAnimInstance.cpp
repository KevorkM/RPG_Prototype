// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "Hero/HeroChara.h"
#include <GameFramework/CharacterMovementComponent.h>

void UMainAnimInstance:: NativeInitializeAnimation(){
	if (Pawn == nullptr) {
		Pawn = TryGetPawnOwner();
		if (Pawn) {
			Hero = Cast<AHeroChara>(Pawn);
		}
}

}

void UMainAnimInstance::UpdateAnimationProperties()
{

	if (Pawn == nullptr) {
		Pawn = TryGetPawnOwner();
	}
	if (Pawn) {
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		movementSpeed = LateralSpeed.Size();

		bIsInTheAir = Pawn->GetMovementComponent()->IsFalling();

		if (Hero == nullptr) {
			Hero = Cast<AHeroChara>(Pawn);
		}
	}

}
