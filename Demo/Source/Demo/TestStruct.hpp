#pragma once

#include "TestStruct2.hpp"

#include <rttr/registration_friend.h>
#include <rttr/rttr_enable.h>

#include "TestStruct.generated.hpp"

struct DStruct(Serialize) TestContainer
{

	TestContainer();

	DField(Serialize)
	int								mFoo = 1;

	DField(Serialize)
	float							mBar = 0.5;

	DField(Serialize)
	TestContainer2					mInner;

	TestContainer_GENERATED
};

File_TestStruct_GENERATED
