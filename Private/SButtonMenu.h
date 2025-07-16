#pragma once

class SButtonMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SButtonMenu){}
	SLATE_END_ARGS()
	
public:
	SButtonMenu();
	~SButtonMenu();
	void Construct(const FArguments& InArgs);

};
