#pragma once

interface Detailed
{
#ifdef _D_EDITOR
	virtual bool DrawDetails(float params[]) = 0;
#endif // _D_EDITOR

};