// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroChara.h"
#include "Weapon.h"
#include "Enemy/Enemy.h"
#include "HUD/MyPlayerController.h"
#include "MySaveGame.h"
#include "ItemStorage.h"
#include <Camera/CameraComponent.h>
#include <GameFramework/SpringArmComponent.h>
#include <GameFramework/PlayerController.h>
#include <Kismet/GameplayStatics.h>
#include <Engine/World.h>
#include <Components/CapsuleComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Animation/AnimInstance.h>
#include <Sound/SoundCue.h>
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/KismetMathLibrary.h>



// Sets default values
AHeroChara::AHeroChara()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	///Spring Arm
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->TargetArmLength = 600.f;
	SpringArmComp->SetupAttachment(RootComponent);

	///Camera
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	/* Attach the camera to the end of SprinArmComp and let it adjust
	to match the controller orientation*/
	CameraComp->bUsePawnControlRotation = false;

	GetCapsuleComponent()->SetCapsuleSize(48.f, 105.f);

	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	/*do not rotate the camera when the controller rotates.
	let that just affect the camera*/
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	//Configure Character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;//Character moves in the direction of the input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);//... at this rotation rate 
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = .2f;

	///Player Status
	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;	
	Stamina = 120.f;
	coins = 0;

	///Movement Status
	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;

	bShiftKeyDown = false;
	bLMBDown = false;
	bESCDown = false;

	///Initialize Enums

	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;

	bMovingForward = false;
	bMovingRight = false;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bhasCombatTarget = false;
}

void AHeroChara::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;

	if (MovementStatus == EMovementStatus::EMS_Sprinting) {
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AHeroChara::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

void AHeroChara::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && CombatMontage) {

			int32 Section = FMath::RandRange(0, 1);
			switch (Section) {
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.2f);
				AnimInstance->Montage_JumpToSection(FName("Attack01"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.8f);
				AnimInstance->Montage_JumpToSection(FName("Attack02"), CombatMontage);
				break;

			default:
				;
			}
		}
	}
}

void AHeroChara::AttackEnd()
{
	bAttacking = false;
	bInterpToEnemy = false;

	if (bLMBDown) {
		Attack();
	}
}

void AHeroChara::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound) {
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

// Called when the game starts or when spawned
void AHeroChara::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = Cast<AMyPlayerController>(GetController());

	LoadGameNoSwitch();
	if (PlayerController) {
		PlayerController->GameModeOnly();
	}

}

void AHeroChara::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AHeroChara::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AHeroChara::LMBDown()
{

	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	if (PlayerController) if (PlayerController->bPauseMenuVisible) return;

	if (ActiveOverlappingItem)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}
	else if (EquippedWeapon)
	{
		Attack();
	}
}

void AHeroChara::LMBUp()
{
	bLMBDown = false;
}

void AHeroChara::ESCDown()
{
	bESCDown = true;

	if (PlayerController) {
		PlayerController->TogglePauseMenu();
	}
}

void AHeroChara::ESCUp()
{
	bESCDown = false;
}

void AHeroChara::DecrementHealth(float Amount)
{
	
}

