// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPVolumeDetails.h"
#include "TDPVolume.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Components/BrushComponent.h"


#define LOCTEXT_NAMESPACE "TDPVolumeDetails"

TSharedRef<IDetailCustomization> TDPVolumeDetails::MakeInstance()
{
	return MakeShared<TDPVolumeDetails>();
}

void TDPVolumeDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	const TArray< TWeakObjectPtr<UObject> >& SelectedObjects = DetailBuilder.GetSelectedObjects();

	for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex)
	{
		const TWeakObjectPtr<UObject>& CurrentObject = SelectedObjects[ObjectIndex];
		if (CurrentObject.IsValid())
		{
			ATDPVolume* currentVolume = Cast<ATDPVolume>(CurrentObject.Get());
			if (currentVolume != nullptr)
			{
				mVolume = currentVolume;
				break;
			}
		}
	}

	// generate button
	DetailBuilder.EditCategory("3D Pathfinding")
		.AddCustomRow(NSLOCTEXT("SVO", "Generate", "Generate"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Generate", "Generate"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &TDPVolumeDetails::OnGenerateSVOClicked)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Generate", "Generate"))
		]
		];

	// clear button
	DetailBuilder.EditCategory("3D Pathfinding")
		.AddCustomRow(NSLOCTEXT("SVO", "Clear", "Clear"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Clear", "Clear"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &TDPVolumeDetails::OnClearSVOClicked)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Clear", "Clear"))
		]
		];

	// draw octree button
	DetailBuilder.EditCategory("3D Pathfinding")
		.AddCustomRow(NSLOCTEXT("SVO", "Draw SVO", "Draw SVO"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Draw SVO", "Draw SVO"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &TDPVolumeDetails::OnDrawSVOClicked)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Draw SVO", "Draw SVO"))
		]
		];

	// draw octree button
	DetailBuilder.EditCategory("3D Pathfinding")
		.AddCustomRow(NSLOCTEXT("SVO", "Draw Leaf Nodes", "Draw Leaf Nodes"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Draw Leaf Nodes", "Draw Leaf Nodes"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &TDPVolumeDetails::OnDrawLeafNodesClicked)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Draw Leaf Nodes", "Draw Leaf Nodes"))
		]
		];

	// draw octree button
	DetailBuilder.EditCategory("3D Pathfinding")
		.AddCustomRow(NSLOCTEXT("SVO", "Draw Mini Leaf Nodes", "Draw Mini Leaf Nodes"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Draw Mini Leaf Nodes", "Draw Mini Leaf Nodes"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &TDPVolumeDetails::OnDrawMiniLeafNodesClicked)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Draw Mini Leaf Nodes", "Draw Mini Leaf Nodes"))
		]
		];

	// draw octree button
	DetailBuilder.EditCategory("3D Pathfinding")
		.AddCustomRow(NSLOCTEXT("SVO", "Clear Drawn SVO", "Clear Drawn SVO"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Clear Drawn SVO", "Clear Drawn SVO"))
		]
	.ValueContent()
		.MaxDesiredWidth(125.f)
		.MinDesiredWidth(125.f)
		[
			SNew(SButton)
			.ContentPadding(2)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.OnClicked(this, &TDPVolumeDetails::OnFlushDrawnSVOClicked)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(NSLOCTEXT("SVO", "Clear Drawn SVO", "Clear Drawn SVO"))
		]
		];
}

FReply TDPVolumeDetails::OnGenerateSVOClicked()
{
	if (mVolume.IsValid())
	{
		mVolume->Generate();
	}

	return FReply::Handled();
}

FReply TDPVolumeDetails::OnClearSVOClicked()
{
	if (mVolume.IsValid())
	{
		mVolume->Clear();
	}

	return FReply::Handled();
}

FReply TDPVolumeDetails::OnDrawSVOClicked()
{
	if (mVolume.IsValid())
	{
		mVolume->DrawOctree();
	}

	return FReply::Handled();
}

FReply TDPVolumeDetails::OnDrawLeafNodesClicked()
{
	if (mVolume.IsValid())
	{
		mVolume->DrawLeafNodes();
	}

	return FReply::Handled();
}

FReply TDPVolumeDetails::OnDrawMiniLeafNodesClicked()
{
	if (mVolume.IsValid())
	{
		mVolume->DrawBlockedMiniLeafNodes();
	}

	return FReply::Handled();
}

FReply TDPVolumeDetails::OnFlushDrawnSVOClicked()
{
	if (mVolume.IsValid())
	{
		mVolume->FlushDrawnOctree();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
