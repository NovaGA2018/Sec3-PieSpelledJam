// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AlsEscapeGameMode.generated.h"


UCLASS(minimalapi)
class AAlsEscapeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAlsEscapeGameMode();
	virtual void BeginPlay() override;


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CustomWidget", meta = (BlueprintProtected = "true"))
		TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY()
		class UUserWidget* CurrentWidget;


};



