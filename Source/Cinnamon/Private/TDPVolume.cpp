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
	mOctree.Layers.Empty();
	mOctree.LeafNodes.Empty();
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
		mOctree.Layers.Emplace();
	}

	// rasterize each layer, from the inner most layer (most precision) to the outer most (less precision), setting parent children links
	for (int32 i = 0; i < mTotalLayers; ++i)
	{
		RasterizeLayer(i);
	}

	for (int32 i = mTotalLayers - 2; i >= 0; --i)
	{
		SetNeighborLinks(i);
	}

	mTotalLayerNodes = mOctree.GetTotalLayerNodes();
	mTotalLeafNodes = mOctree.LeafNodes.Num();
	mTotalBytes = mOctree.MemoryUsage();

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
		mOctree.LeafNodes.Reserve(mBlockedIndices[layer].Num() * 8);
		// likewise for leaf layer nodes
		mOctree.Layers[layer].Reserve(mBlockedIndices[layer].Num() * 8);

		int32 nodes = GetNodeAmountInLayer(layer);
		for (int32 i = 0; i < nodes; ++i)
		{
			// if parent is blocked then this node has to be added
			if (mBlockedIndices[layer].Contains(i >> 3))
			{
				// Add new node
				int32 nodeIndex = mOctree.GetLayer(layer).Emplace();
				TDPNode& node = mOctree.GetLayer(layer)[nodeIndex];

				node.SetMortonCode(i);
				FVector nodePosition;
				GetNodePosition(layer, i, nodePosition);

				NodeIndexType currentLeaf = mOctree.LeafNodes.Emplace();
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
				if (DrawVoxels)
				{
					DrawNodeVoxel(nodePosition, FVector(mLayerVoxelHalfSizeCache[layer]), DebugHelper::LayerColors[layer]);
				}

				if (DrawOctreeMortonCodes)
				{
					DrawDebugString(GetWorld(), nodePosition, FString::FromInt(layer) + ":" + FString::FromInt(nodeIndex), nullptr, DebugHelper::LayerColors[layer]);
				}
			}
		}
	}
	else if (mOctree.GetLayer(layer - 1).Num() > 0)
	{
		mOctree.Layers[layer].Reserve(mBlockedIndices[layer].Num() * 8);

		int32 nodes = GetNodeAmountInLayer(layer);
		for (int32 i = 0; i < nodes; ++i)
		{
			if (IsNodeBlocked(layer, i))
			{
				// Add new node
				int32 nodeIndex = mOctree.GetLayer(layer).Emplace();
				TDPNode& node = mOctree.GetLayer(layer)[nodeIndex];

				node.SetMortonCode(i);
				FVector nodePosition;
				GetNodePosition(layer, node.GetMortonCode(), nodePosition);

				NodeIndexType firstChildIndex;
				if (GetNodeIndexInLayer(layer - 1, node.GetMortonCode() << 3, firstChildIndex))
				{
					// set parent -> first child link
					auto& firstChild = node.GetFirstChild();
					firstChild.SetLayerIndex(layer - 1);
					firstChild.SetNodeIndex(firstChildIndex);

					// set children -> parent links (exactly 8 children)
					auto& octreeLayer = mOctree.GetLayer(layer - 1);
					for (int32 childOffset = 0; childOffset < 8; ++childOffset)
					{
						auto& parent = octreeLayer[firstChildIndex + childOffset].GetParent();
						parent.SetLayerIndex(layer);
						parent.SetNodeIndex(nodeIndex);
					}

					// Debug
					if (DrawParentChildLinks)
					{
						FVector startPosition, endPosition;
						GetNodePosition(layer, node.GetMortonCode(), startPosition);
						GetNodePosition(layer - 1, node.GetMortonCode() << 3, endPosition);
						DrawDebugDirectionalArrow(GetWorld(), startPosition, endPosition, LinkSize, DebugHelper::LayerColors[layer], true);
					}
				}
				else
				{
					// if no children then invalidate link
					auto& firstChild = node.GetFirstChild();
					firstChild.Invalidate();
				}

				// Debug
				if (DrawVoxels)
				{
					DrawNodeVoxel(nodePosition, FVector(mLayerVoxelHalfSizeCache[layer]), DebugHelper::LayerColors[layer]);
				}

				if (DrawOctreeMortonCodes)
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
			mOctree.LeafNodes[leaf].SetSubnode(i);

			// Debug
			if (DrawLeafVoxels)
			{
				DrawNodeVoxel(position, FVector(leafSize / 2), FColor::Emerald);
			}

			if (DrawLeafMortonCodes)
			{
				DrawDebugString(GetWorld(), position, FString::FromInt(leaf) + ":" + FString::FromInt(i), nullptr, FColor::Emerald);
			}
		}
	}
}

