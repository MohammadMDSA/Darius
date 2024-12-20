#include "pch.hpp"
#include "Demo.hpp"

#include <Demo/DetailDrawTest.hpp>
#include <Demo/MovementBehaviour.hpp>
#include <Demo/GameObjectReferencer.hpp>
#include <Demo/LaserShoot.hpp>
#include <Demo/Targeter.hpp>
#include <Demo/TriggerTest.hpp>
#include <Demo/CollisionTest.hpp>
#include <Demo/ComponentReferencer.hpp>
#include <Demo/CharacterControllerInput.hpp>
#include <Demo/BulletShooter.hpp>
#include <Demo/Utils/RotatingMovementComponent.hpp>
#include <Demo/LimitedLifeTimeComponent.hpp>
#include <Demo/Gun.hpp>
#include <Demo/Pinball/PinballHandle.hpp>
#include <Demo/Pinball/PinballBouncer.hpp>
#include <Demo/PinballMaster/PinballMasterHandle.hpp>
#include <Demo/PinballMaster/PinballMasterBouncer.hpp>
#include <Demo/SpaceInvaders/SpaceInvadersJet.hpp>
#include <Demo/SpaceInvaders/SpaceInvadersBullet.hpp>

void DemoProject::Initialize()
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
	Demo::RotatingMovementComponent::StaticConstructor();
	Demo::BulletShooter::StaticConstructor();
	Demo::LimitedLifeTimeComponent::StaticConstructor();
	Demo::Gun::StaticConstructor();
	Demo::PinballHandle::StaticConstructor();
	Demo::PinballBouncer::StaticConstructor();
	Demo::PinballMasterHandle::StaticConstructor();
	Demo::PinballMasterBouncer::StaticConstructor();
	Demo::SpaceInvadersJet::StaticConstructor();
	Demo::SpaceInvadersBullet::StaticConstructor();
}

void DemoProject::Shutdown()
{ }