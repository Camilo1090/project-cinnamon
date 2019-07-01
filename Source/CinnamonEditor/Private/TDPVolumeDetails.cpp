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
	//TSharedPtr<IPropertyHandle> PrimaryTickProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UActorComponent, PrimaryComponentTick));

	//// Defaults only show tick properties
	//if (PrimaryTickProperty->IsValidHandle() && DetailBuilder.HasClassDefaultObject())
	//{
	//	IDetailCategoryBuilder& TickCategory = DetailBuilder.EditCategory("ComponentTick");

	//	TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, bStartWithTickEnabled)));
	//	TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, TickInterval)));
	//	TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, bTickEvenWhenPaused)), EPropertyLocation::Advanced);
	//	TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, bAllowTickOnDedicatedServer)), EPropertyLocation::Advanced);
	//	TickCategory.AddProperty(PrimaryTickProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTickFunction, TickGroup)), EPropertyLocation::Advanced);
	//}

	//PrimaryTickProperty->MarkHiddenByCustomization();

	/*IDetailCategoryBuilder& navigationCategory = DetailBuilder.EditCategory("3D Pathfinding Debug");

	TSharedPtr<IPropertyHandle> drawVoxelProperty = DetailBuilder.GetProperty("DrawVoxels");
	TSharedPtr<IPropertyHandle> drawVoxelLeafProperty = DetailBuilder.GetProperty("DrawLeafVoxels");
	TSharedPtr<IPropertyHandle> drawOnlyBlockedVoxelLeafProperty = DetailBuilder.GetProperty("DrawOnlyBlockedLeafVoxels");
	TSharedPtr<IPropertyHandle> drawMortonCodesProperty = DetailBuilder.GetProperty("DrawMortonCodes");
	TSharedPtr<IPropertyHandle> drawNeighbourLinksProperty = DetailBuilder.GetProperty("DrawNeighbourLinks");
	TSharedPtr<IPropertyHandle> drawParentChildLinksProperty = DetailBuilder.GetProperty("DrawParentChildLinks");
	TSharedPtr<IPropertyHandle> voxelPowerProperty = DetailBuilder.GetProperty("myVoxelPower");
	TSharedPtr<IPropertyHandle> collisionChannelProperty = DetailBuilder.GetProperty("myCollisionChannel");
	TSharedPtr<IPropertyHandle> clearanceProperty = DetailBuilder.GetProperty("myClearance");
	TSharedPtr<IPropertyHandle> generationStrategyProperty = DetailBuilder.GetProperty("myGenerationStrategy");
	TSharedPtr<IPropertyHandle> numLayersProperty = DetailBuilder.GetProperty("myNumLayers");
	TSharedPtr<IPropertyHandle> numBytesProperty = DetailBuilder.GetProperty("myNumBytes");

	drawVoxelProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Debug Voxels", "Debug Voxels"));
	drawVoxelLeafProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Debug Leaf Voxels", "Debug Leaf Voxels"));
	drawMortonCodesProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Debug Morton Codes", "Debug Morton Codes"));
	drawNeighbourLinksProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Debug Links", "Debug Links"));
	drawParentChildLinksProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Parent Child Links", "Parent Child Links"));
	voxelPowerProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Layers", "Layers"));
	voxelPowerProperty->SetInstanceMetaData("UIMin", TEXT("1"));
	voxelPowerProperty->SetInstanceMetaData("UIMax", TEXT("12"));
	collisionChannelProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Collision Channel", "Collision Channel"));
	clearanceProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Clearance", "Clearance"));
	generationStrategyProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Generation Strategy", "Generation Strategy"));
	numLayersProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Num Layers", "Num Layers"));
	numBytesProperty->SetPropertyDisplayName(NSLOCTEXT("SVO Volume", "Num Bytes", "Num Bytes"));

	navigationCategory.AddProperty(voxelPowerProperty);
	navigationCategory.AddProperty(collisionChannelProperty);
	navigationCategory.AddProperty(clearanceProperty);
	navigationCategory.AddProperty(generationStrategyProperty);
	navigationCategory.AddProperty(numLayersProperty);
	navigationCategory.AddProperty(numBytesProperty);*/

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

	/*navigationCategory.AddProperty(debugDistanceProperty);
	navigationCategory.AddProperty(showVoxelProperty);
	navigationCategory.AddProperty(showVoxelLeafProperty);
	navigationCategory.AddProperty(showMortonCodesProperty);
	navigationCategory.AddProperty(showNeighbourLinksProperty);
	navigationCategory.AddProperty(showParentChildLinksProperty);*/
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

#undef LOCTEXT_NAMESPACE
