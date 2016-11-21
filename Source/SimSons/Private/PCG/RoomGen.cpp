// Fill out your copyright notice in the Description page of Project Settings.

#include "SimSons.h"
#include "RoomGen.h"

#define GenerateVector(From, To) FVector(FMath::RandRange(From.X, To.X), FMath::RandRange(From.Y, To.Y), FMath::RandRange(From.Z, To.Z));
#define RG_INF_LOOP_MAX 1024

ARoomGen::ARoomGen()
{
	PrimaryActorTick.bCanEverTick = true;

	ISMeshCompFloor = CreateDefaultSubobject < UInstancedStaticMeshComponent >(TEXT("ISMeshCompFloor"));
	ISMeshCompWall = CreateDefaultSubobject < UInstancedStaticMeshComponent >(TEXT("ISMeshCompWall"));
	ISMeshCompDoorway = CreateDefaultSubobject < UInstancedStaticMeshComponent >(TEXT("ISMeshCompDoorway"));

	RootComponent = ISMeshCompFloor;

	ISMeshCompWall->AttachToComponent(
		ISMeshCompFloor,
		FAttachmentTransformRules::KeepRelativeTransform,
		NAME_None
	);
	ISMeshCompDoorway->AttachToComponent(
		ISMeshCompFloor,
		FAttachmentTransformRules::KeepRelativeTransform,
		NAME_None
	);

	bUseGrid = false;
	
}

void ARoomGen::Init()
{
	GLog->Log("RoomGen :: Initialization...");

	/// Clear all the data...
	ISMeshCompFloor->ClearInstances();
	ISMeshCompWall->ClearInstances();
	ISMeshCompDoorway->ClearInstances();
	Rooms.Empty();
	if (bUseGrid)
	{
		GridPoints.Empty();
	}

	/// Set the content up...
	if (SMeshFloor)
	{
		ISMeshCompFloor->SetStaticMesh(SMeshFloor);
	}
	if (SMeshWall)
	{
		ISMeshCompWall->SetStaticMesh(SMeshWall);
	}
	if (SMeshDoorway)
	{
		ISMeshCompDoorway->SetStaticMesh(SMeshDoorway);
	}
	if (Material)
	{
		ISMeshCompFloor->SetMaterial(0, Material);
	}
	if (MaterialDoorway)
	{
		ISMeshCompDoorway->SetMaterial(0, MaterialDoorway);
	}

	/// Compute number of tiles given by Fund values...
	GLog->Log("RoomGen :: Floor generation...");
	int32 XNum = (FloorX != 0) ? FMath::FloorToInt(FundX / FloorX) : 0;
	int32 YNum = (FloorY != 0) ? FMath::FloorToInt(FundY / FloorY) : 0;

	/// Fill the Grid with points...
	if (bUseGrid)
	{
		FVector NewPoint = FVector::ZeroVector;
		for (int32 i = 0; i < XNum; ++i)
		{
			GridPoints.Add(TArray<FVector>());
			NewPoint.X = i * FloorX;
			
			for (int32 j = 0; j < YNum; ++j)
			{
				NewPoint.Y = j * FloorY;
				GridPoints[i].Add(NewPoint);
			}
		}

		FTransform NewTransf;
		for (auto i : GridPoints)
		{
			for (auto j : i)
			{
				NewTransf.SetLocation(j);
				ISMeshCompFloor->AddInstance(NewTransf);
			}
		}
	}
	else
	{
		/// Generating floor's instances w/o considering grid...
		for (int32 i = 0; i < XNum; ++i)
		{
			for (int32 j = 0; j < YNum; ++j)
			{
				FTransform NewTransf;
				NewTransf.SetLocation(FVector(FloorX * i, FloorY * j, 0));
				ISMeshCompFloor->AddInstance(NewTransf);

				GLog->Log("RoomGen :: Tile...");
			}
		}
	}


	GLog->Log("RoomGen :: First room...");
	//Rooms.SetNum(FMath::RoundUpToPowerOfTwo(_RoomCount)); //, false);

	CurrentRoom = new FRoom(FVector::ZeroVector, FVector(FundX, FundY, 0), NULL);
	CurrentRoom->CalcParams();
	FRoom FR = *CurrentRoom;
	Rooms.Add(*CurrentRoom);

	GLog->Log("RoomGen :: Initialization completed...");
}
void ARoomGen::BeginPlay()
{
	Super::BeginPlay();

}
void ARoomGen::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
#if WITH_EDITOR
void ARoomGen::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);

	//FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	//if (PropertyName == GET_MEMBER_NAME_CHECKED(float, XLen))
	//{
	if (FloorX != 0 && FloorY != 0 &&
		WallX != 0 && WallY != 0)
	{
		_RoomCount = RoomCount - 1;
		Init();
		GenerateRooms();
		GenerateDoorways();
	}
	//}
}
#endif


