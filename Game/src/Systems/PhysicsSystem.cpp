#include "PhysicsSystem.h"
#include "../Core/Model.h"
#include "../Core/BoundingVolumes.h"
#include "../Components/Transform.h"
//#include "../Components/Physics.h"
#include "../Components/Player.h"
#include "../Components/Obstacle.h"
#include "../Components/Powerup.h"

#include "../ECSController.h"

extern ECSController controller;

PhysicsSystem::PhysicsSystem()
{
	controller.AddEventListener(Events::Physics::CREATE_ACTOR, [this](Event& e) { this->CreateActorListener(e); });
	controller.AddEventListener(Events::Player::PLAYER_JUMPED, [this](Event& e) { this->JumpEventListener(e); });
	controller.AddEventListener(Events::Player::RESET_VEHICLE, [this](Event& e) { this->ResetVehicleEventListener(e); });
	controller.AddEventListener(Events::Checkpoint::REACHED, [this](Event& e) {this->CheckpointReachedListener(e); });
	controller.AddEventListener(Events::Player::SPIN_OUT, [this](Event& e) {this->SpinOutListener(e); });
	controller.AddEventListener(Events::Player::BLAST, [this](Event& e) {this->BlastEventListener(e); });
	
	auto rigidBodyArray = controller.GetComponentArray<RigidBody>();
	rigidBodyArray->BindOnRemoveCallback([this](Entity entity, RigidBody& rb) { this->ReleaseActorCallback(entity, rb); });

	auto triggerArray = controller.GetComponentArray<Trigger>();
	triggerArray->BindOnRemoveCallback([this](Entity entity, Trigger& trig) { this->ReleaseTriggerCallback(entity, trig); });
}

void PhysicsSystem::Init()
{
	//gVehicle = std::make_unique<MainVehicle>();


	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	if (!gFoundation)
	{
		std::cout << "PxCreateFoundation failed!" << std::endl;
		return;
	}

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);


	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	if (!gPhysics)
	{
		std::cout << "PxCreatePhysics failed!" << std::endl;
		return;
	}

	PxSceneDesc PhysicsSceneDesc(gPhysics->getTolerancesScale());
	//PhysicsSceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	PhysicsSceneDesc.gravity = PxVec3(0.0f, -20.0f, 0.0f); // double gravity for vehicle testing
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	PhysicsSceneDesc.cpuDispatcher = gDispatcher;
	//PhysicsSceneDesc.filterShader = PxDefaultSimulationFilterShader;
	PhysicsSceneDesc.filterShader = VehicleFilterShader;


	gPhysicsScene = gPhysics->createScene(PhysicsSceneDesc);
	
	gPhysicsCallbacks = PhysicsCallbacks();

	gPhysicsScene->setSimulationEventCallback(&gPhysicsCallbacks);

	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/docs/DebugVisualization.html
	gPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	gPhysicsScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);


	// initialize materials
	materialMap["default"] = gPhysics->createMaterial(0.2f, 0.8f, 0.2f); 
	materialMap["vehicle_tire"] = gPhysics->createMaterial(0.72f, 0.72f, 0.1f);
	materialMap["ice"] = gPhysics->createMaterial(0.1f, 0.02f, 0.0f);
	materialMap["bouncy"] = gPhysics->createMaterial(0.5f, 0.5f, 1.0f);

	// initialize ground materials
	gGroundMaterial = materialMap["default"];


	// initialize PVD
	
	PxPvdSceneClient* pvdClient = gPhysicsScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	PxInitVehicleExtension(*gFoundation); // initialize vehicle extension

	CreateMap();
}

