#include "Island/AFloatingIsland.h"
#include "Math/UnrealMathUtility.h"

AFloatingIsland::AFloatingIsland()
{
    PrimaryActorTick.bCanEverTick = false;

    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
    RootComponent = ProceduralMesh;
}

void AFloatingIsland::BeginPlay()
{
    Super::BeginPlay();
    GenerateIsland();
}

void AFloatingIsland::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AFloatingIsland::GenerateIsland()
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;

    // ----- PARAMETRES -----
    const int32 Rings = 40;      // nombre d’anneaux (résolution radiale)
    const int32 PointsPerRing = 60; // résolution angulaire
    const float Radius = 3000.f; // taille de l’île
    const float BottomDepth = 2000.f; // pointe en dessous

    // ----- VERTEX : CENTRE -----
    Vertices.Add(FVector(0, 0, 0));
    int32 CenterIndex = 0;

    // ----- GENERATION DES ANNEAUX -----
    for (int r = 1; r <= Rings; r++)
    {
        float t = r / (float)Rings;
        float currentRadius = t * Radius;

        for (int p = 0; p < PointsPerRing; p++)
        {
            float angle = 2.f * PI * (p / (float)PointsPerRing);

            float x = FMath::Cos(angle) * currentRadius;
            float y = FMath::Sin(angle) * currentRadius;

            // Falloff circulaire
            float falloff = 1.f - FMath::Pow(t, 3.f);

            // Perlin
            float noise = PerlinNoise2D((x + Seed) * NoiseScale, (y + Seed) * NoiseScale);

            float z = noise * Amplitude * falloff;

            Vertices.Add(FVector(x, y, z));
        }
    }

    int32 TopVertexCount = Vertices.Num();

    // ----- DESSOUS : CÔNE -----
    for (int r = 1; r <= Rings; r++)
    {
        float t = r / (float)Rings;
        float currentRadius = t * Radius;

        for (int p = 0; p < PointsPerRing; p++)
        {
            float angle = 2.f * PI * (p / (float)PointsPerRing);

            float x = FMath::Cos(angle) * currentRadius;
            float y = FMath::Sin(angle) * currentRadius;

            float z = -BottomDepth * (1.f - t); // pente vers la pointe

            Vertices.Add(FVector(x, y, z));
        }
    }

    // Index du centre du cône
    int32 BottomCenter = Vertices.Add(FVector(0, 0, -BottomDepth));

    // ----- TRIANGLES DU DESSUS -----

    for (int p = 0; p < PointsPerRing; p++)
    {
        int next = (p + 1) % PointsPerRing;
        Triangles.Add(CenterIndex);
        Triangles.Add(1 + next);
        Triangles.Add(1 + p);
    }

    // ----- TRIANGLES ENTRE ANNEAUX (côtés) -----

    for (int r = 0; r < Rings - 1; r++)
    {
        int ringStart = 1 + r * PointsPerRing;
        int nextRingStart = ringStart + PointsPerRing;

        for (int p = 0; p < PointsPerRing; p++)
        {
            int next = (p + 1) % PointsPerRing;

            int A = ringStart + p;
            int B = ringStart + next;
            int C = nextRingStart + p;
            int D = nextRingStart + next;

            Triangles.Add(A);
            Triangles.Add(D);
            Triangles.Add(C);

            Triangles.Add(A);
            Triangles.Add(B);
            Triangles.Add(D);
        }
    }

    // ----- TRIANGLES DU DESSOUS -----

    int32 BottomStart = TopVertexCount;

    for (int r = 0; r < Rings - 1; r++)
    {
        int ringStart = BottomStart + r * PointsPerRing;
        int nextRingStart = ringStart + PointsPerRing;

        for (int p = 0; p < PointsPerRing; p++)
        {
            int next = (p + 1) % PointsPerRing;

            int A = ringStart + p;
            int B = ringStart + next;
            int C = nextRingStart + p;
            int D = nextRingStart + next;

            Triangles.Add(A);
            Triangles.Add(C);
            Triangles.Add(D);

            Triangles.Add(A);
            Triangles.Add(D);
            Triangles.Add(B);
        }
    }

    // ----- BASE DU CÔNE -----
    int32 lastRingStart = BottomStart + (Rings - 1) * PointsPerRing;
    for (int p = 0; p < PointsPerRing; p++)
    {
        int next = (p + 1) % PointsPerRing;
        Triangles.Add(lastRingStart + p);
        Triangles.Add(BottomCenter);
        Triangles.Add(lastRingStart + next);
    }

    // ----- MESH -----
    TArray<FVector> Normals;
    TArray<FVector2D> UV;
    TArray<FLinearColor> Colors;
    TArray<FProcMeshTangent> Tangents;

    ProceduralMesh->CreateMeshSection_LinearColor(
        0, Vertices, Triangles, Normals, UV, Colors, Tangents, true
    );

    if (IslandMaterial)
        ProceduralMesh->SetMaterial(0, IslandMaterial);
}



// --- Normales par triangle ---
void AFloatingIsland::CalculateNormals(const TArray<FVector>& Vertices, const TArray<int32>& Triangles, TArray<FVector>& Normals) const
{
    Normals.SetNum(Vertices.Num());
    for (int i = 0; i < Triangles.Num(); i += 3)
    {
        FVector A = Vertices[Triangles[i]];
        FVector B = Vertices[Triangles[i+1]];
        FVector C = Vertices[Triangles[i+2]];
        FVector Normal = FVector::CrossProduct(B - A, C - A).GetSafeNormal();

        Normals[Triangles[i]] = Normal;
        Normals[Triangles[i+1]] = Normal;
        Normals[Triangles[i+2]] = Normal;
    }
}

// --- Perlin 2D maison ---
float AFloatingIsland::Fade(float t) const
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float AFloatingIsland::Lerp(float a, float b, float t) const
{
    return a + t * (b - a);
}

float AFloatingIsland::Grad(int hash, float x, float y) const
{
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float AFloatingIsland::PerlinNoise2D(float x, float y) const
{
    int xi = FMath::FloorToInt(x) & 255;
    int yi = FMath::FloorToInt(y) & 255;

    float xf = x - FMath::FloorToFloat(x);
    float yf = y - FMath::FloorToFloat(y);

    float u = Fade(xf);
    float v = Fade(yf);

    static const int p[512] = {
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
