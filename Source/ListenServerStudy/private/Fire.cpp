// Fill out your copyright notice in the Description page of Project Settings.

#include "Fire.h"
#include "ListenServerStudy.h"
#include "TimerManager.h"

// Sets default values
AFire::AFire()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->Tags = { "Fire" };
	bReplicates = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DEFAULT_SCENE_ROOT"));
	DefaultSceneRoot->SetupAttachment(RootComponent);

	FireEmitter = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FIRE_EMITTER"));
	FireEmitter->SetupAttachment(DefaultSceneRoot);

	SmokeEmitter = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SMOKE_EMITTER"));
	SmokeEmitter->SetupAttachment(DefaultSceneRoot);
	SmokeEmitter->SetWorldScale3D({ 0.0f, 0.0f, 0.0f });
	if(SmokeMaterialInterface != nullptr) //BP에서 Mat Interface를 지정해주었다면
		SmokeDynamicMaterial = UMaterialInstanceDynamic::Create(SmokeMaterialInterface, this);
	SmokeEmitter->SetMaterial(0, SmokeDynamicMaterial);
}

// Called when the game starts or when spawned
void AFire::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority()) //서버에서 실행되는 경우?
	{
		MulticastUpdateEmitterScale(FireEmitter, { FireScaleSize, FireScaleSize, FireScaleSize }); //Multicast 호출
		MulticastUpdateEmitterScale(SmokeEmitter, { SmokeScaleSize, SmokeScaleSize, SmokeScaleSize });
	}
	else //클라이언트에서 실행되는 경우?
	{
		ServerRPCUpdateEmitterScale(FireEmitter, { FireScaleSize, FireScaleSize, FireScaleSize }); //Server RPC 호출
		ServerRPCUpdateEmitterScale(SmokeEmitter, { SmokeScaleSize, SmokeScaleSize, SmokeScaleSize });
	}	
}

void AFire::MulticastUpdateEmitterScale_Implementation(UParticleSystemComponent* target, const FVector& newScale)
{
	if (newScale.Y < 0.0f || newScale.Y > 3.0f) return;
	target->SetWorldScale3D(FMath::VInterpTo(target->GetComponentScale(), newScale, 0.1f, 0.25f)); //매 Tick 마다 FireScaleSize를 업데이트한다.
}

void AFire::ServerRPCUpdateEmitterScale_Implementation(UParticleSystemComponent* target, const FVector& newScale)
{
	MulticastUpdateEmitterScale(target, newScale);
}

float AFire::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageCauser == nullptr && !DamageCauser->ActorHasTag("FireHose")) return 0; //DamageCause이 유효하지 않거나 FireHose가 아니라면 return 
	if (FireScaleSize == 0) TryDestroy(); //불이 모두 꺼졌다면 Destroy

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser); //Super 클래스의 TakeDamage 호출

	FireScaleSize = FMath::Max(0.0f, FireScaleSize - DamageAmount * FMath::FRandRange(1.25f, 1.75f));  //FireScale을 DamageAmount만큼 뺌, (자연스러운 전환위해 rand값 추가)
	SmokeScaleSize = FMath::Min(3.0f, SmokeScaleSize + DamageAmount * FMath::FRandRange(3.0f, 5.0f));  //SmokeScale값도 위와 마찬가지로 업데이트

	return DamageAmount;
}

void AFire::TryDestroy()
{
	FTimerHandle DestroyTimerHandle;
	const float SmokeLifeSpan = 15.0f; //연기의 Life (특정 시간만큼 Opacity 업데이트)
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, 1.0f, true, 200.0f); //타이머 설정

	//if(SmokeDynamicMaterial == nullptr) 
	//	GetWorldTimerManager().ClearTimer(DestroyTimerHandle);

	float OpacityValue = 1.0f; //초기 Opacity

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
			FString::Printf(TEXT("%.5lf"), GetWorldTimerManager().GetTimerRemaining(DestroyTimerHandle)));

	while (GetWorldTimerManager().IsTimerActive(DestroyTimerHandle)) //Timer이 끝날때까지 반복
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, 
					 FString::Printf(TEXT("%.5lf"), GetWorldTimerManager().GetTimerRemaining(DestroyTimerHandle)));

		//Timer이 끝날때까지 반복
		SmokeDynamicMaterial->SetScalarParameterValue("Opacity", OpacityValue);
		OpacityValue -= 0.1f;
	}

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
			FString::Printf(TEXT("%.5lf"), GetWorldTimerManager().GetTimerRemaining(DestroyTimerHandle)));

	Destroy();
}