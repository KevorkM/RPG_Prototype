// Fill out your copyright notice in the Description page of Project Settings.


#include "Collider.h"
#include "ColliderMovementComponent.h"
#include <Components/SphereComponent.h>
#include <Components/StaticMeshComponent.h>
#include <UObject/ConstructorHelpers.h>
#include <Components/InputComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <Camera/CameraComponent.h>


// Sets default values
ACollider::ACollider()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	/// Adding components in the Blueprint
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(GetRootComponent());
	
	//attach the path of the object that is in the game
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshComponentAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	if (MeshComponentAsset.Succeeded()) { 
		MeshComp->SetStaticMesh(MeshComponentAsset.Object); 
		MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
		MeshComp->SetWorldScale3D(FVector(.8f));
	}

	///Setting up a Sphere Collision.
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(GetRootComponent());
	SphereComp->InitSphereRadius(40.f);
	SphereComp->SetCollisionProfileName(TEXT("Pawn"));
	
	///Setting up the Spring arm for the camera. 
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->RelativeRotation = FRotator(-45.f, 0.f, 0.f);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.f;

	///Setting up the Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	///Make this pawn the main movement
	OurMovementComponent = CreateDefaultSubobject<UColliderMovementComponent>(TEXT("MovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;
	
	///make the pawn be able to look around
	CameraInput = FVector2D(0.f,0.f);

	///Automatically possesses the player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void ACollider::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACollider::MoveForward(float input)
{
	FVector Forward = GetActorForwardVector();
	if (OurMovementComponent) {
		OurMovementComponent->AddInputVector(input* Forward);
	}
}

void ACollider::MoveRight(float input)
{
	FVector Right = GetActorRightVector();
	if (OurMovementComponent) {
		OurMovementComponent->AddInputVector(input* Right);
	}
}

void ACollider::CamerePitch(float AxisValue)
{
	CameraInput.Y = AxisValue;
}

void ACollider::CameraYaw(float AxisValue)
{
	CameraInput.X = AxisValue;
}

UPawnMovementComponent* ACollider::GetMovementComponent() const
{
	return OurMovementComponent;
}

// Called every frame
void ACollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	///Rotate in the x-Axis
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += CameraInput.X;
	SetActorRotation(NewRotation);

	///Rotate in the y Axis
	FRotator NewSpringArmRotation = SpringArm->GetComponentRotation();
	NewSpringArmRotation.Pitch = FMath::Clamp(NewSpringArmRotation.Pitch += CameraInput.Y, -80.f, -15.f);

	SpringArm->SetWorldRotation(NewSpringArmRotation);
}

// Called to bind functionality to input
void ACollider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACollider::MoveForward);

	PlayerInputComponent->BindAxis("MoveRight", this, &ACollider::MoveRight);

	PlayerInputComponent->BindAxis("CameraPitch", this, &ACollider::CamerePitch);

	PlayerInputComponent->BindAxis("CameraYaw", this, &ACollider::CameraYaw);
}

