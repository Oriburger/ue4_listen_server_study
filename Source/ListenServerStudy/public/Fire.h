// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Fire.generated.h"


/*
 - Name        : AFire
 - Descirption : FireHose�� ���� ������ ������ �� ����
 - Date        : 2022/06/22 LJH
*/


UCLASS(Category = "FireFighter")
class LISTENSERVERSTUDY_API AFire : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFire();

private:
	UPROPERTY(VisibleDefaultsOnly)
		USceneComponent* DefaultSceneRoot;

	UPROPERTY(EditAnywhere, Category = "VFX")
		UParticleSystemComponent* FireEmitter; //�� �̹���

	UPROPERTY(EditAnywhere, Category = "VFX")
		UParticleSystemComponent* SmokeEmitter; //���� ȿ�� �̹��� (ȭ�� ���� ���� ��)

	UPROPERTY(EditAnywhere, Category = "VFX")
		UMaterialInterface* SmokeMaterialInterface; //���� �̹��� ���� ��Ƽ���� 

	UPROPERTY()
		UMaterialInstanceDynamic* SmokeDynamicMaterial; //���� �̹���  Opacity ������ ���� ���̳��� ��Ƽ����

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay")
		float FireScaleSize = 1.0f;   //���� ũ�� (���� : 0<=size<=1)

	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay")
		float SmokeScaleSize = 0.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Emitter�� Scale�� ������Ʈ
	UFUNCTION(NetMulticast, reliable)//NetMulticast, Reliable)
		void MulticastUpdateEmitterScale(UParticleSystemComponent* target, const FVector& newScale);

	//Emitter�� Scale�� ������Ʈ 
	UFUNCTION(Server, reliable)
		void ServerRPCUpdateEmitterScale(UParticleSystemComponent* target, const FVector& newScale);

	//FireHose�� NS_Emiiter�� Apply�ϴ� �������� �޴´�. FireScaleSize�� DamageAmount ��ŭ ����.
	UFUNCTION()
		virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION()
		void TryDestroy();
};