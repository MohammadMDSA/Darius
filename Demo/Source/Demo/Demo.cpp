
#include <Demo/DetailDrawTest.hpp>
#include <Demo/MovementBehaviour.hpp>
#include <Demo/GameObjectReferencer.hpp>
#include <Demo/LaserShoot.hpp>
#include <Demo/Targeter.hpp>
#include <Demo/TriggerTest.hpp>
#include <Demo/CollisionTest.hpp>
#include <Demo/ComponentReferencer.hpp>
#include <Demo/CharacterControllerInput.hpp>

void InitializeGame()
{
	// Registering components
	Demo::MovementBehaviour::StaticConstructor();
	Demo::LaserShoot::StaticConstructor();
	Demo::DetailDrawTest::StaticConstructor();
	Demo::Targeter::StaticConstructor();
	Demo::GameObjectReferencer::StaticConstructor();
	Demo::TriggerTest::StaticConstructor();
	Demo::CollisionTest::StaticConstructor();
	Demo::ComponentReferencer::StaticConstructor();
	Demo::CharacterControllerInput::StaticConstructor();
}