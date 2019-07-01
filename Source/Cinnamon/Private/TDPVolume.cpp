// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPVolume.h"
#include "Engine/CollisionProfile.h"
#include "Components/BrushComponent.h"
#include "Components/LineBatchComponent.h"
#include "TDPNodeLink.h"
#include "TDPNode.h"
#include "libmorton/include/morton.h"
#include "DrawDebugHelpers.h"
#include <chrono>

ATDPVolume::ATDPVolume(const FObjectInitializer& ObjectInitializer)	: Super(ObjectInitializer)
{	
	GetBrushComponent()->Mobility = EComponentMobility::Static;
	BrushColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
	bColored = true;
}

void ATDPVolume::BeginPlay()
{
	Super::BeginPlay();
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
#if WITH_EDITOR
	UE_LOG(CinnamonLog, Log, TEXT("Initalizing..."));
#endif

	FBox bounds = GetComponentsBoundingBox(true);
	bounds.GetCenterAndExtents(mOrigin, mExtents);
	//DrawDebugBox(GetWorld(), mOrigin, mExtents, FQuat::Identity, DebugHelper::LayerColors[mLayers], true);

	mBlockedIndices.Reset();
	mOctree.Clear();
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

	GetWorld()->PersistentLineBatcher->SetComponentTickEnabled(false);
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

void ATDPVolume::Clear()
{
	mOctree.Clear();
	mBlockedIndices.Reset();
	mTotalLayers = 0;
	mTotalBytes = 0;
	mTotalLayerNodes = 0;
	mTotalLeafNodes = 0;
	FlushDebugStrings(GetWorld());
	FlushPersistentDebugLines(GetWorld());
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
			mBlockedIndices[0].Add(i);
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
#if WITH_EDITOR
					// Debug
					if (DrawOnlyBlockedLeafVoxels)
					{
						DrawNodeVoxel(nodePosition, FVector(mLayerVoxelHalfSizeCache[layer]), DebugHelper::LayerColors[layer]);
					}

					if (DrawOctreeMortonCodes)
					{
						DrawDebugString(GetWorld(), nodePosition, FString::FromInt(layer) + ":" + FString::FromInt(nodeIndex), nullptr, DebugHelper::LayerColors[layer]);
					}
#endif
				}

#if WITH_EDITOR
				// Debug
				if (DrawLeafVoxels && !DrawOnlyBlockedLeafVoxels)
				{
					DrawNodeVoxel(nodePosition, FVector(mLayerVoxelHalfSizeCache[layer]), DebugHelper::LayerColors[layer]);
				}

				if (DrawOctreeMortonCodes)
				{
					DrawDebugString(GetWorld(), nodePosition, FString::FromInt(layer) + ":" + FString::FromInt(nodeIndex), nullptr, DebugHelper::LayerColors[layer]);
				}
#endif
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

#if WITH_EDITOR
					// Debug
					if (DrawParentChildLinks)
					{
						FVector startPosition, endPosition;
						GetNodePosition(layer, node.GetMortonCode(), startPosition);
						GetNodePosition(layer - 1, node.GetMortonCode() << 3, endPosition);
						DrawDebugDirectionalArrow(GetWorld(), startPosition, endPosition, LinkSize, DebugHelper::LayerColors[layer], true);
					}
#endif
				}
				else
				{
					// if no children then invalidate link
					auto& firstChild = node.GetFirstChild();
					firstChild.Invalidate();
				}

#if WITH_EDITOR
				// Debug
				if (DrawVoxels)
				{
					DrawNodeVoxel(nodePosition, FVector(mLayerVoxelHalfSizeCache[layer]), DebugHelper::LayerColors[layer]);
				}

				if (DrawOctreeMortonCodes)
				{
					DrawDebugString(GetWorld(), nodePosition, FString::FromInt(layer) + ":" + FString::FromInt(nodeIndex), nullptr, DebugHelper::LayerColors[layer]);
				}