void ATDPVolume::SetNeighborLinks(const LayerIndexType layer)
{
	auto& octreeLayer = mOctree.GetLayer(layer);

	for (int32 i = 0; i < octreeLayer.Num(); ++i)
	{
		auto& node = octreeLayer[i];
		FVector nodePosition;
		GetNodePosition(layer, node.GetMortonCode(), nodePosition);

		// at most 6 neighbors, each face
		for (int32 j = 0; j < 6; ++j)
		{
			auto& link = node.GetNeighbors()[j];

			NodeIndexType currentNodeIndex = i;
			auto currentLayer = layer;
			// find the closest neighbor, start by looking in the current node's layer
			// go up the layers if no valid neighbor was found in that layer
			while (!FindNeighborLink(currentLayer, currentNodeIndex, j, link, nodePosition))
			{
				auto& parentLink = mOctree.GetLayer(currentLayer)[currentNodeIndex].GetParent();
				if (parentLink.IsValid())
				{
					currentNodeIndex = parentLink.NodeIndex;
					currentLayer = parentLink.LayerIndex;
				}
				else
				{
					++currentLayer;
					GetNodeIndexInLayer(currentLayer, node.GetMortonCode() >> 3, currentNodeIndex);
				}
			}
		}
	}
}

bool ATDPVolume::FindNeighborLink(const LayerIndexType layerIndex, const NodeIndexType nodeIndex, uint8 direction, TDPNodeLink& link, const FVector& nodePosition)
{
	auto& layer = mOctree.GetLayer(layerIndex);
	auto& node = layer[nodeIndex];
	int32 maxCoordinate = static_cast<int32>(FMath::Pow(2, (mLayers - layerIndex)));

	uint_fast32_t x, y, z;
	libmorton::morton3D_64_decode(node.GetMortonCode(), x, y, z);

	FIntVector signedPosition(static_cast<int32>(x), static_cast<int32>(y), static_cast<int32>(z));
	signedPosition += NodeHelper::NeighborDirections[direction];

	// invalidate link if neighbor coordinates are out of bounds
	if (signedPosition.X < 0 || signedPosition.X >= maxCoordinate ||
		signedPosition.Y < 0 || signedPosition.Y >= maxCoordinate ||
		signedPosition.Z < 0 || signedPosition.Z >= maxCoordinate)
	{
		link.Invalidate();

		// Debug
		if (DrawInvalidNeighborLinks)
		{
			FVector startPosition, endPosition;
			GetNodePosition(layerIndex, node.GetMortonCode(), startPosition);
			endPosition = startPosition + (FVector(NodeHelper::NeighborDirections[direction]) * mLayerVoxelHalfSizeCache[layerIndex] * 2);
			DrawDebugDirectionalArrow(GetWorld(), startPosition, endPosition, LinkSize, FColor::Red, true);
		}

		return true;
	}

	x = signedPosition.X;
	y = signedPosition.Y;
	z = signedPosition.Z;

	MortonCodeType neighborCode = libmorton::morton3D_64_encode(x, y, z);

	// assume we'll look for nodes sequentially after current
	int32 stopValue = layer.Num();
	int32 nodeDelta = 1;
	// if we'll be looking for a lower node then change values
	if (neighborCode < node.GetMortonCode())
	{
		nodeDelta = -1;
		stopValue = -1;
	}

	for (int32 i = nodeIndex + nodeDelta; i != stopValue; i += nodeDelta)
	{
		auto& currentNode = layer[i];

		// if node found
		if (currentNode.GetMortonCode() == neighborCode)
		{
			if (layerIndex == 0 && 
				currentNode.HasChildren() && 
				mOctree.LeafNodes[currentNode.GetFirstChild().NodeIndex].IsFullyBlocked())
			{
				link.Invalidate();
				return true;
			}

			link.SetLayerIndex(layerIndex);
			check(i < layer.Num() && i >= 0);
			link.SetNodeIndex(i);

			// Debug
			if (DrawValidNeighborLinks)
			{
				FVector endPosition;
				GetNodePosition(layerIndex, neighborCode, endPosition);
				DrawDebugDirectionalArrow(GetWorld(), nodePosition, endPosition, LinkSize, DebugHelper::LayerColors[layerIndex], true);
			}

			return true;
		}
		else if ((nodeDelta == -1 && currentNode.GetMortonCode() < neighborCode) ||
			(nodeDelta == 1 && currentNode.GetMortonCode() > neighborCode))
		{
			return false;
		}
	}

	return false;
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

bool ATDPVolume::GetNodeIndexInLayer(const LayerIndexType layer, const MortonCodeType nodeCode, NodeIndexType& index) const
{
	const auto& octreeLayer = mOctree.GetLayer(layer);

	if (nodeCode < octreeLayer[octreeLayer.Num() - 1].GetMortonCode())
	{
		for (int32 i = 0; i < octreeLayer.Num(); ++i)
		{
			if (octreeLayer[i].GetMortonCode() == nodeCode)
			{
				index = i;
				return true;
			}
		}
	}

	return false;
}

void ATDPVolume::DrawNodeVoxel(const FVector& position, const FVector& extent, const FColor& color) const
{
	DrawDebugBox(GetWorld(), position, extent, FQuat::Identity, color, true, -1.0f, 0, VoxelThickness);
}

