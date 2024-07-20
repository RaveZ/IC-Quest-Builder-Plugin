// Copyright 2024 Ivan Chandra. All Rights Reserved.


#include "SGraphNode_QuestSystemNode.h"
#include "SlateOptMacros.h"
#include "Types/SlateStructs.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SToolTip.h"
#include "SGraphPanel.h"
#include "SGraphPreviewer.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "SGraphPin.h"
#include "Colors_QuestSystem.h"
#include "GraphEditorSettings.h"
#include "SCommentBubble.h"
#include "SLevelOfDetailBranchNode.h"
#include "QuestSystemGraphNode.h"

#define LOCTEXT_NAMESPACE "EdNode_GenericGraph"

//////////////////////////////////////////////////////////////////////////
class SQuestSystemPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SQuestSystemPin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin)
	{
		this->SetCursor(EMouseCursor::Default);

		bShowLabel = true;

		GraphPinObj = InPin;
		check(GraphPinObj != nullptr);

		const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
		check(Schema);

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(this, &SQuestSystemPin::GetPinBorder)
			.BorderBackgroundColor(this, &SQuestSystemPin::GetPinColor)
			.OnMouseButtonDown(this, &SQuestSystemPin::OnPinMouseDown)
			.Cursor(this, &SQuestSystemPin::GetPinCursor)
			.Padding(FMargin(5.0f))
		);
	}

protected:
	virtual FSlateColor GetPinColor() const override
	{
		return bIsDiffHighlighted ? QuestSystemColors::Pin::Diff :
			IsHovered() ? QuestSystemColors::Pin::Hover : QuestSystemColors::Pin::Default;
	}

	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override
	{
		return SNew(STextBlock);
	}

	const FSlateBrush* GetPinBorder() const
	{
		return FAppStyle::GetBrush(TEXT("Graph.StateNode.Body"));
	}

	//virtual TSharedRef<FDragDropOperation> SpawnPinDragEvent(const TSharedRef<class SGraphPanel>& InGraphPanel, const TArray< TSharedRef<SGraphPin> >& InStartingPins) override
	//{
	//	FGenericGraphDragConnection::FDraggedPinTable PinHandles;
	//	PinHandles.Reserve(InStartingPins.Num());
	//	// since the graph can be refreshed and pins can be reconstructed/replaced 
	//	// behind the scenes, the DragDropOperation holds onto FGraphPinHandles 
	//	// instead of direct widgets/graph-pins
	//	for (const TSharedRef<SGraphPin>& PinWidget : InStartingPins)
	//	{
	//		PinHandles.Add(PinWidget->GetPinObj());
	//	}

	//	return FGenericGraphDragConnection::New(InGraphPanel, PinHandles);
	//}

};