void PhysicsSystem::Update(float deltaTime)
{
	// get the vehicle command from ECS
	/*Entity vehicleEntity = controller.GetEntityByTag("VehicleCommands");
	auto& vehicleCommands = controller.GetComponent<VehicleCommands>(vehicleEntity);
	vehicleCommands.isGrounded = gVehicle->IsGrounded(gPhysicsScene);

	Command command;
	command.throttle = vehicleCommands.throttle;
	
	if (controller.HasComponent<Powerup>(vehicleEntity)) {
		auto& p = controller.GetComponent<Powerup>(vehicleEntity);
		if (p.active && p.type == 1) {
			gVehicle->ApplyBoost(2.0f);
			usedBoost = true;
		}
	}
	else if (usedBoost) {
		gVehicle->ClearBoost();
		usedBoost = false;
	}
	command.brake = vehicleCommands.brake;
	command.steer = vehicleCommands.steer;
	gVehicle->setCommand(command);*/

	for (auto& [entity, vehicle] : vehicles)
	{
		auto& vehicleCommands = controller.GetComponent<VehicleCommands>(entity);
		vehicleCommands.isGrounded = vehicle->IsGrounded(gPhysicsScene);

		Command command;
		command.throttle = vehicleCommands.throttle;

		if (controller.HasComponent<Powerup>(entity)) {
			auto& p = controller.GetComponent<Powerup>(entity);
			if (p.active && p.type == 1) {
				vehicle->ApplyBoost(2.0f);
				usedBoost = true;
			}
		}
		else if (usedBoost) {
			vehicle->ClearBoost();
			usedBoost = false;
		}
		command.brake = vehicleCommands.brake;
		command.steer = vehicleCommands.steer;
		vehicle->setCommand(command);

		if (spinning)
		{
			spinTimer += deltaTime;

			command.throttle = 0.0f;
			command.brake = 0.3f;
			command.steer = sin(spinTimer * 20.0f);

			vehicle->SpinOut();

			if (spinTimer > 1.0f)
			{
				spinning = false;
				spinTimer = 0.0f;
			}
		}


		if (vehicleCommands.inSludge) {
			float drag = vehicleCommands.sludgeFactor;
			float factor = 1.0f - drag * deltaTime;
			factor = PxMax(factor, 0.0f);

			vehicle->ApplySludgeDrag(factor);
		}
	}

	Simulate(deltaTime); // run physics simulation

	DeleteActorsQueue();

	// Update dynamic actor transforms in ECS
	for (auto& entity : entities)
	{
		auto& transform = controller.GetComponent<Transform>(entity);
		
		if (controller.HasComponent<RigidBody>(entity))
		{
			auto& rigidBody = controller.GetComponent<RigidBody>(entity);

			// update the transforms from physics simulation
			PxTransform pxTransform = rigidBody.actor->getGlobalPose();
			transform.position = glm::vec3(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);
			transform.quatRotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);

			// update velocities from physics simulation
			PxVec3 linearVelocity = rigidBody.actor->getLinearVelocity();
			PxVec3 angularVelocity = rigidBody.actor->getAngularVelocity();
			rigidBody.linearVelocity = glm::vec3(linearVelocity.x, linearVelocity.y, linearVelocity.z);
			rigidBody.angularVelocity = glm::vec3(angularVelocity.x, angularVelocity.y, angularVelocity.z);

			if (controller.HasComponent<MovingObstacle>(entity))
			{
				auto& obstacle = controller.GetComponent<MovingObstacle>(entity);

				if (obstacle.paused)
					continue;
				
				// https://stackoverflow.com/questions/1800138/given-a-start-and-end-point-and-a-distance-calculate-a-point-along-a-line
				int currentPathIndex = obstacle.currentPathIndex;
				int previousPathIndex = (currentPathIndex - 1 + obstacle.pathPoints.size()) % obstacle.pathPoints.size();
				
				// calculate the total distance between the two path points
				glm::vec3 totalDistance = obstacle.pathPoints[currentPathIndex] - obstacle.pathPoints[previousPathIndex];

				// calculate rotation value
				glm::quat startRotation = obstacle.pathRotations.empty() ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f) : obstacle.pathRotations[previousPathIndex];
				glm::quat endRotation = obstacle.pathRotations.empty() ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f) : obstacle.pathRotations[currentPathIndex];

				glm::quat newRotation = glm::slerp(startRotation, endRotation, obstacle.progress); // interpolate rotation based on progress

				// advance progress of obstacle
				if (!obstacle.pathTimes.empty()) // if path times are defined, use them to calculate progress
				{
					float segmentDuration = obstacle.pathTimes[currentPathIndex];
					obstacle.progress += deltaTime / segmentDuration; // update progress along the path
				}
				else // otherwise, use speed to calculate progress
				{
					float pathLength = glm::length(totalDistance);
					float distanceToTravel = obstacle.speed * deltaTime; // calculate the distance to travel this frame
					obstacle.progress += distanceToTravel / pathLength; // update progress along the path
				}

				obstacle.progress = glm::clamp(obstacle.progress, 0.0f, 1.0f); // clamp progress between 0 and 1

				// set the new target position for the kinematic actor
				rigidBody.actor->setKinematicTarget(PxTransform(
					PxVec3(
						obstacle.pathPoints[previousPathIndex].x + totalDistance.x * obstacle.progress,
						obstacle.pathPoints[previousPathIndex].y + totalDistance.y * obstacle.progress,
						obstacle.pathPoints[previousPathIndex].z + totalDistance.z * obstacle.progress
					),
					PxQuat(newRotation.x, newRotation.y, newRotation.z, newRotation.w)
				));
				
				// check if the obstacle reached the current target point
				bool segmentComplete = !obstacle.pathTimes.empty() ? obstacle.progress >= 1.0f : glm::length(obstacle.pathPoints[currentPathIndex] - transform.position) < 0.1f;

				if (segmentComplete)
				{
					obstacle.currentPathIndex = (obstacle.currentPathIndex + 1) % obstacle.pathPoints.size(); // increment the path index to the next point
					obstacle.progress = 0.0f; // reset progress for the next segment
				}
			}
		}
		else if (controller.HasComponent<VehicleBody>(entity))
		{
			auto& vehicleBody = controller.GetComponent<VehicleBody>(entity);
			PxTransform pxTransform = vehicles[entity]->getTransform();
			transform.position = glm::vec3(pxTransform.p.x, pxTransform.p.y - 0.3, pxTransform.p.z);
			transform.quatRotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);

			PxVec3 linearVel = vehicles[entity]->getLinearVelocity();
			PxVec3 angularVel = vehicles[entity]->getAngularVelocity();
			vehicleBody.linearVelocity = glm::vec3(linearVel.x, linearVel.y, linearVel.z);
			vehicleBody.angularVelocity = glm::vec3(angularVel.x, angularVel.y, angularVel.z);

			// update wheel transforms
			for (size_t i = 0; i < vehicleBody.wheelEntities.size(); i++)
			{
				Entity wheelEntity = vehicleBody.wheelEntities[i];
				auto& wheelTransform = controller.GetComponent<Transform>(wheelEntity);
				auto [wheelPos, wheelRot] = vehicles[entity]->getWheelTransform(i);

				PxTransform t = vehicles[entity]->getWheelTransform(i);
				wheelTransform.position = glm::vec3(t.p.x, t.p.y, t.p.z);
				wheelTransform.quatRotation = glm::quat(t.q.w, t.q.x, t.q.y, t.q.z);
			}
		}
	}
}

