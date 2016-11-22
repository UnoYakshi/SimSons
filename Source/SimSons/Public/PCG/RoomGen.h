// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RoomGen.generated.h"

USTRUCT(Blueprintable)
struct FRoom
{
	GENERATED_USTRUCT_BODY()

	/// Room's physical data...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	int32 Square;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	int32 Width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	int32 Length;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	FVector Origin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	FVector End;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	FRoom* Parent;

	void Destroy()
	{
		Parent = nullptr;
	}
	void CalcSquare()
	{
		Square = Length * Width;
	}
	void CalcParams()
	{
		Length = End.X - Origin.X;
		Width = End.Y - Origin.Y;
		CalcSquare();
	}
	void SetParams(FVector NewOrigin, FVector NewEnd)
	{
		Origin = NewOrigin;
		End = NewEnd;
		CalcParams();
	}

	FVector GetIntersectionCenterWith(const FRoom AnotherRoom, FVector Tiles)
	{
		float TileX = Tiles.X;
		float TileY = Tiles.Y;
		float TileZ = Tiles.Z;

		/// Another room is above...
		if (End.Y == AnotherRoom.Origin.Y)
		{
			if (Origin.X + TileX < AnotherRoom.End.X &&
				End.X - TileX > AnotherRoom.Origin.X)
			{
				return FVector(
					(FMath::Max(Origin.X, AnotherRoom.Origin.X) + FMath::Min(End.X, AnotherRoom.End.X)) / 2.f,
					AnotherRoom.Origin.Y,
					0.f);
			}
		}
		/// Another room is below...
		else if (Origin.Y == AnotherRoom.End.Y)
		{
			if (Origin.X + TileX < AnotherRoom.End.X &&
				End.X - TileX > AnotherRoom.Origin.X)
			{
				return FVector(
					(FMath::Max(Origin.X, AnotherRoom.Origin.X) + FMath::Min(End.X, AnotherRoom.End.X)) / 2.f,
					AnotherRoom.End.Y,
					0.f);
			}
		}
		/// Another room is on the right side...
		else if (Origin.X == AnotherRoom.End.X)
		{
			if (Origin.Y + TileY < AnotherRoom.End.Y &&
				End.Y - TileY > AnotherRoom.Origin.Y)
			{
				return FVector(
					Origin.X,
					(FMath::Max(Origin.Y, AnotherRoom.Origin.Y) + FMath::Min(End.Y, AnotherRoom.End.Y)) / 2.f,
					0.f);
			}
		}
		/// Another room is on the left side...
		else if (End.X == AnotherRoom.Origin.X)
		{
			if (Origin.Y + TileY < AnotherRoom.End.Y &&
				End.Y - TileY > AnotherRoom.Origin.Y)
			{
				return FVector(
					End.X,
					(FMath::Max(Origin.Y, AnotherRoom.Origin.Y) + FMath::Min(End.Y, AnotherRoom.End.Y)) / 2.f,
					0.f);
			}
		}

		return FVector::ZeroVector;
	}

	FORCEINLINE bool operator==(const FRoom& Other)
	{
		return (
			Width == Other.Width &&
			Length == Other.Length &&
			Origin == Other.Origin &&
			End == Other.End
			) ? true : false;
	}

	/// c-tors
	FRoom() :
		Square(0),
		Origin(FVector::ZeroVector),
		End(FVector::ZeroVector),
		Width(0),
		Length(0),
		Parent(NULL)
	{}
	FRoom(FVector From, FVector To) : 
		Origin(From),
		End(To),
		Length(To.X - From.X),
		Width(To.Y - From.Y),
		Parent(NULL)
	{
		CalcSquare();
	}
	FRoom(FVector From, FVector To, FRoom* _Parent) :
		Origin(From),
		End(To),
		Length(To.X - From.X),
		Width(To.Y - From.Y),
		Parent(_Parent)
	{
		CalcSquare();
	}
};

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SIMSONS_API ARoomGen : public AActor
{
	GENERATED_BODY()
	
public:
	ARoomGen();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;


	///////////////////////////////////
	// PROPS
	///////////////////////////////////
	
	/// Number of rooms defined by user...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 RoomCount;

	/// Floors count...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 FloorCount;

	/// Use grid system?..
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bUseGrid;
	/// Generate floor?..
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bGenerateFloor;


	/// Room's min-max X...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MinRoomX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxRoomX;

	/// Room's min-max Y...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MinRoomY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxRoomY;
	
	/// Fund's values...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 FundX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 FundY;

	/// Floor's values...
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 FloorX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 FloorY;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 WallX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 WallY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 WallZ;


	// Workset...

	int32 RealRoomCount;
	int32 _RoomCount;
	int32 _FloorCount;

	TArray< FVector > RoomDotCoordinates;

	/// Binary array-based tree of Rooms...
	TArray< FRoom > Rooms;

	/// Temp room for processing...
	FRoom* CurrentRoom;

	/// If bUseGrid...
	TArray< TArray< FVector > > GridPoints;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray< FRoom > Restrictions;
	TArray< FVector > DoorPoints;

	// Content...

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UStaticMesh* SMeshFloor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UInstancedStaticMeshComponent* ISMeshCompFloor;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UStaticMesh* SMeshWall;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UInstancedStaticMeshComponent* ISMeshCompWall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UStaticMesh* SMeshDoorway;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UInstancedStaticMeshComponent* ISMeshCompDoorway;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UMaterial* Material;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	UMaterial* MaterialDoorway;



	///////////////////////////////////
	// FUNC
	///////////////////////////////////
	
	UFUNCTION(BlueprintCallable, Category = "CustomCat")
	void Start();
	void Init();

	UFUNCTION()
	void GenerateRooms();
	void GenerateDoorways();
	void GenerateWall(const FVector From, const FVector To, const FRotator Rot);


	// UTIL
	bool SatisfyLimits(const FVector PointToCheck, const FRoom* InRoom, const bool IsHorizontal);
	FVector GeneratePoint(const FVector From, const FVector To);
	void MakeHole(const FVector DoorPoint);
};
