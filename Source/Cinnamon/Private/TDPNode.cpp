// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPNode.h"

TDPNode::TDPNode() : 
	mMortonCode(0), mParent(TDPNodeLink::InvalidLink), mFirstChild(TDPNodeLink::InvalidLink)
{
}

MortonCodeType TDPNode::GetMortonCode() const
{
	return mMortonCode;
}

void TDPNode::SetMortonCode(MortonCodeType code)
{
	mMortonCode = code;
}

TDPNodeLink& TDPNode::GetParent()
{
	return mParent;
}

const TDPNodeLink& TDPNode::GetParent() const
{
	return mParent;
}

void TDPNode::SetParent(TDPNodeLink node)
{
	mParent = node;
}

TDPNodeLink& TDPNode::GetFirstChild()
{
	return mFirstChild;
}

const TDPNodeLink& TDPNode::GetFirstChild() const
{
	return mFirstChild;
}

void TDPNode::SetFirstChild(TDPNodeLink node)
{
	mFirstChild = node;
}

TDPNodeLink* TDPNode::GetNeighbors()
{
	return mNeighbors;
}

const TDPNodeLink* TDPNode::GetNeighbors() const
{
	return mNeighbors;
}

bool TDPNode::HasChildren() const
{
	return mFirstChild.IsValid();
}