void ARoomGen::GenerateRooms()
{
	bool IsHorizontal = true;
	FVector TempDot;
	FRotator TempRot(0.f, 0.f, 0.f);

	/// Working area for current room...
	FVector Start(0, 0, 0);
	FVector End(FundX, FundY, 0);

	/// Dots for wall generation...
	FVector From = Start;
	FVector To = End;

	/// Children of CurrentRoom...
	FRoom* FirstRoom = new FRoom(From, To);
	FRoom* SecondRoom = new FRoom(From, To);


	for (int32 RoomNumber = _RoomCount - 1; RoomNumber >= 0; --RoomNumber)
	{
		GLog->Log(FString::Printf(_T("\n ---- \n %d :: Generate Room"), (_RoomCount - RoomNumber)));

		//*
		/// Check if CurrentRoom can handle new ones...
		if (CurrentRoom->Width < 2 * MinRoomY || CurrentRoom->Length < 2 * MinRoomX)
		{
			GLog->Log("RoomGen :: Not ENOUGH SPACE...");

			/// Find max square room...
			int32 MaxSquare = 0;
			int32 MaxSquareRoom = 0;
			for (FRoom it : Rooms)
			{
				if (it.Length >= MinRoomX &&
					it.Width >= MinRoomY &&
					it.Square > MaxSquare)
				{
					MaxSquare = it.Square;
					*CurrentRoom = it;
				}
			}

			/// Assign max square room's to current coordinates...
			Start = CurrentRoom->Origin;
			End = CurrentRoom->End;
		}
		//*/

		/// Choose random point...
		int32 InfLoopCounter = 0;
		if (!bUseGrid)
		{
			do
			{
				GLog->Log("Dot :: New");

				if (++InfLoopCounter > RG_INF_LOOP_MAX)
				{
					GLog->Log("RoomGen :: Current config led to almost inf. loop...");
					GLog->Log("RoomGen :: Change the config's parameters...");
					return;
				}

				TempDot = GenerateVector(From, To)
			} while (!SatisfyLimits(TempDot, CurrentRoom, IsHorizontal));
		}
		else
		{
			do
			{
				GLog->Log("Dot :: New");

				if (++InfLoopCounter > RG_INF_LOOP_MAX)
				{
					GLog->Log("RoomGen :: Current config led to almost inf. loop...");
					GLog->Log("RoomGen :: Change the config's parameters...");
					return;
				}
				TempDot = GeneratePoint(From, To);
			} while (!SatisfyLimits(TempDot, CurrentRoom, IsHorizontal));
		}
		GLog->Log(FString::Printf(_T("Dot :: %d_%d"), (int32)TempDot.X, (int32)TempDot.Y));
		RoomDotCoordinates.Add(TempDot);


		/*
			GLog->Log(FString::Printf(_T("CURRENT ROOM: (%d ; %d) (%d ; %d)"), (int32)CurrentRoom->Origin.X, (int32)CurrentRoom->Origin.Y, (int32)CurrentRoom->End.X, (int32)CurrentRoom->End.Y));
			//GLog->Log(FString::Printf(_T("CURRENT ROOM: (%d ; %d) (%d ; %d)"), (int32)CurrentRoom->Origin.X, (int32)CurrentRoom->Origin.Y, (int32)CurrentRoom->End.X, (int32)CurrentRoom->End.Y));
			//GLog->Log(FString::Printf(_T("CURRENT ROOM: %d x %d\n"), (int32)CurrentRoom->Length, (int32)CurrentRoom->Width));

			GLog->Log(FString::Printf(_T("1 ROOM: (%d ; %d) (%d ; %d)"), (int32)FirstRoom->Origin.X, (int32)FirstRoom->Origin.Y, (int32)FirstRoom->End.X, (int32)FirstRoom->End.Y));
			//GLog->Log(FString::Printf(_T("1 ROOM: (%d ; %d) (%d ; %d)"), (int32)FirstRoom.Origin.X, (int32)FirstRoom.Origin.Y, (int32)FirstRoom.End.X, (int32)FirstRoom.End.Y));
			//GLog->Log(FString::Printf(_T("1 ROOM: %d x %d\n"), (int32)FirstRoom.Length, (int32)FirstRoom.Width));

			GLog->Log(FString::Printf(_T("2 ROOM: (%d ; %d) (%d ; %d)"), (int32)SecondRoom->Origin.X, (int32)SecondRoom->Origin.Y, (int32)SecondRoom->End.X, (int32)SecondRoom->End.Y));
			//GLog->Log(FString::Printf(_T("2 ROOM: (%d ; %d) (%d ; %d)"), (int32)SecondRoom.Origin.X, (int32)SecondRoom.Origin.Y, (int32)SecondRoom.End.X, (int32)SecondRoom.End.Y));
			//GLog->Log(FString::Printf(_T("2 ROOM: %d x %d\n\n"), (int32)SecondRoom.Length, (int32)SecondRoom.Width));
			//*/

		/// Set new rooms' data...
		if (IsHorizontal)
			{
				//FirstRoom.SetParams(Start, FVector(End.X, TempDot.Y, 0));
				//SecondRoom.SetParams(FVector(Start.X, TempDot.Y, 0), End);
				FirstRoom->SetParams(Start, FVector(End.X, TempDot.Y, 0));
				SecondRoom->SetParams(FVector(Start.X, TempDot.Y, 0), End);
			}
		else
			{
				//FirstRoom.SetParams(Start, FVector(TempDot.X, End.Y, 0));
				//SecondRoom.SetParams(FVector(TempDot.X, Start.Y, 0), End);
				FirstRoom->SetParams(Start, FVector(TempDot.X, End.Y, 0));
				SecondRoom->SetParams(FVector(TempDot.X, Start.Y, 0), End);
			}


		/*
			GLog->Log(FString::Printf(_T("CURRENT ROOM: (%d ; %d) (%d ; %d)"), (int32)CurrentRoom->Origin.X, (int32)CurrentRoom->Origin.Y, (int32)CurrentRoom->End.X, (int32)CurrentRoom->End.Y));
			//GLog->Log(FString::Printf(_T("CURRENT ROOM: %d x %d\n"), (int32)CurrentRoom->Length, (int32)CurrentRoom->Width));

			GLog->Log(FString::Printf(_T("1 ROOM: (%d ; %d) (%d ; %d)"), (int32)FirstRoom.Origin.X, (int32)FirstRoom.Origin.Y, (int32)FirstRoom.End.X, (int32)FirstRoom.End.Y));
			//GLog->Log(FString::Printf(_T("1 ROOM: %d x %d\n"), (int32)FirstRoom.Length, (int32)FirstRoom.Width));

			GLog->Log(FString::Printf(_T("2 ROOM: (%d ; %d) (%d ; %d)"), (int32)SecondRoom.Origin.X, (int32)SecondRoom.Origin.Y, (int32)SecondRoom.End.X, (int32)SecondRoom.End.Y));
			//GLog->Log(FString::Printf(_T("2 ROOM: %d x %d\n\n"), (int32)SecondRoom.Length, (int32)SecondRoom.Width));
			//*/

		/// Delete current old, rusty room, and...
		//*
		for (int32 i = Rooms.Num() - 1; i >= 0; --i)
		{
			if (Rooms[i] == *CurrentRoom)
			{
				//GLog->Log(FString::Printf(_T("Delete :: (%d ; %d) (%d ; %d)"), (int32)Rooms[i].Origin.X, (int32)Rooms[i].Origin.Y, (int32)Rooms[i].End.X, (int32)Rooms[i].End.Y));
				Rooms.RemoveAt(i, 1, true);
				//GLog->Log(FString::Printf(_T("Delete :: Deleted old room at %d"), i));
			}
		}
		//*/

		/// Add new ones...
		Rooms.Add(*FirstRoom);
		Rooms.Add(*SecondRoom);

		/// Set From, To, Start, and End points...
		if (IsHorizontal)
			{
				TempRot.Yaw = 0.f;

				From.X = Start.X;
				From.Y = TempDot.Y;
				To.X = End.X;
				To.Y = TempDot.Y;

				if ((End.Y - Start.Y) / 2 > TempDot.Y - Start.Y)
				{
					Start.Y = TempDot.Y;
					//CurrentRoom = &SecondRoom;
					*CurrentRoom = *SecondRoom;
				}
				else
				{
					End.Y = TempDot.Y;
					//CurrentRoom = &FirstRoom;
					*CurrentRoom = *FirstRoom;
				}
			}
		else
			{
				TempRot.Yaw = 90.f;

				From.X = TempDot.X;
				From.Y = Start.Y;
				To.X = TempDot.X;
				To.Y = End.Y;

				if ((End.X - Start.X) / 2 > TempDot.X - Start.X)
				{
					Start.X = TempDot.X;
					//CurrentRoom = &SecondRoom;
					*CurrentRoom = *SecondRoom;
				}
				else
				{
					End.X = TempDot.X;
					//CurrentRoom = &FirstRoom;
					*CurrentRoom = *FirstRoom;
				}
			}
		IsHorizontal = !IsHorizontal;

		/*
			GLog->Log(FString::Printf(_T("CURRENT ROOM: (%d ; %d) (%d ; %d)"), (int32)CurrentRoom->Origin.X, (int32)CurrentRoom->Origin.Y, (int32)CurrentRoom->End.X, (int32)CurrentRoom->End.Y));
			//GLog->Log(FString::Printf(_T("CURRENT ROOM: (%d ; %d) (%d ; %d)"), (int32)CurrentRoom->Origin.X, (int32)CurrentRoom->Origin.Y, (int32)CurrentRoom->End.X, (int32)CurrentRoom->End.Y));
			//GLog->Log(FString::Printf(_T("CURRENT ROOM: %d x %d\n"), (int32)CurrentRoom->Length, (int32)CurrentRoom->Width));

			GLog->Log(FString::Printf(_T("1 ROOM: (%d ; %d) (%d ; %d)"), (int32)FirstRoom->Origin.X, (int32)FirstRoom->Origin.Y, (int32)FirstRoom->End.X, (int32)FirstRoom->End.Y));
			//GLog->Log(FString::Printf(_T("1 ROOM: (%d ; %d) (%d ; %d)"), (int32)FirstRoom.Origin.X, (int32)FirstRoom.Origin.Y, (int32)FirstRoom.End.X, (int32)FirstRoom.End.Y));
			//GLog->Log(FString::Printf(_T("1 ROOM: %d x %d\n"), (int32)FirstRoom.Length, (int32)FirstRoom.Width));

			GLog->Log(FString::Printf(_T("2 ROOM: (%d ; %d) (%d ; %d)"), (int32)SecondRoom->Origin.X, (int32)SecondRoom->Origin.Y, (int32)SecondRoom->End.X, (int32)SecondRoom->End.Y));
			//GLog->Log(FString::Printf(_T("2 ROOM: (%d ; %d) (%d ; %d)"), (int32)SecondRoom.Origin.X, (int32)SecondRoom.Origin.Y, (int32)SecondRoom.End.X, (int32)SecondRoom.End.Y));
			//GLog->Log(FString::Printf(_T("2 ROOM: %d x %d\n\n"), (int32)SecondRoom.Length, (int32)SecondRoom.Width));
			//*/

		/*
			/// Set new rooms' data...
			FirstRoom.Origin = Start;
			FirstRoom.End = To;
			FirstRoom.CalcParams();

			SecondRoom.Origin = From;
			SecondRoom.End = End;
			SecondRoom.CalcParams();

			/// Add new rooms...
			Rooms.Add(FirstRoom);
			Rooms.Add(SecondRoom);
			//*/

		GLog->Log(FString::Printf(_T("RoomGen :: Rooms Array %d"), (int32)Rooms.Num()));

		GenerateWall(From, To, TempRot);
	}

	GLog->Log("RoomGen :: Completed");
}