#endif
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

		if (IsVoxelBlocked(position, leafSize / 2, true))
		{
			mOctree.LeafNodes[leaf].SetSubnode(i);

#if WITH_EDITOR
			// Debug
			if (DrawMiniLeafVoxels)
			{
				DrawNodeVoxel(position, FVector(leafSize / 2), FColor::Emerald);
			}

			if (DrawLeafMortonCodes)
			{
				DrawDebugString(GetWorld(), position, FString::FromInt(leaf) + ":" + FString::FromInt(i), nullptr, FColor::Emerald);
			}
#endif
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
			while (!FindNeighborLink(currentLayer, currentNodeIndex, j, link, nodePosition)
				&& currentLayer < mOctree.Layers.Num() - 2)
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

#if WITH_EDITOR
		// Debug
		if (DrawInvalidNeighborLinks)
		{
			FVector startPosition, endPosition;
			GetNodePosition(layerIndex, node.GetMortonCode(), startPosition);
			endPosition = startPosition + (FVector(NodeHelper::NeighborDirections[direction]) * mLayerVoxelHalfSizeCache[layerIndex] * 2);
			DrawDebugDirectionalArrow(GetWorld(), startPosition, endPosition, LinkSize, FColor::Red, true);
		}
#endif

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

#if WITH_EDITOR
			// Debug
			if (DrawValidNeighborLinks)
			{
				FVector endPosition;
				GetNodePosition(layerIndex, neighborCode, endPosition);
				DrawDebugDirectionalArrow(GetWorld(), nodePosition, endPosition, LinkSize, DebugHelper::LayerColors[layerIndex], true);
			}
#endif

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

bool ATDPVolume::IsVoxelBlocked(const FVector& position, const float halfSize, bool useClearance) const
{
	FCollisionQueryParams collisionParams;
	collisionParams.bFindInitialOverlaps = true;
	collisionParams.bTraceComplex = mComplexCollision;

	float clearance = useClearance ? mCollisionClearance : 0.0f;
	bool result = GetWorld()->OverlapBlockingTestByChannel(position, FQuat::Identity, mCollisionChannel, FCollisionShape::MakeBox(FVector(halfSize + clearance)), collisionParams);

#if WITH_EDITOR
	if (DrawCollisionVoxels && result)
	{
		DrawDebugBox(GetWorld(), position, FVector(halfSize + clearance), FQuat::Identity, FColor::Black, true);
	}
#endif

	return result;
}

const TDPTree& ATDPVolume::GetOctree() const
{
	return mOctree;
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

bool ATDPVolume::GetNodePositionFromLink(TDPNodeLink link, FVector& position) const
{
	const auto& node = mOctree.GetLayer(link.LayerIndex)[link.NodeIndex];
	GetNodePosition(link.LayerIndex, node.GetMortonCode(), position);

	// if layer 0 and valid children, check 64 bit leaf for any set bits
	if (link.LayerIndex == 0 && node.GetFirstChild().IsValid())
	{
		/*FVector origin = position - FVector(mLayerVoxelHalfSizeCache[0]);
		float leafSize = mLayerVoxelHalfSizeCache[0] / 2;
		uint_fast32_t x, y, z;
		libmorton::morton3D_64_decode(link.SubnodeIndex, x, y, z);
		position = origin + FVector(x * leafSize, y * leafSize, z * leafSize) + FVector(leafSize / 2);*/
		float voxelSize = mLayerVoxelHalfSizeCache[0] * 2;
		uint_fast32_t x, y, z;
		libmorton::morton3D_64_decode(link.SubnodeIndex, x, y, z);
		position += FVector(x * voxelSize / 4, y * voxelSize / 4, z * voxelSize / 4) - FVector(voxelSize * 0.375f);

		const auto& leafNode = mOctree.LeafNodes[node.GetFirstChild().NodeIndex];

		return !leafNode.GetSubnode(link.SubnodeIndex);
	}

	return true;
}

bool ATDPVolume::GetLinkFromPosition(const FVector& position, TDPNodeLink& link) const
{
	if (!IsPointInside(position))
	{
#if WITH_EDITOR
		UE_LOG(CinnamonLog, Warning, TEXT("GetLinkFromPosition: position outside the navigation volume."));
#endif
		return false;
	}

	FVector mortonOrigin = mOrigin - mExtents;
	FVector localPosition = position - mortonOrigin;

	LayerIndexType currentLayer = mTotalLayers - 1;
	NodeIndexType currentNode = 0;

	while (currentLayer >= 0)
	{
		const auto& layer = mOctree.GetLayer(currentLayer);

		FIntVector voxel;
		GetVoxelMortonPosition(position, currentLayer, voxel);

		MortonCodeType code = libmorton::morton3D_64_encode(static_cast<uint_fast32_t>(voxel.X), static_cast<uint_fast32_t>(voxel.Y), static_cast<uint_fast32_t>(voxel.Z));

		for (NodeIndexType i = currentNode; i < layer.Num(); ++i)
		{
			const auto& node = layer[i];
			if (node.GetMortonCode() == code)
			{
				// if node found and it has no children then this is our most precise node for the given position
				if (!node.GetFirstChild().IsValid())
				{
					link.SetLayerIndex(currentLayer);
					link.SetNodeIndex(i);
					link.SetSubnodeIndex(0);

					return true;
				}

				// if we are in layer 0 then we have to find the subnode inside the leaf node that contains the given position
				if (currentLayer == 0)
				{
					const auto& leaf = mOctree.LeafNodes[node.GetFirstChild().NodeIndex];
					float voxelHalfSize = mLayerVoxelHalfSizeCache[currentLayer];
					float leafSize = voxelHalfSize / 2;

					FVector nodePosition;
					GetNodePosition(currentLayer, node.GetMortonCode(), nodePosition);
					FVector nodeOrigin = nodePosition - FVector(voxelHalfSize);
					FVector nodeLocalPosition = position - nodeOrigin;

					int32 leafIndex;
					FVector leafPosition;
					bool found = false;
					for (leafIndex = 0; leafIndex < 64; ++leafIndex)
					{
						uint_fast32_t x, y, z;
						libmorton::morton3D_64_decode(leafIndex, x, y, z);
						leafPosition = nodeOrigin + FVector(x * leafSize, y * leafSize, z * leafSize) + FVector(leafSize / 2);
						if ((position.X >= leafPosition.X - (leafSize / 2)) && (position.X <= leafPosition.X + (leafSize / 2)) &&
							(position.Y >= leafPosition.Y - (leafSize / 2)) && (position.Y <= leafPosition.Y + (leafSize / 2)) &&
							(position.Z >= leafPosition.Z - (leafSize / 2)) && (position.Z <= leafPosition.Z + (leafSize / 2)))
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
#if WITH_EDITOR
						UE_LOG(CinnamonLog, Warning, TEXT("GetLinkFromPosition: could not find node."));
#endif
						return false;
					}

					/*FIntVector mortonPosition;
					mortonPosition.X = FMath::FloorToInt(nodeLocalPosition.X / (voxelHalfSize / 2));
					mortonPosition.Y = FMath::FloorToInt(nodeLocalPosition.Y / (voxelHalfSize / 2));
					mortonPosition.Z = FMath::FloorToInt(nodeLocalPosition.Z / (voxelHalfSize / 2));*/

					link.SetLayerIndex(currentLayer);
					link.SetNodeIndex(i);

					//MortonCodeType leafIndex = libmorton::morton3D_64_encode(mortonPosition.X, mortonPosition.Y, mortonPosition.Z);

					if (leaf.GetSubnode(leafIndex))
					{
#if WITH_EDITOR
						UE_LOG(CinnamonLog, Warning, TEXT("GetLinkFromPosition: node at position %s is blocked."), *position.ToString());
						DrawNodeVoxel(leafPosition, FVector(mLayerVoxelHalfSizeCache[0] / 4), FColor::Emerald);
						DrawNodeVoxel(nodePosition, FVector(mLayerVoxelHalfSizeCache[0]), DebugHelper::LayerColors[0]);
#endif
						return false;
					}

					link.SetSubnodeIndex(static_cast<SubnodeIndexType>(leafIndex));

					return true;
				}

				// if no leaf node then we keep going down the octree to the next layer and 
				currentLayer = node.GetFirstChild().LayerIndex;
				currentNode = node.GetFirstChild().NodeIndex;

				break;
			}
		}
	}

#if WITH_EDITOR
	UE_LOG(CinnamonLog, Warning, TEXT("GetLinkFromPosition: could not find a node for the given link."));
#endif

	return false;
}

void ATDPVolume::GetVoxelMortonPosition(const FVector& position, const LayerIndexType layer, FIntVector& mortonPosition) const
{
	FVector mortonOrigin = mOrigin - mExtents;
	FVector localPosition = position - mortonOrigin;

	float voxelSize = mLayerVoxelHalfSizeCache[layer] * 2;

	mortonPosition.X = FMath::FloorToInt(localPosition.X / voxelSize);
	mortonPosition.Y = FMath::FloorToInt(localPosition.Y / voxelSize);
	mortonPosition.Z = FMath::FloorToInt(localPosition.Z / voxelSize);
}

int32 ATDPVolume::GetTotalLayers() const
{
	return mTotalLayers;
}

const TDPNode* ATDPVolume::GetNodeFromLink(const TDPNodeLink& link) const
{
	if (link.IsValid())
	{
		return &mOctree.GetLayer(link.LayerIndex)[link.NodeIndex];
	}

	return nullptr;
}

void ATDPVolume::GetNodeNeighborsFromLink(const TDPNodeLink& link, TArray<TDPNodeLink>& neighbors) const
{
	const auto node = GetNodeFromLink(link);

	if (node)
	{
		// check the six neighbor links
		for (int32 i = 0; i < 6; ++i)
		{
			const auto& neighborLink = node->GetNeighbors()[i];

			// nothing to do if link is invalid
			if (!neighborLink.IsValid())
			{
				continue;
			}

			const auto neighbor = GetNodeFromLink(neighborLink);

			if (neighbor)
			{
				// if neighbor has no children then just add it and continue
				if (!neighbor->HasChildren())
				{
					neighbors.Add(neighborLink);
					continue;
				}

				// if there are children then they need to be evaluated until highest precision is reached
				TArray<TDPNodeLink> remainingLinks;
				remainingLinks.Add(neighborLink);

				while (remainingLinks.Num() > 0)
				{
					auto currentLink = remainingLinks.Pop();
					const auto currentNode = GetNodeFromLink(currentLink);

					if (currentNode)
					{
						// highest precision reached with this child
						if (!currentNode->HasChildren())
						{
							neighbors.Add(currentLink);
							continue;
						}

						// non leaf nodes will have 8 children and 4 will be neighbors for each face
						if (currentLink.LayerIndex > 0)
						{
							for (const auto& childIndex : NodeHelper::ChildOffsets[i])
							{
								auto childLink = currentNode->GetFirstChild();
								childLink.NodeIndex += childIndex;
								const auto childNode = GetNodeFromLink(childLink);

								if (childNode)
								{
									// more work to do if there are more children
									if (childNode->HasChildren())
									{
										remainingLinks.Emplace(childLink);
									}
									// no children, highest precision this route
									else
									{
										neighbors.Emplace(childLink);
									}
								}
							}
						}
						// leaf nodes have 64 children and 16 will be facing the given node
						else
						{
							for (const auto& leafIndex : NodeHelper::LeafChildOffsets[i])
							{
								auto leafLink = currentNode->GetFirstChild();
								const auto& leafNode = mOctree.LeafNodes[leafLink.NodeIndex];
								leafLink.SetSubnodeIndex(leafIndex);

								// only add them if they are not blocked
								if (!leafNode.GetSubnode(leafIndex))
								{
									neighbors.Emplace(leafLink);
								}
							}
						}
					}
				}
			}
		}
	}
}

void ATDPVolume::GetLeafNeighborsFromLink(const TDPNodeLink& link, TArray<TDPNodeLink>& neighbors) const
{
	MortonCodeType leafIndex = link.SubnodeIndex;
	const auto node = GetNodeFromLink(link);

	if (node)
	{
		const auto& leaf = mOctree.LeafNodes[node->GetFirstChild().NodeIndex];

		uint_fast32_t x = 0, y = 0, z = 0;
		libmorton::morton3D_64_decode(leafIndex, x, y, z);

		for (int32 i = 0; i < 6; ++i)
		{
			FIntVector mortonPosition{ static_cast<int32>(x), static_cast<int32>(y), static_cast<int32>(z) };
			mortonPosition += NodeHelper::NeighborDirections[i];

			if (mortonPosition.X >= 0 && mortonPosition.X < 4 &&
				mortonPosition.Y >= 0 && mortonPosition.Y < 4 &&
				mortonPosition.Z >= 0 && mortonPosition.Z < 4)
			{
				MortonCodeType neighborIndex = libmorton::morton3D_64_encode(mortonPosition.X, mortonPosition.Y, mortonPosition.Z);

				if (leaf.GetSubnode(neighborIndex))
				{
					continue;
				}
				else
				{
					neighbors.Emplace(0, link.NodeIndex, neighborIndex);
					continue;
				}
			}
			else
			{
				const auto& neighborLink = node->GetNeighbors()[i];
				const auto neighborNode = GetNodeFromLink(neighborLink);

				if (neighborNode)
				{
					if (!neighborNode->GetFirstChild().IsValid())
					{
						neighbors.Add(neighborLink);
						continue;
					}

					const auto& leafNode = mOctree.LeafNodes[neighborNode->GetFirstChild().NodeIndex];

					if (leafNode.IsFullyBlocked())
					{
						continue;
					}
					else
					{
						if (mortonPosition.X < 0)
						{
							mortonPosition.X = 3;
						}
						else if (mortonPosition.X > 3)
						{
							mortonPosition.X = 0;
						}
						else if (mortonPosition.Y < 0)
						{
							mortonPosition.Y = 3;
						}
						else if (mortonPosition.Y > 3)
						{
							mortonPosition.Y = 0;
						}
						if (mortonPosition.Z < 0)
						{
							mortonPosition.Z = 3;
						}
						else if (mortonPosition.Z > 3)
						{
							mortonPosition.Z = 0;
						}

						MortonCodeType subnodeIndex = libmorton::morton3D_64_encode(mortonPosition.X, mortonPosition.Y, mortonPosition.Z);

						if (!leafNode.GetSubnode(subnodeIndex))
						{
							neighbors.Emplace(0, neighborNode->GetFirstChild().NodeIndex, subnodeIndex);
						}
					}
				}
			}
		}
	}
}

bool ATDPVolume::IsPointInside(const FVector& point) const
{
	return GetComponentsBoundingBox(true).IsInside(point);
}

void ATDPVolume::DrawVoxelFromLink(const TDPNodeLink& link, const FColor& color, const FString& label) const
{
	FVector position;
	GetNodePositionFromLink(link, position);

	const auto& node = mOctree.GetLayer(link.LayerIndex)[link.NodeIndex];
	float size = mLayerVoxelHalfSizeCache[link.LayerIndex];

	// if layer 0 and valid children
	if (link.LayerIndex == 0 && node.GetFirstChild().IsValid())
	{
		size /= 4;
	}

	DrawNodeVoxel(position, FVector(size), color);
	DrawDebugString(GetWorld(), position, label, nullptr, color);
}

void ATDPVolume::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (mEnableSerialization)
	{
		Ar << mOctree;
		Ar << mLayerVoxelHalfSizeCache;

		mTotalLayers = mOctree.Layers.Num();
		mTotalBytes = mOctree.MemoryUsage();
	}
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

