// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraComponent.h"

#include "Interfaces/HitInterface.h"
#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"

AWeapon::AWeapon() 
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());
	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());

	FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("Field System"));
	FieldSystem->SetupAttachment(GetRootComponent());
	RadialFalloff = CreateDefaultSubobject<URadialFalloff>(TEXT("Radial Falloff"));
	RadialVector = CreateDefaultSubobject<URadialVector>(TEXT("Radial Vector"));
	FieldSystemMetaDataFilter = CreateDefaultSubobject<UFieldSystemMetaDataFilter>(TEXT("Field System Meta Data Filter"));
	FieldSystemMetaDataFilter->ObjectType = EFieldObjectType::Field_Object_Destruction;

	/*
		IMPROVEMENT: Pass blueprint fields functionality to here, but it may be not a good idea because its
		components have a lot of properties that need test, so in the blueprint it is more practice to do it,
		although it is a good valid excersise

		DONE: It was a good experience to do that, it helped me to use the C++ Unreal Engine Documentation,
	*/
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	ItemState = EItemState::EIS_Equipped;
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
	DisableSphereCollision();
	PlayEquipSound();
	DeactivateEmbers();
}

void AWeapon::DeactivateEmbers()
{
	if (ItemEffect) {
		ItemEffect->Deactivate();
	}
}

void AWeapon::DisableSphereCollision()
{
	if (Sphere) {
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeapon::PlayEquipSound()
{
	if (EquipSound) {
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquipSound,
			GetActorLocation()
		);
	}
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//if (GetOwner() == OtherActor) return; // check if the Actor is hitting itself
	if (ActorIsSameType(OtherActor)) return;

	FHitResult BoxHit;
	BoxTrace(BoxHit);

	if (BoxHit.GetActor()) {
		if (ActorIsSameType(BoxHit.GetActor())) return;
		//if (GetOwner() == BoxHit.GetActor()) return;

		UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, GetInstigatorController(), this, UDamageType::StaticClass());
		ExecuteGetHit(BoxHit);
		CreateFields(BoxHit.ImpactPoint);
	}
}

bool AWeapon::ActorIsSameType(AActor* OtherActor)
{
	return GetOwner()->ActorHasTag(TEXT("Enemy")) == OtherActor->ActorHasTag(TEXT("Enemy"));
}

void AWeapon::CreateFields(const FVector& FieldLocation)
{
	RadialFalloff->SetRadialFalloff(
		700000.f,
		0.9f,
		1.f,
		0.f,
		125.f,
		FieldLocation,
		EFieldFalloffType::Field_FallOff_None
	);

	RadialVector->SetRadialVector(100000.f, FieldLocation);

	FieldSystem->ApplyPhysicsField(
		true,
		EFieldPhysicsType::Field_ExternalClusterStrain,
		nullptr,
		RadialFalloff
	);

	FieldSystem->ApplyPhysicsField(
		true,
		EFieldPhysicsType::Field_LinearForce,
		FieldSystemMetaDataFilter,
		RadialVector
	);
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());

	for (AActor* Actor : IgnoreActors) {
		ActorsToIgnore.AddUnique(Actor);
	}

	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		BoxTraceExtent,
		BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit,
		true
	);

	IgnoreActors.AddUnique(BoxHit.GetActor());
}

void AWeapon::ExecuteGetHit(FHitResult& BoxHit)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
	if (HitInterface) {
		HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
	}
}