void ARoomGen::GenerateWall(const FVector From, const FVector To, const FRotator Rot)
{
	GLog->Log("Wall :: Starting...");

	bool EqualX = (From.X == To.X);
	bool EqualY = (From.Y == To.Y);

	if (EqualX && EqualY)
	{
		GLog->Log("Wall :: Nothing to build...");
		return;
	}

	//GLog->Log(FString::Printf(_T("Wall :: (%d ; %d) (%d ; %d)\n"), (int32)From.X, (int32)From.Y, (int32)To.X, (int32)To.Y));


	int32 BlockX = WallX;
	int32 BlockY = WallY;

	int32 MinX = FMath::Min(From.X, To.X);
	int32 MaxX = FMath::Max(From.X, To.X);
	int32 MinY = FMath::Min(From.Y, To.Y);
	int32 MaxY = FMath::Max(From.Y, To.Y);

	/// It's to deal with different dots' position against each other...
	bool FromXGreat = (From.X > To.X);
	bool FromXLess = (From.X < To.X);
	bool FromYGreat = (From.Y > To.Y);
	bool FromYLess = (From.Y < To.Y);

	int32 FromX, FromY, ToX, ToY;

	//*
	/// Generate the wall...
	if (EqualX)
	{
		for (int32 j = MinY; j < MaxY; j += BlockX)
		{
			//FTransform NewTransform(Rot.Quaternion(), FVector(From.X, (float)j, 0.f));
			//*
			FTransform NewTransform(Rot); //= GetTransform();
										  //NewTransform.SetRotation(Rot.Quaternion());
			NewTransform.SetLocation(FVector(From.X, (float)j, 0.f));
			//NewTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
			//*/
			ISMeshCompWall->AddInstance(NewTransform);
		}
	}
	else if (EqualY)
	{
		for (int32 i = MinX; i < MaxX; i += BlockY)
		{
			//FTransform NewTransform(Rot.Quaternion(), FVector((float)i, From.Y, 0.f));

			//*
			FTransform NewTransform(Rot); //= GetTransform();
										  //NewTransform.SetRotation(Rot.Quaternion());
			NewTransform.SetLocation(FVector((float)i, From.Y, 0.f));
			//*/
			ISMeshCompWall->AddInstance(NewTransform);
		}
	}
	else
	{
		if (FromXLess)
		{
			FromX = From.X;
			ToX = To.X;

			if (FromYLess)
			{
				FromY = From.Y;
				ToY = To.Y;
			}
			else
			{
				FromY = To.Y;
				ToY = From.Y;

				BlockY *= -1;
			}
		}
		else
		{
			FromX = To.X;
			ToX = From.X;

			if (FromYLess)
			{
				FromY = From.Y;
				ToY = To.Y;
			}
			else
			{
				FromY = To.Y;
				ToY = From.Y;

				BlockY *= -1;
			}

			BlockX *= -1;
		}

		/// Generate instances...
		for (int32 i = FromX; i < ToX; i += BlockX)
		{
			for (int32 j = FromY; j < ToY; j += BlockY)
			{
				//FTransform NewTransform(Rot.Quaternion(), FVector((float)i, (float)j, 0.f));
				//*	
				FTransform NewTransform(Rot); //= GetTransform();
											  //NewTransform.SetRotation(Rot.Quaternion());
				NewTransform.SetLocation(FVector((float)i, (float)j, 0.f));
				//*/
				ISMeshCompWall->AddInstance(NewTransform);
			}
		}
	}
	//*/

	GLog->Log("Wall :: Completed...");

}

