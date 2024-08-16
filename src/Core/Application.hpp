#pragma once

#include <Core/Signal.hpp>

#ifndef D_APP
#define D_APP Darius::Core::Application
#endif // !D_CORE

namespace Darius::Core::Application
{
	// Events
	D_H_SIGNAL_DECL(AppSuspended, void());
	D_H_SIGNAL_DECL(AppResuming, void());
	D_H_SIGNAL_DECL(AppDeactivated, void());
	D_H_SIGNAL_DECL(AppActivated, void());
	D_H_SIGNAL_DECL(AppQuitting, void());
	D_H_SIGNAL_DECL(NewAudioDeviceConnected, void());

	// Internal
	void							_AppSuspended();
	void							_AppResuming();
	void							_AppDeactivated();
	void							_AppActivated();
	void							_AppQuitting();
	void							_NewAudioDeviceConnected();

	void							RequestMinimize();

	class GameProject
	{
	public:
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	};
}
