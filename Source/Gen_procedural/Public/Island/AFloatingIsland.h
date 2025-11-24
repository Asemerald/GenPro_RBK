#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "AFloatingIsland.generated.h"

UCLASS()
class GEN_PROCEDURAL_API AFloatingIsland : public AActor
{
	GENERATED_BODY()
    
public:    
	AFloatingIsland();

protected:
	virtual void BeginPlay() override;

public:    
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UProceduralMeshComponent* ProceduralMesh;

	// Paramètres de l’île
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Island")
	int32 Width = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Island")
	int32 Height = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Island")
	float Spacing = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise")
	float NoiseScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise")
	float Amplitude = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Island")
	UMaterialInterface* IslandMaterial;

	void GenerateIsland();

private:
	// Perlin 2D maison
	float PerlinNoise2D(float x, float y) const;
	float Fade(float t) const;
	float Lerp(float a, float b, float t) const;
	float Grad(int hash, float x, float y) const;

	void CalculateNormals(const TArray<FVector>& Vertices, const TArray<int32>& Triangles, TArray<FVector>& Normals) const;
};