void ARoomGen::GenerateDoorways()
{
	/// To check for other rooms' walls intersection...
	FVector WallCenterBetween2Rooms = FVector::ZeroVector;
	int32 Crit = Rooms.Num();
	if (Crit <= 0)
	{
		return;
	}
	int32 MaxStep = FMath::Log2(RoomCount);// +2;

										   /// Check for all intersections...
	for (int32 i = 0; i < Crit; ++i)
	{
		/*
		GLog->Log("\n ---- \n");
		GLog->Log(FString::Printf(_T("DoorGen :: Rooms[%d]"), i+1));
		GLog->Log(WallStart.ToString());
		GLog->Log(WallEnd.ToString());
		//*/
		int32 botBound = i - MaxStep;
		FMath::Clamp(botBound, 0, Crit - 1);

		for (int32 j = i + MaxStep; j > i; --j)
		//for (int32 j = Crit - 1; j >= 0; --j)
		{
			/// To prevent OutOfArray errors... IDK why Clamp isn't working...
			if (j == i)
			{
				if (j == 0)
				{
					break;
				}
				continue;
			}
			if (j >= Crit)
			{
				continue;
			}
			else if (j < 0)
			{
				break;
			}
			//FMath::Clamp(j, 0, Crit-1);


			//*
			/// Find a wall between to rooms...
			WallCenterBetween2Rooms = Rooms[i].GetIntersectionCenterWith(Rooms[j]);//, FVector(WallX, WallY, 400.f));
			if (WallCenterBetween2Rooms != FVector::ZeroVector)
			{
				// @TODO :: Deal with shift... IDK what cause it...
				//WallCenterBetween2Rooms += FVector(560.f, 150.f, 500.f);
				WallCenterBetween2Rooms.Z = 500.f;
				GLog->Log("\n ---- \n DoorGen :: Intersection");
				GLog->Log(FString::Printf(_T("Room[%d] -- Rooms[%d]"), i, j));
				GLog->Log(WallCenterBetween2Rooms.ToString());

				//FTransform NT = GetTransform();
				FTransform NT;
				NT.SetLocation(WallCenterBetween2Rooms);
				//NT.AddToTranslation(WallCenterBetween2Rooms);
				ISMeshCompDoorway->AddInstance(NT);
				//MakeHole(WallCenterBetween2Rooms);
			}
			//*/
		}
	}
}

