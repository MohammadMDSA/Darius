#pragma once

#include <Core/Application.hpp>

class DemoProject : public D_APP::GameProject
{
public:
	virtual void Initialize() override;
	virtual void Shutdown() override;
};