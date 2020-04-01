// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "OceanMesh.generated.h"

//Enum to choose between algorithms
UENUM(BlueprintType)
enum class EWaveType : uint8 {
	SinWaves UMETA(DisplayName="Sin Waves"),
	GerstnerWaves UMETA(DisplayName="Gerstner Waves")
};


//Struct of Wave parameters
USTRUCT(BlueprintType)
struct FSinWave {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveLength;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amplitude;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Sharpness;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Direction;
	

	FSinWave(){
		WaveLength = 1.f;
		Amplitude = 1.f;
		Speed = 0.f;
		Direction = FVector2D(0.f, 0.f);
		Sharpness = 1.f;
	}
};


UCLASS(BlueprintType)
class WAVEGENERATION_API AOceanMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Default Constructor
	AOceanMesh();

	//Set of SinWaves Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSinWave> SinWaves;

	//Number of Waves being summed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NumWaves;

	//Current algorithm being used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWaveType CurrentWaveType;
	
	//Procedural Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UProceduralMeshComponent* CustomMesh;


	//AUXILIAR FUNCTIONS FOR UI
	UFUNCTION(BlueprintCallable, Category="OceanSet")
	void SetAmplitude(int wave, float value);

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void SetWaveLength(int wave, float value);

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void SetSpeed(int wave, float value);

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void SetSharpness(int wave, float value);

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void SetDirectionX(int wave, float value);

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void SetDirectionY(int wave, float value);

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void RemoveWave(int wave);

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void SetWaterMaterial();

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void SetBlankMaterial();

	UFUNCTION(BlueprintCallable, Category = "OceanSet")
	void SetWireFrameMaterial();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Responsible to recalculate the mesh
	void ReDraw(int Nx, int Ny, float wx = 1., float wy = 1.);

	//Resets the mesh values
	void Reset();

	//Calculates normals
	void deriveNormals(bool normalize = true, bool equalWeightPerFace = false);

	//Calculates point using Gerstner Wave algorithm
	FVector GerstnerWave(int x, int y);

	//Calculates point using Summation of sine waves algorithm
	FVector SinWave(float x, float y);

private:
	//Procedural Mesh Parameters
	TArray<FVector> positions;
	TArray<FVector> normals;
	TArray<FColor> colors;
	TArray<FVector2D> uvs;
	TArray<int32> indices;
	TArray<FProcMeshTangent> tangents;

	//Auxiliar time float to use in the different algorithms. Incremented at everytick.
	float Time;

	//Materials
	class UMaterial* WaterMaterial;
	class UMaterial* WireFrameMaterial;

	// Create a ProceduralMeshComponent section using current vertex data
	void createSection(UProceduralMeshComponent * pmc, int32 section=0) const {
		pmc->CreateMeshSection(
			section,
			positions, indices, normals, uvs, colors, tangents,
			true // whether to generate collision data
		);
	}

	// Update a ProceduralMeshComponent section using current vertex data
	void updateSection(UProceduralMeshComponent * pmc, int32 section=0) const {
		pmc->UpdateMeshSection(
			section,
			positions, normals, uvs, colors, tangents
		);
	}

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
