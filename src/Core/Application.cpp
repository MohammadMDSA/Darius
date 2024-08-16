#include "pch.hpp"
#include "Application.hpp"


namespace Darius::Core::Application
{
	D_H_SIGNAL_DEFINITION(AppSuspended, void());
	D_H_SIGNAL_DEFINITION(AppResuming, void());
	D_H_SIGNAL_DEFINITION(AppDeactivated, void());
	D_H_SIGNAL_DEFINITION(AppActivated, void());
	D_H_SIGNAL_DEFINITION(AppQuitting, void());
	D_H_SIGNAL_DEFINITION(NewAudioDeviceConnected, void());


	void _AppSuspended()
	{
		AppSuspendedSignal();
	}

	void _AppResuming()
	{
		AppResumingSignal();
	}

	void _AppDeactivated()
	{
		AppDeactivatedSignal();
	}

	void _AppActivated()
	{
		AppActivatedSignal();
	}

	void _AppQuitting()
	{
		AppQuittingSignal();
	}

	void _NewAudioDeviceConnected()
	{
		NewAudioDeviceConnectedSignal();
	}

	void RequestMinimize()
	{
		::ShowWindow(GetActiveWindow(), SW_MINIMIZE);

	}

}