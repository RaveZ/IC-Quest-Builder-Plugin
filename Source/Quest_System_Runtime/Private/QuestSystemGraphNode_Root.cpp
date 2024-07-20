// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "QuestSystemGraphNode_Root.h"
#include "QuestSystemGraphNode.h"


#if WITH_EDITOR
FText UQuestSystemGraphNode_Root::GetNodeDescription() const
{
	return FText::FromString(TEXT("Root Node"));
}
#endif