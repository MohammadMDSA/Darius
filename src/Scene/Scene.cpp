#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "Scene.hpp"

#include <Core/Memory/Memory.hpp>
#include <Utils/Assert.hpp>

namespace Darius::Scene
{
	bool								_initialized = false;

	//D_MEMORY::Pool<GameObject>			GameObjectPool;

	/*bool Create(std::string& name)
	{
		GameObjectPool = D_MEMORY::Pool<GameObject>()
	}*/
	
	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

}