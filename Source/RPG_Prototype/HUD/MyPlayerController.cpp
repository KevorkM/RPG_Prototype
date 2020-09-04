// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include <Blueprint/UserWidget.h>


void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}
	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);

	if (WEnemyHealthBar) {
		EnemyHealthBar = CreateWidget<UUserWidget>(this, WEnemyHealthBar);
		if (EnemyHealthBar) {
			EnemyHealthBar->AddToViewport();
			EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignement(0.f, 0.f);
		EnemyHealthBar->SetAlignmentInViewport(Alignement);
	}
	if (WPauseMenu) {
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu) {
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
		
	}

}

void AMyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyHealthBar) {
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
		PositionInViewport.Y -= 90.f;


		FVector2D SizeInViewport = FVector2D(300.f, 25.f);

		EnemyHealthBar->SetPositionInViewport(PositionInViewport);
		EnemyHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}
}

void AMyPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu) {

		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputModeGameAndUIOnly;

		SetInputMode(InputModeGameAndUIOnly);

		bShowMouseCursor = true;

	}

}

void AMyPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu) {

		GameModeOnly();

		bShowMouseCursor = false;

		bPauseMenuVisible = false;	
	}
}

void AMyPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible) {
		RemovePauseMenu();
	}
	else {
		DisplayPauseMenu();
	}
}

void AMyPlayerController::DisplayEnemyHealthBar()
{
	bEnemyHealthBarVisible = true;
	EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);
}

void AMyPlayerController::RemoveEnemyHealthBar()
{
	bEnemyHealthBarVisible = false;
	EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
}

void AMyPlayerController::GameModeOnly()
{
	FInputModeGameOnly InputModeGameOnly;

	SetInputMode(InputModeGameOnly);
}
