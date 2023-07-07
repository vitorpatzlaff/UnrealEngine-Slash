// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBar.generated.h"

class UProgressBar;

UCLASS()
class SLASH_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	/* 
		Bind this variable to a progress bar of the SAME name in the blueprint.
		Note that the widget blueprint MUST INHERIT from this class and the variable MUST be
		with the SAME name of the blueprint progress bar for the binding.
	*/ 
};
