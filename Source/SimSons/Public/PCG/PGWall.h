// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PGWall.generated.h"

UCLASS()
class SIMSONS_API APGWall : public AActor
{
	GENERATED_BODY()
	
public:	
	APGWall();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;

	////////////////////////////////////
	// PROPS
	////////////////////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", Meta = (MakeEditWidget = true))
	FVector EndPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float XLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float YLen;

	int32 XNum;
	int32 YNum;
	FTimerHandle AlgoTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UStaticMesh* SMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UMaterial* Material;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UInstancedStaticMeshComponent* ISMeshComp;


	FVector XYRadiusVect;//(1.f, 1.f, 0.f);
	FVector ZeroVect;// (0.f, 0.f, 0.f);
	FVector CurPoint;


	// UTIL
	void Generate();
	bool IsOnEdges(FVector PointToCheck, FVector LeftBotPoint, FVector RightTopPoint);
	bool IsOnLine(FVector ToCheck, FVector PointOne, FVector PointTwo);

};
