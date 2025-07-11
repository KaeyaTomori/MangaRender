#pragma once

class SFileButton : public SButton
{
protected:
	// virtual void Press() override;

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
};
