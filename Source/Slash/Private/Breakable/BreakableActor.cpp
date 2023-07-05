// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CapsuleComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Breakable/BreakableActor.h"
#include "Items/Treasure.h"

// Sets default values
ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	SetRootComponent(GeometryCollection);
	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	HasBeenHit = false;
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint)
{
	FVector Location = GetActorLocation();

	if (BreakSound) {
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BreakSound,
			Location
		);
	}
	// can be in the blueprint

	UWorld* World = GetWorld();

	if (!HasBeenHit && World && TreasureClasses.Num() > 0) {
		Location.Z += 75.f;

		const int8 Selection = FMath::RandRange(0, TreasureClasses.Num() - 1);
		World->SpawnActor<ATreasure>(TreasureClasses[Selection], Location, GetActorRotation());

		HasBeenHit = true;

		Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		//SetLifeSpan(3.f);
		/* 
			This two above things can be done on the OnChaosBreakEvent.
			Further info, the GetHit_Implementation captures only hits from the player, now this event captures anything
			that breaks the "GeometryCollection"
		*/
	}
}
