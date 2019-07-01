// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPLeafNode.h"


void TDPLeafNode::SetSubnode(SubnodeIndexType subnode)
{
	mSubnodes |= 1ULL << subnode;
}

void TDPLeafNode::ClearSubnode(SubnodeIndexType subnode)
{
	mSubnodes &= !(1ULL << subnode);
}

bool TDPLeafNode::GetSubnode(MortonCodeType code) const
{
	return (mSubnodes & (1ULL << code)) != 0;
}

uint64& TDPLeafNode::GetSubnodes()
{
	return mSubnodes;
}

const uint64& TDPLeafNode::GetSubnodes() const
{
	return mSubnodes;
}

bool TDPLeafNode::IsFullyBlocked() const
{
	return mSubnodes == -1;
}

bool TDPLeafNode::IsEmpty() const
{
	return !mSubnodes;
}
