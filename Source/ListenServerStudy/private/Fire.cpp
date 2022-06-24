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
	if(SmokeMaterialInterface != nullptr) //BP���� Mat Interface�� �������־��ٸ�
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

	if (HasAuthority()) //�������� ����Ǵ� ���?
	{
		MulticastUpdateEmitterScale(FireEmitter, { FireScaleSize, FireScaleSize, FireScaleSize }); //Multicast ȣ��
		MulticastUpdateEmitterScale(SmokeEmitter, { SmokeScaleSize, SmokeScaleSize, SmokeScaleSize });
	}
	else //Ŭ���̾�Ʈ���� ����Ǵ� ���?
	{
		ServerRPCUpdateEmitterScale(FireEmitter, { FireScaleSize, FireScaleSize, FireScaleSize }); //Server RPC ȣ��
		ServerRPCUpdateEmitterScale(SmokeEmitter, { SmokeScaleSize, SmokeScaleSize, SmokeScaleSize });
	}	
}

void AFire::MulticastUpdateEmitterScale_Implementation(UParticleSystemComponent* target, const FVector& newScale)
{
	if (newScale.Y < 0.0f || newScale.Y > 3.0f) return;
	target->SetWorldScale3D(FMath::VInterpTo(target->GetComponentScale(), newScale, 0.1f, 0.25f)); //�� Tick ���� FireScaleSize�� ������Ʈ�Ѵ�.
}

void AFire::ServerRPCUpdateEmitterScale_Implementation(UParticleSystemComponent* target, const FVector& newScale)
{
	MulticastUpdateEmitterScale(target, newScale);
}

float AFire::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageCauser == nullptr && !DamageCauser->ActorHasTag("FireHose")) return 0; //DamageCause�� ��ȿ���� �ʰų� FireHose�� �ƴ϶�� return 
	if (FireScaleSize == 0) TryDestroy(); //���� ��� �����ٸ� Destroy

	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser); //Super Ŭ������ TakeDamage ȣ��

	FireScaleSize = FMath::Max(0.0f, FireScaleSize - DamageAmount * FMath::FRandRange(1.25f, 1.75f));  //FireScale�� DamageAmount��ŭ ��, (�ڿ������� ��ȯ���� rand�� �߰�)
	SmokeScaleSize = FMath::Min(3.0f, SmokeScaleSize + DamageAmount * FMath::FRandRange(3.0f, 5.0f));  //SmokeScale���� ���� ���������� ������Ʈ

	return DamageAmount;
}

void AFire::TryDestroy()
{
	FTimerHandle DestroyTimerHandle;
	const float SmokeLifeSpan = 15.0f; //������ Life (Ư�� �ð���ŭ Opacity ������Ʈ)
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, 1.0f, true, 200.0f); //Ÿ�̸� ����

	//if(SmokeDynamicMaterial == nullptr) 
	//	GetWorldTimerManager().ClearTimer(DestroyTimerHandle);

	float OpacityValue = 1.0f; //�ʱ� Opacity

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
			FString::Printf(TEXT("%.5lf"), GetWorldTimerManager().GetTimerRemaining(DestroyTimerHandle)));

	while (GetWorldTimerManager().IsTimerActive(DestroyTimerHandle)) //Timer�� ���������� �ݺ�
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, 
					 FString::Printf(TEXT("%.5lf"), GetWorldTimerManager().GetTimerRemaining(DestroyTimerHandle)));

		//Timer�� ���������� �ݺ�
		SmokeDynamicMaterial->SetScalarParameterValue("Opacity", OpacityValue);
		OpacityValue -= 0.1f;
	}

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
			FString::Printf(TEXT("%.5lf"), GetWorldTimerManager().GetTimerRemaining(DestroyTimerHandle)));

	Destroy();
}