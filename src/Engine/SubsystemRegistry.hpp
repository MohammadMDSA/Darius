#pragma once

#ifndef D_SUBSYSTEM
#define D_SUBSYSTEM Darius::Subsystem
#endif

namespace Darius::Subsystem
{
	void RegisterSubsystems();
	void InitialzieSubsystems();
	void ShutdownSubsystems();

}
