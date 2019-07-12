// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "TDPDefinitions.h"
#include "TDPTree.h"
#include "TDPVolume.generated.h"

class UTDPDynamicObstacleComponent;

/**
 * 
 */
UCLASS(Category="3D Pathfinding")
class CINNAMON_API ATDPVolume : public AVolume
{
	GENERATED_BODY()

public:
	ATDPVolume(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;

	//~ Begin AActor Interface
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;
	//~ End AActor Interface

	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditUndo() override;
	void OnPostShapeChanged();

	bool ShouldTickIfViewportsOnly() const override;
	//~ End UObject Interface
#endif

	UFUNCTION(BlueprintCallable, Category = "3D Pathfinding")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "3D Pathfinding")
	void Generate();

	UFUNCTION(BlueprintCallable, Category = "3D Pathfinding")
	void Clear();

	UFUNCTION(BlueprintCallable, Category = "3D Pathfinding Debug")
	void DrawOctree() const;

	UFUNCTION(BlueprintCallable, Category = "3D Pathfinding Debug")
	void DrawLeafNodes() const;

	UFUNCTION(BlueprintCallable, Category = "3D Pathfinding Debug")
	void DrawBlockedMiniLeafNodes() const;

	UFUNCTION(BlueprintCallable, Category = "3D Pathfinding Debug")
	void FlushDrawnOctree() const;

	UFUNCTION(BlueprintCallable, Category = "3D Pathfinding")
	FVector GetRandomPointInVolume() const;

public:
	// Debug Info
#if WITH_EDITOR
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawVoxels = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawLeafVoxels = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawOnlyBlockedLeafVoxels = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	float VoxelThickness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawMiniLeafVoxels = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawOctreeMortonCodes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawLeafMortonCodes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawParentChildLinks = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawValidNeighborLinks = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawInvalidNeighborLinks = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	float LinkSize = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawCollisionVoxels = false;
#endif

public:
	const TDPTree& GetOctree() const;
	float GetVoxelSizeInLayer(LayerIndexType layer) const;
	bool GetNodePositionFromLink(TDPNodeLink link, FVector& position) const;
	bool GetLinkFromPosition(const FVector& position, TDPNodeLink& link) const;
	void GetVoxelMortonPosition(const FVector& position, const LayerIndexType layer, FIntVector& mortonPosition) const;
	int32 GetTotalLayers() const;
	TDPNode* GetNodeFromLink(const TDPNodeLink& link);
	const TDPNode* GetNodeFromLink(const TDPNodeLink& link) const;
	void GetNodeNeighborsFromLink(const TDPNodeLink& link, TArray<TDPNodeLink>& neighbors) const;
	void GetLeafNeighborsFromLink(const TDPNodeLink& link, TArray<TDPNodeLink>& neighbors) const;
	bool IsPointInside(const FVector& point) const;
	void DrawVoxelFromLink(const TDPNodeLink& link, const FColor& color = FColor::Black, const FString& label = FString()) const;

	void RequestOctreeUpdate(UTDPDynamicObstacleComponent& obstacle);
	TArray<TDPNodeLink> GetAffectedNodes(AActor* actor) const;
	TArray<TDPNodeLink> GetAffectedNodes(const FBox& box, const TSet<AActor*>& filter = TSet<AActor*>()) const;

	virtual void Serialize(FArchive& Ar) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Enable Serialization"))
	bool mEnableSerialization = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Layers"))
	int32 mLayers = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Collision Channel"))
	TEnumAsByte<ECollisionChannel> mCollisionChannel = ECollisionChannel::ECC_Pawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Collision Clearance"))
	float mCollisionClearance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Complex Collision"))
	bool mComplexCollision = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Total Layers"))
	int32 mTotalLayers = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Total Layer Nodes"))
	int32 mTotalLayerNodes = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Total Leaf Nodes"))
	int32 mTotalLeafNodes = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Total Bytes"))
	int32 mTotalBytes = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Origin"))
	FVector mOrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Extents"))
	FVector mExtents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Dynamic Update Enabled"))
	bool mDynamicUpdateEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Optimized Dynamic Update"))
	bool mOptimizedDynamicUpdate = false;

	TDPTree mOctree;
	TArray<TSet<MortonCodeType>> mBlockedIndices;

	bool mOctreeDirty = false;

	TSet<AActor*> mActors;
	TArray<UTDPDynamicObstacleComponent*> mPendingDynamicObstacles;

	TSet<TDPNodeLink> mInvalidNodes;

private:
	void RasterizeLowRes();
	void RasterizeLayer(LayerIndexType layer);
	void RasterizeLeafNode(const FVector& origin, NodeIndexType leaf);
	void SetNeighborLinks(const LayerIndexType layer);
	bool FindNeighborLink(const LayerIndexType layerIndex, const NodeIndexType nodeIndex, uint8 direction, TDPNodeLink& link, const FVector& nodePosition);
	void UpdateOctree();
	void UpdateNode(const TDPNodeLink link);
	void UpdateLeafNode(const FVector& origin, NodeIndexType leaf);

	bool IsNodeBlocked(LayerIndexType layer, MortonCodeType code) const;
	bool IsVoxelBlocked(const FVector& position, const float halfSize, bool useClearance = false) const;
	bool IsVoxelBlocked(const FVector& position, const float halfSize, const TSet<AActor*>& filter, bool useClearance = false) const;

	int32 GetNodeAmountInLayer(LayerIndexType layer) const;
	bool GetNodeIndexFromMortonCode(const LayerIndexType layer, const MortonCodeType nodeCode, NodeIndexType& index) const;
	void GetNodePosition(LayerIndexType layer, MortonCodeType code, FVector& position) const;
	NodeIndexType FindInsertIndex(LayerIndexType layer, MortonCodeType code) const;

	void DrawNodeVoxel(const LayerIndexType layer, const TDPNode& node) const;
	void DrawNodeVoxel(const FVector& position, const FVector& extent, const FColor& color) const;

	TArray<float> mLayerVoxelHalfSizeCache;
};
