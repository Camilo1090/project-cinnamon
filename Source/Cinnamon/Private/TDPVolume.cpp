// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPVolume.h"
#include "Engine/CollisionProfile.h"
#include "Components/BrushComponent.h"
#include "Components/LineBatchComponent.h"
#include "TDPNodeLink.h"
#include "libmorton/include/morton.h"
#include "DrawDebugHelpers.h"
#include <chrono>

ATDPVolume::ATDPVolume(const FObjectInitializer& ObjectInitializer)	: Super(ObjectInitializer)
{	
	GetBrushComponent()->Mobility = EComponentMobility::Static;
	BrushColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
	bColored = true;

	//Initialize();

	//UE_LOG(CinnamonLog, Log, TEXT("Size of Link: %d"), sizeof(TDPNodeLink));
}

void ATDPVolume::BeginPlay()
{

}

void ATDPVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
}

void ATDPVolume::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();
}

#if WITH_EDITOR

void ATDPVolume::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ATDPVolume::PostEditUndo()
{
	Super::PostEditUndo();
}

void ATDPVolume::OnPostShapeChanged()
{
	// no implementation
}

#endif

bool ATDPVolume::ShouldTickIfViewportsOnly() const
{
	return true;
}

void ATDPVolume::Initialize()
{
	UE_LOG(CinnamonLog, Log, TEXT("Initalizing..."));
	FBox bounds = GetComponentsBoundingBox(true);
	bounds.GetCenterAndExtents(mOrigin, mExtents);
	//DrawDebugBox(GetWorld(), mOrigin, mExtents, FQuat::Identity, DebugHelper::LayerColors[mLayers], true);

	mBlockedIndices.Empty();
	mTree.Layers.Empty();
	mTree.LeafNodes.Empty();
	mLayerVoxelHalfSizeCache.Reset();

	mTotalLayers = mLayers + 1;
	mLayerVoxelHalfSizeCache.Reserve(mTotalLayers);

	for (int32 i = 0; i < mTotalLayers; ++i)
	{
		mLayerVoxelHalfSizeCache.Add(GetVoxelSizeInLayer(i) / 2);
	}
}

void ATDPVolume::Generate()
{
	// initialize volume
	Initialize();

#if WITH_EDITOR

	auto startTime = std::chrono::high_resolution_clock::now();

#endif // WITH_EDITOR


	RasterizeLowRes();

	for (int32 i = 0; i < mTotalLayers; ++i)
	{
		mTree.Layers.Emplace();
	}

	for (int32 i = 0; i < mTotalLayers; ++i)
	{
		RasterizeLayer(i);
	}

	mTotalLayerNodes = mTree.GetTotalLayerNodes();
	mTotalLeafNodes = mTree.LeafNodes.Num();
	mTotalBytes = mTree.MemoryUsage();

#if WITH_EDITOR

	float totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1000.0f;
	UE_LOG(CinnamonLog, Log, TEXT("Total Time (s): %f"), totalTime);
	UE_LOG(CinnamonLog, Log, TEXT("Total Layer Nodes: %d"), mTotalLayerNodes);
	UE_LOG(CinnamonLog, Log, TEXT("Total Leaf Nodes: %d"), mTotalLeafNodes);
	UE_LOG(CinnamonLog, Log, TEXT("Memory Usage: %d Bytes"), mTotalBytes);

#endif
}

void ATDPVolume::RasterizeLowRes()
{
	mBlockedIndices.Emplace();

	const int32 lowresLayer = 1;
	int32 nodes = GetNodeAmountInLayer(1);
	for (int32 i = 0; i < nodes; ++i)
	{
		FVector position;
		GetNodePosition(lowresLayer, i, position);

		if (IsVoxelBlocked(position, mLayerVoxelHalfSizeCache[lowresLayer]))
		{
			mBlockedIndices[lowresLayer - 1].Add(i);
		}
	}

	for (int32 i = 0; i < mLayers; ++i)
	{
		mBlockedIndices.Emplace();
		for (MortonCodeType& code : mBlockedIndices[i])
		{
			mBlockedIndices[i + 1].Add(code >> 3); // your parent is eight times you
		}
	}
}