void PhysicsSystem::DeleteActorsQueue()
{
	// delete any actors that were marked for deletion during the simulation step
	for (PxActor* actor : actorsToDelete)
	{
		gPhysicsScene->removeActor(*actor);
		actor->release();
	}
	actorsToDelete.clear();
}

void PhysicsSystem::Simulate(float deltaTime)
{
	//gVehicle->step(deltaTime);

	for (auto& [_, vehicle] : vehicles)
	{
		vehicle->step(deltaTime);
	}
	gPhysicsScene->simulate(deltaTime);
	gPhysicsScene->fetchResults(true);
}

void PhysicsSystem::Cleanup()
{
	/*gVehicle->cleanup();
	gVehicle.reset();*/

	for (auto& [_, vehicle] : vehicles)
	{
		vehicle->cleanup();
		vehicle.reset();
	}

	PxCloseVehicleExtension();
	gPhysicsScene->release();
	gDispatcher->release();
	gPhysics->release();
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();
		transport->release();
	}
	gFoundation->release();
}

void PhysicsSystem::CreateMap()
{
	// get all entities with Transform components
	for (auto& entity : entities)
	{
		auto& transform = controller.GetComponent<Transform>(entity);

		if (controller.HasComponent<StaticBody>(entity))
		{
			auto& entityModel = controller.GetComponent<StaticBody>(entity);
			for (const Mesh& mesh : entityModel.collisionMesh->getMeshes())
			{
				PxTriangleMesh* triangleMesh = CreateTriangleMesh(mesh);
				if (!triangleMesh)
				{
					std::cout << "Failed to create triangle mesh for entity!" << std::endl;
					continue;
				}
				
				PxMeshScale scale = PxMeshScale(PxVec3(transform.scale.x, transform.scale.y, transform.scale.z), PxQuat(PxIdentity)); 
				PxTransform meshTransform = 
					PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z),
					PxQuat(transform.quatRotation.x, transform.quatRotation.y, transform.quatRotation.z, transform.quatRotation.w));
				
				PxShape* shape = gPhysics->createShape(PxTriangleMeshGeometry(triangleMesh, scale), *gGroundMaterial);
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

				PxFilterData filterData(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
				shape->setSimulationFilterData(filterData);

				PxRigidStatic* staticActor = gPhysics->createRigidStatic(meshTransform);
				staticActor->attachShape(*shape);
				staticActor->setActorFlag(PxActorFlag::eVISUALIZATION, false);

				auto& actor = controller.GetComponent<StaticBody>(entity);
				actor.actor = staticActor;

				gPhysicsScene->addActor(*staticActor);
				shape->release();
				triangleMesh->release();
			}
		}
		else if (controller.HasComponent<RigidBody>(entity))
		{
			// create dynamic rigid body
			auto& rigidBodyComponent = controller.GetComponent<RigidBody>(entity);
			PxBounds3 aabb = PxBounds3::empty();

			// check if a bounding volume already exists for this model
			if (rigidBodyComponent.boundingVolume)
			{
				AABB& bv = *rigidBodyComponent.boundingVolume;
				std::array<glm::vec3, 8> vertices = bv.getVertices();
				for (const glm::vec3& vertex : vertices)
				{
					aabb.include(PxVec3(vertex.x, vertex.y, vertex.z));
				}
			} 
			else // create aabb from collision mesh
			{
				for (const Mesh& mesh : rigidBodyComponent.collisionMesh->getMeshes())
				{
				
				}
			}

			PxVec3 extent = aabb.getExtents();
			// scale by the transform scale
			extent.x *= transform.scale.x;
			extent.y *= transform.scale.y;
			extent.z *= transform.scale.z;

			PxBoxGeometry boxGeometry(extent);
			PxTransform bodyTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z),
				PxQuat(transform.quatRotation.x, transform.quatRotation.y, transform.quatRotation.z, transform.quatRotation.w));

			PxShape* shape = gPhysics->createShape(boxGeometry, *gGroundMaterial);

			PxFilterData filterData(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);
			shape->setSimulationFilterData(filterData);

			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

			PxRigidDynamic* dynamicActor = gPhysics->createRigidDynamic(bodyTransform);
			dynamicActor->attachShape(*shape);
			dynamicActor->setActorFlag(PxActorFlag::eVISUALIZATION, false);

			dynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, controller.HasComponent<MovingObstacle>(entity));

			PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidBodyComponent.mass);
			rigidBodyComponent.actor = dynamicActor;
			gPhysicsScene->addActor(*dynamicActor);
			shape->release();
		}
		else if (controller.HasComponent<VehicleBody>(entity))
		{
			/*if (!gVehicle->setup(gPhysicsScene, gPhysics, materialMap["vehicle_tire"]))
			{
				std::cout << "Vehicle failed to initialize!" << std::endl;
				return;
			}*/
			vehicles[entity] = std::make_unique<MainVehicle>();
			Transform initTransform = controller.GetComponent<Transform>(entity);

			PxTransform it = PxTransform(PxVec3(initTransform.position.x, initTransform.position.y, initTransform.position.z),
				PxQuat(initTransform.quatRotation.x, initTransform.quatRotation.y, initTransform.quatRotation.z, initTransform.quatRotation.w));

			if (!vehicles[entity]->setup(gPhysicsScene, gPhysics, materialMap["vehicle_tire"], it))
			 {
				 std::cout << "Vehicle failed to initialize!" << std::endl;
				 return;
			}
			vehicles[entity]->setEntityUserData(entity);
		}
		else if (controller.HasComponent<Trigger>(entity))
		{
			// create trigger volume
			auto& trigger = controller.GetComponent<Trigger>(entity);

			PxBoxGeometry box = PxBoxGeometry(PxVec3(trigger.width, trigger.height, trigger.length));
			PxTransform boxTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z), PxQuat(transform.quatRotation.x, transform.quatRotation.y, transform.quatRotation.z, transform.quatRotation.w));

			PxShape* shape = gPhysics->createShape(box, *materialMap["default"]);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);

			PxFilterData filterData(COLLISION_FLAG_TRIGGER, COLLISION_FLAG_TRIGGER_AGAINST, 0, 0);
			shape->setSimulationFilterData(filterData);
			
			PxRigidStatic* staticActor = gPhysics->createRigidStatic(boxTransform);
			staticActor->attachShape(*shape);
			staticActor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
			staticActor->userData = reinterpret_cast<void*>(entity);

			trigger.actor = staticActor;

			gPhysicsScene->addActor(*staticActor);
			shape->release();
		}
	}
}

