// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditor.h"

#include "AssetRegistryModule.h"
#include "HAL/IConsoleManager.h"

#include "Engine/StaticMesh.h"
#include "Chaos/TriangleMeshImplicitObject.h"
#include "PhysicsEngine/BodySetup.h"

namespace LyraEditorUtilities
{

//////////////////////////////////////////////////////////////////////////

// returns true if the mesh has one or more degenerate triangles
bool CheckMeshDataForProblem(const Chaos::FTriangleMeshImplicitObject::ParticlesType& Particles, const Chaos::FTrimeshIndexBuffer& Elements)
{
	// Internal helper because the index buffer type is templated
	auto CheckTris = [&](const auto& Elements, int32 NumTriangles)
	{
		using VecType = Chaos::FTriangleMeshImplicitObject::ParticleVecType;

		for (int32 FaceIdx = 0; FaceIdx < NumTriangles; ++FaceIdx)
		{
			const VecType& A = Particles.X(Elements[FaceIdx][0]);
			const VecType& B = Particles.X(Elements[FaceIdx][1]);
			const VecType& C = Particles.X(Elements[FaceIdx][2]);

			const VecType AB = B - A;
			const VecType AC = C - A;
			VecType Normal = VecType::CrossProduct(AB, AC);

			if (Normal.SafeNormalize() < SMALL_NUMBER)
			{
				return true;
			}
		}

		return false;
	};

	const int32 NumTriangles = Elements.GetNumTriangles();
	if (Elements.RequiresLargeIndices())
	{
		return CheckTris(Elements.GetLargeIndexBuffer(), NumTriangles);
	}
	else
	{
		return CheckTris(Elements.GetSmallIndexBuffer(), NumTriangles);
	}
}

void CheckChaosMeshCollision(FOutputDevice& Ar)
{
	for (UStaticMesh* MeshAsset : TObjectRange<UStaticMesh>())
	{
		if (UBodySetup* BodySetup = MeshAsset->GetBodySetup())
		{
			for (const TSharedPtr<Chaos::FTriangleMeshImplicitObject, ESPMode::ThreadSafe>& TriMesh : BodySetup->ChaosTriMeshes)
			{
				if (Chaos::FTriangleMeshImplicitObject* TriMeshData = TriMesh.Get())
				{
					if (CheckMeshDataForProblem(TriMeshData->Particles(), TriMeshData->Elements()))
					{
						UE_LOG(LogConsoleResponse, Warning, TEXT("Mesh asset %s has one or more degenerate triangles in collision data"), *GetPathNameSafe(MeshAsset));
					}
				}
			}
		}
	}
}

FAutoConsoleCommandWithWorldArgsAndOutputDevice GCheckChaosMeshCollisionCmd(
	TEXT("Lyra.CheckChaosMeshCollision"),
	TEXT("Usage:\n")
	TEXT("  Lyra.CheckChaosMeshCollision\n")
	TEXT("\n")
	TEXT("It will check Chaos collision data for all *loaded* static mesh assets for any degenerate triangles"),
	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(
		[](const TArray<FString>& Params, UWorld* World, FOutputDevice& Ar)
{
	CheckChaosMeshCollision(Ar);
}));


//////////////////////////////////////////////////////////////////////////

}; // End of namespace
