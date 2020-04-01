// Fill out your copyright notice in the Description page of Project Settings.


#include "OceanMesh.h"

// Sets default values
AOceanMesh::AOceanMesh()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CustomMesh = CreateDefaultSubobject<UProceduralMeshComponent>("CustomMesh");
	SetRootComponent(CustomMesh);
	CustomMesh->bUseAsyncCooking = true;

	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/WaterMaterial.WaterMaterial'"));
	WaterMaterial = Material.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial> WireMaterial(TEXT("Material'/Game/Wireframe.Wireframe'"));
	WireFrameMaterial = WireMaterial.Object;


	CurrentWaveType = EWaveType::SinWaves;
	NumWaves = 1;
	SinWaves.Init(FSinWave(), NumWaves);
}

// Called when the game starts or when spawned
void AOceanMesh::BeginPlay()
{
	Super::BeginPlay();

	ReDraw(100,100,100.f,100.f);
	createSection(CustomMesh);
}

// Called every frame
void AOceanMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Time += DeltaTime;


	
	ReDraw(100, 100, 100.f, 100.f);
	deriveNormals();
	updateSection(CustomMesh);

}

void AOceanMesh::Reset() {
	positions.Reset();
	normals.Reset();
	colors.Reset();
	uvs.Reset();
	indices.Reset();
	tangents.Reset();
}

void AOceanMesh::ReDraw(int Nx, int Ny, float wx, float wy) {
	Reset();
	
	for (int j = 0; j < Ny; ++j) {
		for (int i = 0; i < Nx; ++i) {
			float v = float(j) / (Ny - 1);
			float u = float(i) / (Nx - 1);
			FVector Point;

			if (CurrentWaveType == EWaveType::GerstnerWaves) {
				Point = GerstnerWave(i, j);
			}
			else {
				Point = SinWave(i, j);
			}

			positions.Emplace(Point.X, Point.Y, Point.Z);
			uvs.Emplace(u, v);
			if (i < (Nx - 1) && j < (Ny - 1)) {
				auto ij = Nx * j + i;
				auto Ij = Nx * j + i + 1;
				auto iJ = Nx * (j + 1) + i;
				auto IJ = Nx * (j + 1) + i + 1;
				indices.Push(ij); indices.Push(iJ); indices.Push(Ij);
				indices.Push(Ij); indices.Push(iJ); indices.Push(IJ);
			}
		}
	}
}

//Calculates the coordinates of a vertex based on the Gerstner Wave algorthm given its x and y position at a certain timestamp Time
FVector AOceanMesh::GerstnerWave(int x, int y)
{
	FVector Result = FVector(x,y,0);

	for (int32 Index = 0; Index != SinWaves.Num(); ++Index) {
		//Setup values to make the equations below more readable
		float Amplitude = (SinWaves[Index]).Amplitude;
		float Frequency = 2/(SinWaves[Index]).WaveLength;
		float Sharpness = (SinWaves[Index]).Sharpness;
		float Speed = 2 * (SinWaves[Index]).Speed  / (SinWaves[Index]).WaveLength;
		FVector2D Direction = (SinWaves[Index]).Direction.GetSafeNormal(); //normalize the vector, necessary because of the user interface
		float DotProduct = (SinWaves[Index]).Direction.X * x + (SinWaves[Index]).Direction.Y * y;


		Result.X += Sharpness * Amplitude * Direction.X *  cosf( Frequency * DotProduct + Time * Speed);
		Result.Y += Sharpness * Amplitude * Direction.Y * cosf(Frequency * DotProduct + Time * Speed);
		Result.Z += Amplitude * sinf(DotProduct * Frequency + Time * Speed);
	}

	return Result;
}

//Calculates the coordinates of a vertex based on the Sumation of Waves algorthm given its x and y position at a certain timestamp Time
FVector AOceanMesh::SinWave(float x, float y) {
	FVector Result = FVector(x, y, 0);
	float ZValue = 0.f;

	for (int32 Index = 0; Index != SinWaves.Num(); ++Index) {
		FVector2D Direction = (SinWaves[Index]).Direction.GetSafeNormal();
		float Amplitude = (SinWaves[Index]).Amplitude;
		float Frequency = 2 / (SinWaves[Index]).WaveLength;
		float Speed = 2 * (SinWaves[Index]).Speed / (SinWaves[Index]).WaveLength;

		ZValue += Amplitude * sinf((Direction.X * x + Direction.Y * y) * Frequency + Time * Speed);
	}
	Result.Z = ZValue;

	return  Result;
}


// Derive normals based on triangle faces
void AOceanMesh::deriveNormals(bool normalize, bool equalWeightPerFace) {
	if (positions.Num() < 3) return;

	// Same number of normals as positions
	normals.Init(FVector(0, 0, 0), positions.Num());

	// Get vector perpendicular to triangle formed by three points
	auto getPerp = [&](const FVector& p1, const FVector& p2, const FVector& p3) {
		auto perp = (p3 - p1) ^ (p2 - p1);				// MWAAT (mean weighted by areas of adjacent triangles)
		if (equalWeightPerFace) perp.Normalize();	// MWE (mean weighted equally)
		return perp;
	};

	if (indices.Num() >= 3) { // smooth normals
		for (int j = 0; j < indices.Num() - 2; j += 3) { // iterate over triangles
			auto i1 = indices[j], i2 = indices[j + 1], i3 = indices[j + 2];
			auto perp = getPerp(positions[i1], positions[i2], positions[i3]);
			normals[i1] += perp;
			normals[i2] += perp;
			normals[i3] += perp;
		}
	}
	else { // flat normals
		for (int i = 0; i < positions.Num() - 2; i += 3) {
			auto i1 = i, i2 = i + 1, i3 = i + 2;
			auto perp = getPerp(positions[i1], positions[i2], positions[i3]);
			normals[i1] = perp;
			normals[i2] = perp;
			normals[i3] = perp;
		}
	}

	if (normalize) {
		for (auto& v : normals) v.Normalize();
	}
}


void AOceanMesh::SetAmplitude(int wave, float value) {
	SinWaves[wave].Amplitude = value;
}

void AOceanMesh::SetWaveLength(int wave, float value) {
	SinWaves[wave].WaveLength = value;
}

void AOceanMesh::SetSharpness(int wave, float value) {
	SinWaves[wave].Sharpness = value;
}

void AOceanMesh::SetSpeed(int wave, float value) {
	SinWaves[wave].Speed = value;
}

void AOceanMesh::SetDirectionX(int wave, float value) {
	SinWaves[wave].Direction.X = value;
}

void AOceanMesh::SetDirectionY(int wave, float value) {
	SinWaves[wave].Direction.Y = value;
}

void AOceanMesh::RemoveWave(int wave) {
	SinWaves.RemoveAt(wave);
}

void AOceanMesh::SetWaterMaterial() {
	CustomMesh->SetMaterial(0, WaterMaterial);
}

void AOceanMesh::SetBlankMaterial() {
	CustomMesh->SetMaterial(0, nullptr);
}

void AOceanMesh::SetWireFrameMaterial() {
	CustomMesh->SetMaterial(0, WireFrameMaterial);
}