PxTriangleMesh* PhysicsSystem::CreateTriangleMesh(const Mesh& mesh)
{
	std::vector<PxVec3> positions;
	positions.reserve(mesh.getVertices().size());
	for (const Vertex& v : mesh.getVertices())
	{
		positions.emplace_back(PxVec3(v.position.x, v.position.y, v.position.z));
	}

	// https://nvidia-omniverse.github.io/PhysX/physx/5.6.1/docs/Geometry.html
	PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = static_cast<PxU32>(positions.size());
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = positions.data();

	meshDesc.triangles.count = static_cast<PxU32>(mesh.getIndices().size() / 3);
	meshDesc.triangles.stride = 3 * sizeof(GLuint);
	meshDesc.triangles.data = mesh.getIndices().data();

	PxTolerancesScale scale = PxTolerancesScale();
	PxCookingParams params(scale);

	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
	params.meshWeldTolerance = 0.001f;

	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
	if (!status)
	{
		std::cout << "Failed to cook triangle mesh" << std::endl;
		return NULL;
	}
	PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());

	return gPhysics->createTriangleMesh(readBuffer);
}

void PhysicsSystem::CreateActorListener(Event& e)
{
	Entity entity = e.GetParam<Entity>(Events::Physics::Create_Actor::ENTITY);
	auto& transform = controller.GetComponent<Transform>(entity);

	if (controller.HasComponent<RigidBody>(entity))
	{
		// create dynamic rigid body
		auto& rigidBodyComponent = controller.GetComponent<RigidBody>(entity);
		PxBounds3 aabb = PxBounds3::empty();

		// check if a bounding volume already exists for this model
		if (rigidBodyComponent.boundingVolume)
		{
			AABB& bv = *rigidBodyComponent.boundingVolume;
			std::array<glm::vec3, 8> vertices = bv.getVertices();
			for (const glm::vec3& vertex : vertices)
			{
				aabb.include(PxVec3(vertex.x, vertex.y, vertex.z));
			}
		}
		else // create aabb from collision mesh
		{
			for (const Mesh& mesh : rigidBodyComponent.collisionMesh->getMeshes())
			{

			}
		}

		PxVec3 extent = aabb.getExtents();
		// scale by the transform scale
		extent.x *= transform.scale.x;
		extent.y *= transform.scale.y;
		extent.z *= transform.scale.z;

		PxBoxGeometry boxGeometry(extent);
		PxTransform bodyTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z),
			PxQuat(transform.quatRotation.x, transform.quatRotation.y, transform.quatRotation.z, transform.quatRotation.w));

		PxShape* shape = gPhysics->createShape(boxGeometry, *gGroundMaterial);

		PxFilterData filterData(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);
		shape->setSimulationFilterData(filterData);

		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);

		PxRigidDynamic* dynamicActor = gPhysics->createRigidDynamic(bodyTransform);
		dynamicActor->attachShape(*shape);
		dynamicActor->setActorFlag(PxActorFlag::eVISUALIZATION, false);

		if (controller.HasComponent<MovingObstacle>(entity))
		{
			dynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
		}

		PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidBodyComponent.mass);
		rigidBodyComponent.actor = dynamicActor;
		gPhysicsScene->addActor(*dynamicActor);
		shape->release();
	}
	else if (controller.HasComponent<Trigger>(entity))
	{
		// create trigger volume
		auto& trigger = controller.GetComponent<Trigger>(entity);

		PxBoxGeometry box = PxBoxGeometry(PxVec3(trigger.width, trigger.height, trigger.length));
		PxTransform boxTransform = PxTransform(PxVec3(transform.position.x, transform.position.y, transform.position.z), PxQuat(transform.quatRotation.x, transform.quatRotation.y, transform.quatRotation.z, transform.quatRotation.w));

		PxShape* shape = gPhysics->createShape(box, *materialMap["default"]);
		shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);

		PxFilterData filterData(COLLISION_FLAG_TRIGGER, COLLISION_FLAG_TRIGGER_AGAINST, 0, 0);
		shape->setSimulationFilterData(filterData);

		PxRigidStatic* staticActor = gPhysics->createRigidStatic(boxTransform);
		staticActor->attachShape(*shape);
		staticActor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
		staticActor->userData = reinterpret_cast<void*>(entity);

		trigger.actor = staticActor;

		gPhysicsScene->addActor(*staticActor);
		shape->release();
	}
}

