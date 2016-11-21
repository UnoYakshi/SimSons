// Fill out your copyright notice in the Description page of Project Settings.

#include "SimSons.h"
#include "PGWall.h"


APGWall::APGWall()
{
	PrimaryActorTick.bCanEverTick = true;

	//RootComponent = CreateDefaultSubobject< USceneComponent >(TEXT("RootSceneComp"));
	//RootComponent->SetHiddenInGame(false);

	ISMeshComp = CreateDefaultSubobject < UInstancedStaticMeshComponent >(TEXT("ISMeshComp"));
	//USceneComponent* RootScene = CreateDefaultSubobject< USceneComponent >(TEXT("Root"));//= ISMeshComp;
	RootComponent = ISMeshComp;//RootScene;
	//ISMeshComp->AttachTo(RootComponent);

	
	//Mobility
	ISMeshComp->SetMobility(EComponentMobility::Movable);

	/*
	ISMeshComp->bOwnerNoSee = false;
	ISMeshComp->bCastDynamicShadow = false;
	ISMeshComp->CastShadow = true;
	ISMeshComp->BodyInstance.SetObjectType(ECC_WorldDynamic);
	//Visibility
	//ISMeshComp->SetHiddenInGame(false);
	//Collision
	ISMeshComp->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ISMeshComp->BodyInstance.SetObjectType(ECC_WorldDynamic);
	ISMeshComp->BodyInstance.SetResponseToAllChannels(ECR_Ignore);
	ISMeshComp->BodyInstance.SetResponseToChannel(ECC_WorldStatic, ECR_Block);
	ISMeshComp->BodyInstance.SetResponseToChannel(ECC_WorldDynamic, ECR_Block);
	//*/

	SMesh = CreateDefaultSubobject < UStaticMesh >(TEXT("SMesh"));
	Material = CreateDefaultSubobject < UMaterial >(TEXT("Material"));

	XYRadiusVect.Set(1.f, 1.f, 0.f);
	//ZeroVect.Set(0.f, 0.f, 0.f);
}
void APGWall::BeginPlay()
{
	Super::BeginPlay();
	
}
void APGWall::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}
#if WITH_EDITOR
void APGWall::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);

	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(float, XLen))
	//{
		Generate();
	//}
}
#endif
void APGWall::Generate()
{
	if (FMath::Frac(EndPoint.X) != 0)
	{
		EndPoint.X = FMath::Floor(EndPoint.X);
	}
	if (FMath::Frac(EndPoint.Y) != 0)
	{
		EndPoint.X = FMath::Floor(EndPoint.Y);
	}

	if (EndPoint * XYRadiusVect != FVector::ZeroVector && ISMeshComp)
	{
		GLog->Log("Change");
		ISMeshComp->ClearInstances();

		if (SMesh)
		{
			ISMeshComp->SetStaticMesh(SMesh);
		}
		if (Material)
		{
			ISMeshComp->SetMaterial(0, Material);
		}

		XNum = FMath::FloorToInt(EndPoint.X / XLen);
		YNum = FMath::FloorToInt(EndPoint.Y / YLen);

		for (int32 i = 0; i < XNum; ++i)
		{

			for (int32 j = 0; j < YNum; ++j)
			{
				/// Check if the current mesh's location is on the rectangle's edges...
				CurPoint.Set(XLen * i, YLen * j, 0);
				if (IsOnEdges(CurPoint, RootComponent->GetComponentLocation(), FVector(EndPoint.X - XLen, EndPoint.Y - YLen, 0)))
				{
					FTransform NewTransf;
					NewTransf.SetLocation(CurPoint);
					ISMeshComp->AddInstance(NewTransf);
				}
			}
		}
	}
}
void APGWall::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	/*
	if (SMesh)
	{
	ISMeshComp->SetStaticMesh(SMesh);
	}
	if (Material)
	{
	ISMeshComp->SetMaterial(0, Material);
	}
	//*/
	
}

bool APGWall::IsOnEdges(FVector PointToCheck, FVector LeftBotPoint, FVector RightTopPoint)
{
	if (
		IsOnLine(PointToCheck, LeftBotPoint, FVector(LeftBotPoint.X, RightTopPoint.Y, 0))
		|| IsOnLine(PointToCheck, FVector(LeftBotPoint.X, RightTopPoint.Y, 0), RightTopPoint)
		|| IsOnLine(PointToCheck, RightTopPoint, FVector(RightTopPoint.X, LeftBotPoint.Y, 0))
		|| IsOnLine(PointToCheck, FVector(RightTopPoint.X, LeftBotPoint.Y, 0), LeftBotPoint)
		)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool APGWall::IsOnLine(FVector ToCheck, FVector PointOne, FVector PointTwo)
{
	float MinX = FMath::Min(PointOne.X, PointTwo.X);
	float MaxX = FMath::Max(PointOne.X, PointTwo.X);

	float MinY = FMath::Min(PointOne.Y, PointTwo.Y);
	float MaxY = FMath::Max(PointOne.Y, PointTwo.Y);

	float CurX = ToCheck.X;
	float CurY = ToCheck.Y;

	/// If it's either on the horizontal, or vertical line...
	if ((CurX == MinX && CurX == MaxX && MinY <= CurY && CurY <= MaxY)//
		|| (CurY == MinY && CurY == MaxY && MinX <= CurX && CurX <= MaxX))//
	{
		return true;
	}
	else
	{
		return false;
	}
}
