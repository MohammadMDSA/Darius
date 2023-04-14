#pragma once

#include <rttr/registration_friend.h>
#include <rttr/rttr_enable.h>

#include "TestStruct2.generated.hpp"

struct DStruct(Serialize) TestContainer2
{
	TestContainer2();

	DField(Serialize)
	int								mFoo = 1;

	DField(Serialize)
	float							mBar = 0.5;

	TestContainer2_GENERATED
};

File_TestStruct2_GENERATED
