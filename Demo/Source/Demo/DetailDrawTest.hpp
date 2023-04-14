#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "DetailDrawTest.generated.hpp"

namespace Demo
{
	enum class DEnum(Serialize) TestEnumz
	{
		Foo,
		Bar,
		Buz,
		Cat
	};

	class DClass(Serialize) DetailDrawTest : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(DetailDrawTest, D_ECS_COMP::BehaviourComponent, "Debug/DetailDrawTest", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

	private:
		DField(Serialize)
		bool							mBool = true;

		DField(Serialize)
		char							mChar = 55;

		DField(Serialize)
		INT8							mInt8 = -1;

		DField(Serialize)
		INT16							mInt16 = -2;

		DField(Serialize)
		INT32							mInt32 = -3;

		DField(Serialize)
		INT64							mInt64 = -4;

		DField(Serialize)
		UINT8							mUint8 = 5;

		DField(Serialize)
		UINT16							mUint16 = 6;

		DField(Serialize)
		UINT32							mUint32 = 7;
		
		DField(Serialize)
		UINT64							mUint64 = 8;

		DField(Serialize)
		std::string						mString = "Test";

		DField(Serialize)
		TestEnumz						mEnum = TestEnumz::Buz;

	public:
		Demo_DetailDrawTest_GENERATED
	};
}

File_DetailDrawTest_GENERATED
