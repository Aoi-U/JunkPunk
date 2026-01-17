#pragma once

#include <iostream>

#include "PxPhysicsAPI.h"
#include "base/Base.h"
#include "directdrivetrain/DirectDrivetrain.h"

using namespace physx;
using namespace physx::vehicle2;
using namespace snippetvehicle;


class VehicleParamHelper
{
public:
	VehicleParamHelper() = default;

	void setBaseParams(BaseVehicleParams& baseParams);

	void setDirectDriveParams(DirectDrivetrainParams& directDriveParams);

	// physx integration params
	void setPhysXIntegrationParams(const PxVehicleAxleDescription& axleDescription,
		PxVehiclePhysXMaterialFriction* physXMaterialFrictions, PxU32 nbPhysXMaterialFrictions,
		PxReal physXDefaultMaterialFriction, PhysXIntegrationParams& physXParams);

	const PxQueryFilterData queryFilterData{ PxFilterData(0, 0, 0, 0), PxQueryFlag::eSTATIC };
	PxQueryFilterCallback* queryFilterCallback = NULL;
	const PxTransform physxActorCMassLocalPose{PxVec3(0.0f, 0.5f, 0.0f), PxQuat(PxIdentity)};
	const PxVec3 physxActorBoxShapeHalfExtents{ 0.44097f, 0.2f, 0.8f };
	const PxTransform physxActorBoxShapeLocalPose{ PxVec3(0.0f, 0.5f, 0.0f), PxQuat(PxIdentity) };

private:
	BaseVehicleParams* bp;

	// high level params

	void setAxleDescription();

	void setFrame();

	void setScale();

	// rigid body params

	void setRigidBodyParams();

	// suspension state params

	void setSuspensionStateCalculationParams();

	// command responses

	void setBrakeResponseParams();

	void setHandbrakeResponseParams();

	void setSteerResponseParams();

	void setAckermannParams();

	// suspension params

	void setSuspensionParams();

	void setSuspensionComplianceParams();

	void setSuspensionForceParams();

	// tire params

	void setTireForceParams();

	// wheel params

	void setWheelParams();

	
	// direct drivetrain params
	void setThrottleResponseParams(DirectDrivetrainParams& directDrivetrainParams);
};