void ATDPVolume::RasterizeLayer(LayerIndexType layer)
{
	if (layer == 0)
	{
		// allocate space for all leaves
		// we know this value from lowres rasterize
		mTree.LeafNodes.Reserve(mBlockedIndices[layer].Num() * 8);
		// likewise for leaf layer nodes
		mTree.Layers[layer].Reserve(mBlockedIndices[layer].Num() * 8);

		int32 nodes = GetNodeAmountInLayer(layer);
		for (int32 i = 0; i < nodes; ++i)
		{
			// if parent is blocked then this node has to be added
			if (mBlockedIndices[layer].Contains(i >> 3))
			{
				// Add new node
				int32 nodeIndex = mTree.GetLayer(layer).Emplace();
				TDPNode& node = mTree.GetLayer(layer)[nodeIndex];

				node.SetMortonCode(i);
				FVector nodePosition;
				GetNodePosition(layer, i, nodePosition);

				NodeIndexType currentLeaf = mTree.LeafNodes.Emplace();
				if (IsVoxelBlocked(nodePosition, mLayerVoxelHalfSizeCache[layer], true))
				{
					FVector origin = nodePosition - FVector(mLayerVoxelHalfSizeCache[layer]);
					RasterizeLeafNode(origin, currentLeaf);
					auto& child = node.GetFirstChild();
					child.SetLayerIndex(0);
					child.SetNodeIndex(currentLeaf);
					child.SetSubnodeIndex(0);
				}

				// Debug
				if (ShowVoxels)
				{
					DrawDebugBox(GetWorld(), nodePosition, FVector(mLayerVoxelHalfSizeCache[layer]), FQuat::Identity, DebugHelper::LayerColors[layer], true);
				}

				if (ShowMortonCodes)
				{
					DrawDebugString(GetWorld(), nodePosition, FString::FromInt(layer) + ":" + FString::FromInt(nodeIndex), nullptr, DebugHelper::LayerColors[layer]);
				}
			}
		}
	}
	else if (mTree.GetLayer(layer - 1).Num() > 0)
	{
		mTree.Layers[layer].Reserve(mBlockedIndices[layer].Num() * 8);

		int32 nodes = GetNodeAmountInLayer(layer);
		for (int32 i = 0; i < nodes; ++i)
		{
			if (IsNodeBlocked(layer, i))
			{
				// Add new node
				int32 nodeIndex = mTree.GetLayer(layer).Emplace();
				TDPNode& node = mTree.GetLayer(layer)[nodeIndex];

				node.SetMortonCode(i);
				FVector nodePosition;
				GetNodePosition(layer, node.GetMortonCode(), nodePosition);

				// Debug
				if (ShowVoxels)
				{
					DrawDebugBox(GetWorld(), nodePosition, FVector(mLayerVoxelHalfSizeCache[layer]), FQuat::Identity, DebugHelper::LayerColors[layer], true);
				}

				if (ShowMortonCodes)
				{
					DrawDebugString(GetWorld(), nodePosition, FString::FromInt(layer) + ":" + FString::FromInt(nodeIndex), nullptr, DebugHelper::LayerColors[layer]);
				}
			}
		}
	}
}

void ATDPVolume::RasterizeLeafNode(const FVector& origin, NodeIndexType leaf)
{
	const int32 totalLeaves = 64;
	const float leafSize = mLayerVoxelHalfSizeCache[0] / 2; // 64 leaves per leaf node (cached value is already half)

	for (int32 i = 0; i < totalLeaves; ++i)
	{
		uint_fast32_t x, y, z;
		libmorton::morton3D_64_decode(i, x, y, z);
		FVector position = origin + FVector(x * leafSize, y * leafSize, z * leafSize) + FVector(leafSize / 2);

		if (IsVoxelBlocked(position, leafSize / 2))
		{
			mTree.LeafNodes[leaf].SetSubnode(i);

			// Debug
			if (ShowLeafVoxels)
			{
				DrawDebugBox(GetWorld(), position, FVector(leafSize / 2), FQuat::Identity, FColor::Emerald, true);
			}

			if (ShowMortonCodes)
			{
				DrawDebugString(GetWorld(), position, FString::FromInt(leaf) + ":" + FString::FromInt(i), nullptr, FColor::Emerald);
			}
		}
	}
}

bool ATDPVolume::IsNodeBlocked(LayerIndexType layer, MortonCodeType code) const
{
	return layer == mBlockedIndices.Num() || mBlockedIndices[layer].Contains(code >> 3);
}

bool ATDPVolume::IsVoxelBlocked(const FVector& position, const float halfSize, bool useTolerance) const
{
	FCollisionQueryParams collisionParams;
	collisionParams.bFindInitialOverlaps = true;
	collisionParams.bTraceComplex = false;

	// TODO: make tolerance member variable
	float tolerance = useTolerance ? 1.0f : 0.0f;

	//DrawDebugBox(GetWorld(), position, FVector(halfSize + tolerance), FQuat::Identity, FColor::Black, true);
	return GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, mCollisionChannel, FCollisionShape::MakeBox(FVector(halfSize + tolerance)), collisionParams);
}

void ATDPVolume::GetNodePosition(LayerIndexType layer, MortonCodeType code, FVector& position) const
{
	float size = mLayerVoxelHalfSizeCache[layer] * 2;
	uint_fast32_t x, y, z;
	libmorton::morton3D_64_decode(code, x, y, z);
	position = mOrigin - mExtents + (size * FVector(x, y, z)) + FVector(size / 2);
}

int32 ATDPVolume::GetNodeAmountInLayer(LayerIndexType layer) const
{
	return FMath::Pow(8, (mLayers - layer));
}

float ATDPVolume::GetVoxelSizeInLayer(LayerIndexType layer) const
{
	return mExtents.X / FMath::Pow(2, mLayers) * FMath::Pow(2, layer + 1);
}

