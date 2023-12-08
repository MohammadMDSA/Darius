#pragma once

#include <Utils/Common.hpp>

#include <rttr/registration_friend.h>
#include <rttr/rttr_enable.h>

#include "TestStruct2.generated.hpp"

struct DStruct(Serialize) TestContainer2
{
	GENERATED_BODY();

public:
	TestContainer2();

	DField(Serialize)
	int								mFoo = 1;

	DField(Serialize)
	float							mBar = 0.5;

};

File_TestStruct2_GENERATED
