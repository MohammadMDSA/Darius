#pragma once

#include <rttr/rttr_enable.h>

interface Detailed
{
	RTTR_ENABLE()
public:
#ifdef _D_EDITOR
	virtual bool DrawDetails(float params[]) = 0;
	virtual bool IsEditableInDetailsWindow() const = 0;
#endif // _D_EDITOR

};