void SGraphNode_QuestSystemNode::Construct(const FArguments& InArgs, UEdNode_QuestSystemNode* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
	InNode->SGraphNode = this;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGraphNode_QuestSystemNode::UpdateGraphNode()
{
	const FMargin NodePadding = FMargin(5);
	const FMargin NamePadding = FMargin(2);

	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	const FSlateBrush* NodeTypeIcon = GetNameIcon();

	FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
	TSharedPtr<SErrorText> ErrorText;
	TSharedPtr<SVerticalBox> NodeBody;
	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	TWeakPtr<SNodeTitle> WeakNodeTitle = NodeTitle;

	TSharedPtr<STextBlock> DescriptionText;
	auto GetNodeTitlePlaceholderWidth = [WeakNodeTitle]() -> FOptionalSize
		{
			TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
			const float DesiredWidth = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().X : 0.0f;
			return FMath::Max(75.0f, DesiredWidth);
		};
	auto GetNodeTitlePlaceholderHeight = [WeakNodeTitle]() -> FOptionalSize
		{
			TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
			const float DesiredHeight = (NodeTitlePin.IsValid()) ? NodeTitlePin->GetTitleSize().Y : 0.0f;
			return FMath::Max(22.0f, DesiredHeight);
		};
	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(3.0f)
			.BorderBackgroundColor(this, &SGraphNode_QuestSystemNode::GetHighlightColor)
			[
				SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body"))
					.Padding(0.0f)
					.BorderBackgroundColor(this, &SGraphNode_QuestSystemNode::GetBorderBackgroundColor)
					//.OnMouseButtonDown(this, &SGraphNode_QuestSystemNode::OnMouseDown)
					[
						SNew(SOverlay)

							// Pins and node details
							+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(SHorizontalBox)

									// INPUT PIN AREA
									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SBox)
											.MinDesiredHeight(NodePadding.Top)
											[
												SAssignNew(LeftNodeBox, SVerticalBox)
											]
									]

									// STATE NAME AREA
									+ SHorizontalBox::Slot()
									.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
									[
										SNew(SVerticalBox)
											/*+ SVerticalBox::Slot()
											.AutoHeight()
											[
												DecoratorsBox.ToSharedRef()
											]*/
											+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SNew(SOverlay)
												+ SOverlay::Slot()
												.HAlign(HAlign_Center)
												.VAlign(VAlign_Center)
												.Padding(8.0f)
												[
													SNew(SBorder)
														.BorderImage(FAppStyle::GetBrush("Graph.StateNode.ColorSpill"))
														.BorderBackgroundColor(TitleShadowColor)
														.HAlign(HAlign_Center)
														.VAlign(VAlign_Center)
														.Visibility(EVisibility::SelfHitTestInvisible)
														.Padding(6.0f)
														[
															SAssignNew(NodeBody, SVerticalBox)

																// Title
																+ SVerticalBox::Slot()
																.AutoHeight()
																[
																	SNew(SHorizontalBox)

																		// Error message
																		+ SHorizontalBox::Slot()
																		.AutoWidth()
																		[
																			SAssignNew(ErrorText, SErrorText)
																				.BackgroundColor(this, &SGraphNode_QuestSystemNode::GetErrorColor)
																				.ToolTipText(this, &SGraphNode_QuestSystemNode::GetErrorMsgToolTip)
																		]

																		// Node Title
																		+ SHorizontalBox::Slot()
																		.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
																		[
																			SNew(SVerticalBox)
																				+ SVerticalBox::Slot()
																				.AutoHeight()
																				[
																					SAssignNew(InlineEditableText, SInlineEditableTextBlock)
																						.Style(FAppStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
																						.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
																						.OnVerifyTextChanged(this, &SGraphNode_QuestSystemNode::OnVerifyNameTextChanged)
																						.OnTextCommitted(this, &SGraphNode_QuestSystemNode::OnNameTextCommited)
																						.IsReadOnly(this, &SGraphNode_QuestSystemNode::IsNameReadOnly)
																						.IsSelected(this, &SGraphNode_QuestSystemNode::IsSelectedExclusively)
																				]
																				+ SVerticalBox::Slot()
																				.AutoHeight()
																				[
																					NodeTitle.ToSharedRef()
																				]
																				+ SVerticalBox::Slot()
																				.AutoHeight()
																				[
																					// DESCRIPTION MESSAGE
																					SAssignNew(DescriptionText, STextBlock)
																						.Visibility(this, &SGraphNode_QuestSystemNode::GetDescriptionVisibility)
																						.Text(this, &SGraphNode_QuestSystemNode::GetDescription)
																				]
																		]
																]
														]
												]
											]
								
									]

									// OUTPUT PIN AREA
									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SBox)
											.MinDesiredHeight(NodePadding.Bottom)
											[
												SAssignNew(RightNodeBox, SVerticalBox)
													/*+ SVerticalBox::Slot()
													.HAlign(HAlign_Fill)
													.VAlign(VAlign_Fill)
													.Padding(20.0f, 0.0f)
													.FillHeight(1.0f)
													[
														SAssignNew(OutputPinBox, SHorizontalBox)
													]*/
											]
									]
							]

							// Drag marker overlay
							/*+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Top)
							[
								SNew(SBorder)
									.BorderBackgroundColor(QuestSystemColors::Action::DragMarker)
									.ColorAndOpacity(QuestSystemColors::Action::DragMarker)
									.BorderImage(FAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
									.Visibility(this, &SGraphNode_QuestSystemNode::GetDragOverMarkerVisibility)
									[
										SNew(SBox)
											.HeightOverride(4)
									]
							]*/

							// Blueprint indicator overlay
							+ SOverlay::Slot()
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Top)
							[
								SNew(SImage)
									.Image(FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Blueprint")))
									.Visibility(this, &SGraphNode_QuestSystemNode::GetBlueprintIconVisibility)
							]
					]
			]
	];
	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);
	CreatePinWidgets();
}

void SGraphNode_QuestSystemNode::CreatePinWidgets()
{
	UEdNode_QuestSystemNode* StateNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = SNew(SQuestSystemPin, MyPin);

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SGraphNode_QuestSystemNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility(TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced));
	}

	TSharedPtr<SVerticalBox> PinBox;
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		PinBox = LeftNodeBox;
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		PinBox = RightNodeBox;
		OutputPins.Add(PinToAdd);
	}

	if (PinBox)
	{
		PinBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			//.Padding(6.0f, 0.0f)
			[
				PinToAdd
			];
	}
}