void PhysicsSystem::JumpEventListener(Event& e)
{
	Entity entity = e.GetParam<Entity>(Events::Player::Player_Jumped::ENTITY);
	
	vehicles[entity]->jump();
}

void PhysicsSystem::ResetVehicleEventListener(Event& e)
{
	Entity entity = e.GetParam<Entity>(Events::Player::Reset_Vehicle::ENTITY);
	
	vehicles[entity]->respawnAtCheckpoint();
}

void PhysicsSystem::CheckpointReachedListener(Event& e)
{
	Entity playerEntity = e.GetParam<Entity>(Events::Checkpoint::Reached::PLAYER_ENTITY);
	Entity checkpointEntity = e.GetParam<Entity>(Events::Checkpoint::Reached::CHECKPOINT_ENTITY);

	auto& transform = controller.GetComponent<Transform>(checkpointEntity);
	auto& checkpoint = controller.GetComponent<CheckPoint>(checkpointEntity);
	glm::vec3 position = transform.position;
	glm::quat rotation = checkpoint.orientation;
	
	vehicles[playerEntity]->setCheckpoint(position, rotation);
}

void PhysicsSystem::ReleaseActorCallback(Entity entity, RigidBody& rb)
{
	if (rb.actor)
	{
		actorsToDelete.push_back(rb.actor);
	}
}