// UTIL
/// Check if Point satisfies user's parameters...
bool ARoomGen::SatisfyLimits(const FVector PointToCheck, const FRoom* InRoom, const bool IsHorizontal)
{
	FVector Origin = InRoom->Origin;
	FVector End = InRoom->End;

	/// ////////////////////////////////////////////////////////////////////////
	// @TODO :: Maybe, it's useful to have array of used dots to compare with...
	/// ////////////////////////////////////////////////////////////////////////

	/*
	int32 ExistRoomsNum = Rooms.Num();
	GLog->Log(FString::Printf(_T("Limit :: Rooms %d"), (int32)ExistRoomsNum));
	//*/
	/*
	GLog->Log(FString::Printf(_T("Limit :: Room: Origin(%d ; %d) End(%d ; %d)"), (int32)InRoom->Origin.X, (int32)InRoom->Origin.Y, (int32)InRoom->End.X, (int32)InRoom->End.Y));
	//GLog->Log(FString::Printf(_T("Limit :: Room: %d x %d"), (int32)InRoom->Length, (int32)InRoom->Width));
	GLog->Log(FString::Printf(_T("Limit :: Dot: (%d ; %d)\n\n"), (int32)PointToCheck.X, (int32)PointToCheck.Y));
	//*/

	if (IsHorizontal)
	{
		bool SatMinYBot = (PointToCheck.Y - (float)MinRoomY >= InRoom->Origin.Y) ? true : false;
		bool SatMinYTop = (PointToCheck.Y + (float)MinRoomY <= InRoom->End.Y) ? true : false;
		bool SatMaxYBot = (PointToCheck.Y - (float)MaxRoomY <= InRoom->Origin.Y) ? true : false;
		bool SatMaxYTop = (PointToCheck.Y + (float)MaxRoomY >= InRoom->End.Y) ? true : false;

		return (SatMinYBot && SatMinYTop && SatMaxYBot && SatMaxYTop);
	}
	else
	{
		/// Rght instead of Right is a sacrifice for beauty...
		bool SatMinXLeft = (PointToCheck.X - (float)MinRoomX >= InRoom->Origin.X) ? true : false;
		bool SatMinXRght = (PointToCheck.X + (float)MinRoomX <= InRoom->End.X) ? true : false;
		bool SatMaxXLeft = (PointToCheck.X - (float)MaxRoomX <= InRoom->Origin.X) ? true : false;
		bool SatMaxXRght = (PointToCheck.X + (float)MaxRoomX >= InRoom->End.X) ? true : false;

		return (SatMinXLeft && SatMinXRght && SatMaxXLeft &&  SatMaxXRght);
	}

}
FVector ARoomGen::GeneratePoint(const FVector From, const FVector To)
{
	FVector CurrentPoint;
	TArray< FVector > PossiblePoints;
	/*
	for (auto Row : GridPoints)
	{
		for (FVector Cell : row)
		{
			if (From.X <= Cell.X && From.Y <= cell.Y &&
				To.X <= cell.X && To.X <= cell.X)
			{
				//
			}
		}
	}
	//*/
	for (int32 row = GridPoints.Num()-1; row >= 0; --row)
	{
		for (int32 col = GridPoints[row].Num()-1; col >= 0; --col)
		{
			CurrentPoint = GridPoints[row][col];
			if (From.X <= CurrentPoint.X &&
				From.Y <= CurrentPoint.Y &&
				To.X >= CurrentPoint.X &&
				To.Y >= CurrentPoint.Y)
			{
				PossiblePoints.Add(CurrentPoint);
			}
		}
	}

	return PossiblePoints[FMath::RandRange(0, PossiblePoints.Num()-1)];
}