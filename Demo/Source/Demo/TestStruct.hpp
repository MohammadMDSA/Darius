#pragma once

#include <rttr/registration_friend.h>
#include <rttr/rttr_enable.h>

#include "TestStruct.generated.hpp"

struct DStruct(Serialize) TestContainer
{
	DField(Serialize)
	int								mFoo = 1;

	DField(Serialize)
	float							mBar = 0.5;

	TestContainer_GENERATED
};

File_TestStruct_GENERATED