void PhysicsSystem::ReleaseTriggerCallback(Entity entity, Trigger& trig)
{
	if (trig.actor)
	{
		actorsToDelete.push_back(trig.actor);
	}
}

void PhysicsSystem::SpinOutListener(Event& e) {
	Entity entity = e.GetParam<Entity>(Events::Player::Spin_Out::Entity);
	
	spinning = true;
	spinTimer = 0.0f;
}

void PhysicsSystem::BlastEventListener(Event& e) {
	Entity source = e.GetParam<Entity>(Events::Player::Blast::ENTITY);

	float radius = 40.0f;
	float strength = 20000.0f;

	glm::vec3 origin = controller.GetComponent<Transform>(source).position;

	for (auto& [entity, vehicle] : vehicles) {
		if (entity == source)
			continue;

		if (!controller.HasComponent<Transform>(entity))
			continue;

		glm::vec3 targetPos = controller.GetComponent<Transform>(entity).position;

		glm::vec3 dir = targetPos - origin;
		float dist = glm::length(dir);

		if (dist > radius || dist <= 0.001f)
			continue;

		float falloff = 1.0f - (dist / radius);

		falloff = falloff * falloff;

		float force = strength * falloff;
		
		PxVec3 impulse = PxVec3(dir.x, dir.y, dir.z) * force;
		impulse.y += 0.5f * force;
		std::cout << "BLASTED\n";
		vehicle->ApplyImpulse(impulse);
	}
	}