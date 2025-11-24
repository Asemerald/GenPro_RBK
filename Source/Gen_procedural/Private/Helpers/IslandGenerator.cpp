#include "Helpers/IslandGenerator.h"

#include "ProceduralMeshComponent.h"
#include "Math/UnrealMathUtility.h"

FIslandGenerator::FIslandGenerator(int32 InWidth, int32 InHeight, float InSpacing, float InNoiseScale, float InAmplitude, int32 InSeed)
    : Width(InWidth)
    , Height(InHeight)
    , Spacing(InSpacing)
    , NoiseScale(InNoiseScale)
    , Amplitude(InAmplitude)
    , Seed(InSeed)
{
}

// --- Génération du mesh ---
void FIslandGenerator::GenerateMesh(TArray<FVector>& OutVertices, TArray<int32>& OutTriangles, TArray<FVector>& OutNormals, TArray<FVector2D>& OutUVs, TArray<FColor>& OutColors, TArray<FProcMeshTangent>& OutTangents)
{
    OutVertices.Empty();
    OutTriangles.Empty();
    OutNormals.Empty();
    OutUVs.Empty();
    OutColors.Empty();
    OutTangents.Empty();

    // Génération des vertices
    for (int x = 0; x < Width; x++)
    {
        for (int y = 0; y < Height; y++)
        {
            float nx = (float)x / (float)Width - 0.5f;
            float ny = (float)y / (float)Height - 0.5f;
            float dist = FMath::Sqrt(nx * nx + ny * ny);

            float noiseValue = PerlinNoise2D(x * NoiseScale + Seed, y * NoiseScale + Seed);
            float z = noiseValue * Amplitude * (1.0f - dist);

            OutVertices.Add(FVector(x * Spacing, y * Spacing, z));
            OutUVs.Add(FVector2D((float)x / (Width - 1), (float)y / (Height - 1)));
            OutNormals.Add(FVector::UpVector);
            OutColors.Add(FColor::White);
            OutTangents.Add(FProcMeshTangent(1, 0, 0));
        }
    }

    // Génération des triangles
    for (int x = 0; x < Width - 1; x++)
    {
        for (int y = 0; y < Height - 1; y++)
        {
            int i = x * Height + y;

            OutTriangles.Add(i);
            OutTriangles.Add(i + Height);
            OutTriangles.Add(i + Height + 1);

            OutTriangles.Add(i);
            OutTriangles.Add(i + Height + 1);
            OutTriangles.Add(i + 1);
        }
    }
}

// --- Perlin 2D maison ---
float FIslandGenerator::Fade(float t) const
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float FIslandGenerator::Lerp(float a, float b, float t) const
{
    return a + t * (b - a);
}

float FIslandGenerator::Grad(int hash, float x, float y) const
{
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float FIslandGenerator::PerlinNoise2D(float x, float y) const
{
    int xi = FMath::FloorToInt(x) & 255;
    int yi = FMath::FloorToInt(y) & 255;

    float xf = x - FMath::FloorToFloat(x);
    float yf = y - FMath::FloorToFloat(y);

    float u = Fade(xf);
    float v = Fade(yf);

    // Table de permutation simple
    static const int p[512] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
        140,36,103,30,69,142,8,99,37,240,21,10,23,190, 6,148,
        247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
        57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,
        74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
        60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
        65,25,63,161,1,216,80,73,209,76,132,187,208, 89,18,169,
        200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
        52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
        207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
        119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
        129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
        218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
        81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
        184,84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,
        222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
        140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
        247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
        57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
        74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
        60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
        65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,
        200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
        52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
        207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
        119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
        129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
        218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
        81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
        184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,
        222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };

    int aa = p[(p[xi] + yi) & 255];
    int ab = p[(p[xi] + yi + 1) & 255];
    int ba = p[(p[xi + 1] + yi) & 255];
    int bb = p[(p[xi + 1] + yi + 1) & 255];

    float x1 = Lerp(Grad(aa, xf, yf), Grad(ba, xf - 1, yf), u);
    float x2 = Lerp(Grad(ab, xf, yf - 1), Grad(bb, xf - 1, yf - 1), u);

    return Lerp(x1, x2, v);
}
