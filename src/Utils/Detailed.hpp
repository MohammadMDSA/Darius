#pragma once

#include <rttr/rttr_enable.h>

interface Detailed
{
	RTTR_ENABLE()
public:
#ifdef _D_EDITOR
	virtual std::string GetDetailedName() const = 0;
	virtual Detailed* GetDetailedParent() const = 0;
	virtual bool DrawDetails(float params[]) = 0;
	virtual bool IsEditableInDetailsWindow() const = 0;
#endif // _D_EDITOR

};