bool SGraphNode_QuestSystemNode::IsNameReadOnly() const
{
	UEdNode_QuestSystemNode* EdNode_Node = Cast<UEdNode_QuestSystemNode>(GraphNode);
	check(EdNode_Node != nullptr);

	UQuestSystemGraph* QuestSystemGraph = EdNode_Node->QuestSystemGraphNode->QuestGraph;
	check(QuestSystemGraph != nullptr);

	return (!QuestSystemGraph->bCanRenameNode || !EdNode_Node->QuestSystemGraphNode->IsNameEditable()) || SGraphNode::IsNameReadOnly();
}
FText SGraphNode_QuestSystemNode::GetDescription() const
{
	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	return MyNode ? MyNode->GetDescription() : FText::GetEmpty();
}
EVisibility SGraphNode_QuestSystemNode::GetDescriptionVisibility() const
{
	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail) ? EVisibility::Visible : EVisibility::Collapsed;

}
bool SGraphNode_QuestSystemNode::OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
{
	FString TextAsString = InText.ToString();

	FName OriginalName;

	bool bValid(true);

	if ((GetEditableNodeTitle() != InText.ToString()) && OnVerifyTextCommit.IsBound())
	{
		OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "Name Not Valid.");
		bValid = OnVerifyTextCommit.Execute(InText, GraphNode, OutErrorMessage);
	}

	if (UEdNode_QuestSystemNode* MyNode = Cast<UEdNode_QuestSystemNode>(GraphNode))
	{
		OriginalName = MyNode->QuestSystemGraphNode->ID;
	}

	for (TObjectIterator<UQuestSystemGraphNode> It; It; ++It)
	{
		if (UQuestSystemGraphNode* QuestSystem = Cast<UQuestSystemGraphNode>(*It))
		{
			if (FName(InText.ToString()) == OriginalName)
			{
				return true;
			}
			if (QuestSystem->ID == FName(InText.ToString()))
			{
				OutErrorMessage = LOCTEXT("RenameFailed_NotValid", "This ID is already used in another Node.");
				return false;
			}
		}
	}


	return bValid;
}
FText SGraphNode_QuestSystemNode::GetPinTooltip(UEdGraphPin* GraphPinObj) const
{
	return FText();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGraphNode_QuestSystemNode::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	SGraphNode::OnNameTextCommited(InText, CommitInfo);

	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);

	if (MyNode != nullptr && MyNode->QuestSystemGraphNode != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("QuestSystemEditorRenameNode", "Quest System Editor: Rename Node"));
		MyNode->Modify();
		MyNode->QuestSystemGraphNode->Modify();
		MyNode->QuestSystemGraphNode->SetNodeTitle(InText);
		UpdateGraphNode();
	}
}

FSlateColor SGraphNode_QuestSystemNode::GetBorderBackgroundColor() const
{
	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	return MyNode ? MyNode->GetBackgroundColor() : QuestSystemColors::NodeBorder::HighlightAbortRange0;
}

FSlateColor SGraphNode_QuestSystemNode::GetHighlightColor() const
{
	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	if (MyNode->QuestSystemGraphNode->QuestSystem == nullptr)
		return QuestSystemColors::NodeBorder::NoHighlight;


	if (MyNode->QuestSystemGraphNode->QuestSystem->CurrentNode == MyNode->QuestSystemGraphNode && (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld))
	{
		return QuestSystemColors::NodeBorder::HighlightCurrentNode;
		
	}
	else
	{
		return QuestSystemColors::NodeBorder::NoHighlight;
	}
}

FSlateColor SGraphNode_QuestSystemNode::GetBackgroundColor() const
{
	return QuestSystemColors::NodeBody::Default;
}

EVisibility SGraphNode_QuestSystemNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

EVisibility SGraphNode_QuestSystemNode::GetDebuggerSearchFailedMarkerVisibility() const
{
	return EVisibility();
}

const FSlateBrush* SGraphNode_QuestSystemNode::GetNameIcon() const
{
	return FAppStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

EVisibility SGraphNode_QuestSystemNode::GetBlueprintIconVisibility() const
{
	UEdNode_QuestSystemNode* MyNode = CastChecked<UEdNode_QuestSystemNode>(GraphNode);
	const bool bCanShowIcon = (MyNode != nullptr && MyNode->UsesBlueprint());

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return (bCanShowIcon && (!MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail)) ? EVisibility::Visible : EVisibility::Collapsed;

}

#undef LOCTEXT_NAMESPACE
