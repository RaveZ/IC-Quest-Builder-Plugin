// Copyright 2024 Ivan Chandra. All Rights Reserved.

#pragma once
#include <EdGraphUtilities.h>
#include <EdGraph/EdGraphNode.h>

class FGraphPanelNodeFactory_GenericGraph : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};