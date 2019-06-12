// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "TDPDefinitions.h"
#include "TDPTree.h"
#include "TDPVolume.generated.h"

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

public:
	// Debug Info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawVoxels = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	float VoxelThickness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding Debug")
	bool DrawLeafVoxels = false;

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

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Layers"))
	int32 mLayers = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Pathfinding", meta = (AllowPrivateAccess = true, DisplayName = "Collision Channel"))
	TEnumAsByte<ECollisionChannel> mCollisionChannel = ECollisionChannel::ECC_Pawn;

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

	TDPTree mOctree;
	TArray<TSet<MortonCodeType>> mBlockedIndices;

private:
	void RasterizeLowRes();
	void RasterizeLayer(LayerIndexType layer);
	void RasterizeLeafNode(const FVector& origin, NodeIndexType leaf);
	void SetNeighborLinks(const LayerIndexType layer);
	bool FindNeighborLink(const LayerIndexType layerIndex, const NodeIndexType nodeIndex, uint8 direction, TDPNodeLink& link, const FVector& nodePosition);

	bool IsNodeBlocked(LayerIndexType layer, MortonCodeType code) const;
	bool IsVoxelBlocked(const FVector& position, const float halfSize, bool useTolerance = false) const;

	void GetNodePosition(LayerIndexType layer, MortonCodeType code, FVector& position) const;
	int32 GetNodeAmountInLayer(LayerIndexType layer) const;
	float GetVoxelSizeInLayer(LayerIndexType layer) const;
	bool GetNodeIndexInLayer(const LayerIndexType layer, const MortonCodeType nodeCode, NodeIndexType& index) const;

	void DrawNodeVoxel(const FVector& position, const FVector& extent, const FColor& color) const;

	TArray<float> mLayerVoxelHalfSizeCache;
};