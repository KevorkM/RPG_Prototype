// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Collider.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UCameraComponent;
class USpringArmComponent;
class UColliderMovementComponent;

UCLASS()
class RPG_PROTOTYPE_API ACollider : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollider();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	///Mesh
	UPROPERTY(VisibleAnywhere, Category = "MeshComp")
	UStaticMeshComponent*MeshComp;

	///Collider
	UPROPERTY(VisibleAnywhere, Category = "MeshComp")
	USphereComponent*SphereComp;

	///Camera related
	UPROPERTY(VisibleAnywhere, Category = "MeshComp")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = "MeshComp")
	USpringArmComponent*SpringArm;

	///Movement
	UPROPERTY(VisibleAnywhere, Category = "MeshComp")
	UColliderMovementComponent* OurMovementComponent;


	FORCEINLINE UStaticMeshComponent*GetMeshComponent() { return MeshComp; }
	FORCEINLINE void SetMeshComponent(UStaticMeshComponent*Mesh) { MeshComp = Mesh; }

	FORCEINLINE USphereComponent*GetSphereComponent() { return SphereComp; }
	FORCEINLINE void SetSphereComponent(USphereComponent*Sphere) { SphereComp = Sphere; }

	FORCEINLINE UCameraComponent*GetCameraComponent() { return Camera; }
	FORCEINLINE void SetCameraComponent(UCameraComponent*InCamera) { Camera = InCamera; }

	FORCEINLINE USpringArmComponent*GetSpringArmComponent() { return SpringArm; }
	FORCEINLINE void SetSpringArmComponent(USpringArmComponent*InSpringArm) { SpringArm = InSpringArm; }
protected:
	
	void MoveForward(float input);
	void MoveRight(float input);
	void CamerePitch(float AxisValue);
	void CameraYaw(float AxisValue);

	virtual UPawnMovementComponent* GetMovementComponent() const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FVector2D CameraInput;
};
