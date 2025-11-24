#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

/**
 * Classe helper pour générer un mesh d'île flottante procédural.
 * Ne dépend pas de ProceduralMeshComponent, peut être réutilisée dans n'importe quel Actor.
 */
class FIslandGenerator
{
public:
	FIslandGenerator(int32 InWidth = 50, int32 InHeight = 50, float InSpacing = 100.f, float InNoiseScale = 0.1f, float InAmplitude = 200.f, int32 InSeed = 1337);

	// Génère le mesh, rempli les tableaux passés en référence
	void GenerateMesh(TArray<FVector>& OutVertices, TArray<int32>& OutTriangles, TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs, TArray<FColor>& OutColors, TArray<FProcMeshTangent>& OutTangents);

private:
	int32 Width;
	int32 Height;
	float Spacing;
	float NoiseScale;
	float Amplitude;
	int32 Seed;

	// Fonctions pour Perlin 2D
	float PerlinNoise2D(float x, float y) const;
	float Fade(float t) const;
	float Lerp(float a, float b, float t) const;
	float Grad(int hash, float x, float y) const;
};