float AHeroChara::TakeDamage(float DamageAmount, FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	if (Health - DamageAmount <= 0.f) {
		Health -= DamageAmount;
		Die();
		if (DamageCauser) {
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy) {
				Enemy->bHasValidTarget = false;
			}
		}
	}
	else {
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AHeroChara::IncrementCoins(int32 Amount)
{
	coins += Amount;
}

void AHeroChara::IncrementHealth(float Amount)
{
	if (Health + Amount >= MaxHealth) {
		Health = MaxHealth;
	
	}
	else {
		Health += Amount;
	}
}

void AHeroChara::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead)return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && CombatMontage) {
		AnimInstance->Montage_Play(CombatMontage, 1.0);
		AnimInstance->Montage_JumpToSection(FName("Death01"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AHeroChara::Jump()
{
	if (PlayerController) if (PlayerController->bPauseMenuVisible) return;

	if (MovementStatus != EMovementStatus::EMS_Dead) {
		Super::Jump();
	}
}

void AHeroChara::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AHeroChara::UpdateCombatTarget()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0)
	{
		if (PlayerController)
		{
			PlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if (ClosestEnemy)
	{
		FVector Location = GetActorLocation();
		float MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size();

		for (auto Actor : OverlappingActors)
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}
		if (PlayerController)
		{
			PlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bhasCombatTarget = true;
	}
}

void AHeroChara::SwitchLevel(FName LevelName)
{
	UWorld * World = GetWorld();
	if (World) {
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel);

		if (CurrentLevelName != LevelName) {

			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

bool AHeroChara::CanMove(float Value)
{

	if (PlayerController) {
	return (Value != 0.f)
		&& (!bAttacking)
		&& (MovementStatus != EMovementStatus::EMS_Dead)
		&& !PlayerController->bPauseMenuVisible;
	}
	return false;
}

void AHeroChara::MoveForward(float Value)
{
	bMovingForward = false;
	
	if (CanMove(Value)) {

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}
}

void AHeroChara::MoveRight(float Value)
{
	bMovingRight = false;
	if (CanMove(Value)) {

		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	
		bMovingRight = true;
	}
}

void AHeroChara::Turn(float Value)
{
	if (CanMove(Value)) {
		AddControllerYawInput(Value);
	}
}

void AHeroChara::LookUp(float Value)
{
	if (CanMove(Value)) {
		AddControllerPitchInput(Value);
	}
}

void AHeroChara::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate*BaseTurnRate*GetWorld()->GetDeltaSeconds());
}

void AHeroChara::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

// Called every frame
void AHeroChara::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else
			{
				Stamina -= DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Sprinting);
			if (bMovingForward || bMovingRight)
			{
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else
			{
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		else // Shift key up
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown)
		{
			if (Stamina - DeltaStamina <= 0.f)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				Stamina = 0;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else
			{
				Stamina -= DeltaStamina;
				
				if (bMovingForward || bMovingRight)
					{
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else
				{
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
		}
		else // Shift key up
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown)
		{
			Stamina = 0.f;
		}
		else // Shift key up
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	case EStaminaStatus::ESS_ExhaustedRecovering:
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	default:
		;
	}
	
	if (bInterpToEnemy && CombatTarget) {
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation);
	}
	if (CombatTarget) {
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (PlayerController) {
			PlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

FRotator AHeroChara::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
	}

// Called to bind functionality to input
void AHeroChara::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	check(PlayerInputComponent);
	///Bind Action type
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AHeroChara::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AHeroChara::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AHeroChara::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AHeroChara::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AHeroChara::LMBUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AHeroChara::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AHeroChara::ESCUp);

	///Bind Axis Type
	PlayerInputComponent->BindAxis("MoveForward", this, &AHeroChara::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHeroChara::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AHeroChara::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AHeroChara::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AHeroChara::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AHeroChara::LookUpAtRate);


}

void AHeroChara::SaveGame()
{
	UMySaveGame* SaveGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));

	SaveGameInstance->CharacterStates.Health = Health;
	SaveGameInstance->CharacterStates.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStates.Stamina = Stamina;
	SaveGameInstance->CharacterStates.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStates.Coin = coins;

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStates.LevelName = MapName;

	if (EquippedWeapon) {
		SaveGameInstance->CharacterStates.WeaponName = EquippedWeapon->Name;
	}

	SaveGameInstance->CharacterStates.Location = GetActorLocation();
	SaveGameInstance->CharacterStates.Rotation = GetActorRotation();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance,SaveGameInstance->PlayerName,SaveGameInstance->UserIndex);
}

void AHeroChara::LoadGame(bool SetPosition)
{
	UMySaveGame* LoadGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
	
	LoadGameInstance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStates.Health;
	MaxHealth = LoadGameInstance->CharacterStates.MaxHealth;
	Stamina = LoadGameInstance->CharacterStates.Stamina;
	MaxStamina = LoadGameInstance->CharacterStates.MaxStamina;
	coins = LoadGameInstance->CharacterStates.Coin;

	if (WeaponStorage) {
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons) {

			FString WeaponName = LoadGameInstance->CharacterStates.WeaponName;
			if (Weapons->WeaponMap.Contains(WeaponName)) {
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}

	if (SetPosition) {
		SetActorLocation(LoadGameInstance->CharacterStates.Location);
		SetActorRotation(LoadGameInstance->CharacterStates.Rotation);
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	if (LoadGameInstance->CharacterStates.LevelName != TEXT("")) {

		FName LevelName(*LoadGameInstance->CharacterStates.LevelName);

		SwitchLevel(LevelName);

	}

	
}

void AHeroChara::LoadGameNoSwitch()
{
	UMySaveGame* LoadGameInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));

	LoadGameInstance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStates.Health;
	MaxHealth = LoadGameInstance->CharacterStates.MaxHealth;
	Stamina = LoadGameInstance->CharacterStates.Stamina;
	MaxStamina = LoadGameInstance->CharacterStates.MaxStamina;
	coins = LoadGameInstance->CharacterStates.Coin;

	if (WeaponStorage) {
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons) {

			FString WeaponName = LoadGameInstance->CharacterStates.WeaponName;
			if (Weapons->WeaponMap.Contains(WeaponName)) {
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}

void AHeroChara::SetEquippedWeapon(AWeapon * WeaponToSet)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = WeaponToSet;
}