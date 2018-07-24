// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpBase.generated.h"

UCLASS()
class ALSESCAPE_API APickUpBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickUpBase();

	UFUNCTION(BlueprintPure, Category = "Pickup")
		float GetPower();

	UFUNCTION(BlueprintPure, Category = "Pickup")
		FString GetType();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (BlueprintProtected = "True")) float PickupPower;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (BlueprintProtected = "True")) FString PickupType;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
