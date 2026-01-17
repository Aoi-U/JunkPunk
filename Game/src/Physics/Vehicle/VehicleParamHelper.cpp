#include "VehicleParamHelper.h"
#include <array>


void VehicleParamHelper::setBaseParams(BaseVehicleParams& baseParams)
{
	bp = &baseParams;

	setAxleDescription();
	setFrame();
	setScale();
	setRigidBodyParams();
	setSuspensionStateCalculationParams();
	setBrakeResponseParams();
	setHandbrakeResponseParams();
	setSteerResponseParams();
	setAckermannParams();
	setSuspensionParams();
	setSuspensionComplianceParams();
	setSuspensionForceParams();
	setTireForceParams();
	setWheelParams();
}

void VehicleParamHelper::setDirectDriveParams(DirectDrivetrainParams& directDriveParams)
{
	setThrottleResponseParams(directDriveParams);
}

void VehicleParamHelper::setAxleDescription()
{
	bp->axleDescription.setToDefault();
	
	const PxU32 frontWheels[] = { 0, 1 };
	bp->axleDescription.addAxle(2, frontWheels);
	const PxU32 rearWheels[] = { 2, 3 };
	bp->axleDescription.addAxle(2, rearWheels);
}

void VehicleParamHelper::setFrame()
{
	//bp->frame.setToDefault();
	const PxU32 LngAxis = 4; // PosZ
	const PxU32 LatAxis = 0; // PosX
	const PxU32 VrtAxis = 2; // PosY

	bp->frame.lngAxis = static_cast<PxVehicleAxes::Enum>(LngAxis);
	bp->frame.latAxis = static_cast<PxVehicleAxes::Enum>(LatAxis);
	bp->frame.vrtAxis = static_cast<PxVehicleAxes::Enum>(VrtAxis);
}

void VehicleParamHelper::setScale()
{
	const PxReal scale = 1.0f;
	bp->scale.scale = scale;
}

void VehicleParamHelper::setRigidBodyParams()
{
	const PxReal mass = 2014.4000244140625f;
	const PxVec3 moi = PxVec3(3200, 3414, 750);
	bp->rigidBodyParams.mass = mass;
	bp->rigidBodyParams.moi = moi;
}

void VehicleParamHelper::setSuspensionStateCalculationParams()
{
	const PxU32 JounceCalculationType = 1;
	const bool LimitSuspensionExpansionVelocity = false;

	bp->suspensionStateCalculationParams.suspensionJounceCalculationType = static_cast<PxVehicleSuspensionJounceCalculationType::Enum>(JounceCalculationType);
	bp->suspensionStateCalculationParams.limitSuspensionExpansionVelocity = LimitSuspensionExpansionVelocity;
}

void VehicleParamHelper::setBrakeResponseParams()
{
	const PxU32 MaxResponse = 1500.0f;
	std::array<PxReal, 4> WheelResponseMultiplier = { 0.0f, 0.0f, 1.0f, 1.0f };

	bp->brakeResponseParams[0].maxResponse = MaxResponse;
	for (PxU32 i = 0; i < WheelResponseMultiplier.size(); i++)
	{
		bp->brakeResponseParams[0].wheelResponseMultipliers[i] = WheelResponseMultiplier[i];
	}
}

void VehicleParamHelper::setHandbrakeResponseParams()
{
	const PxU32 MaxResponse = 0.0f;
	std::array<PxReal, 4> WheelResponseMultiplier = { 0.0f, 0.0f, 1.0f, 1.0f };
	bp->brakeResponseParams[1].maxResponse = MaxResponse;
	for (PxU32 i = 0; i < WheelResponseMultiplier.size(); i++)
	{
		bp->brakeResponseParams[1].wheelResponseMultipliers[i] = WheelResponseMultiplier[i];
	}
}

void VehicleParamHelper::setSteerResponseParams()
{
	const PxU32 MaxResponse = 1.0f;
	std::array<PxU32, 4> WheelResponseMultiplier = { 1, 1, 0, 0 };
	bp->steerResponseParams.maxResponse = MaxResponse;
	for (PxU32 i = 0; i < WheelResponseMultiplier.size(); i++)
	{
		bp->steerResponseParams.wheelResponseMultipliers[i] = WheelResponseMultiplier[i];
	}
}

void VehicleParamHelper::setAckermannParams()
{
	const PxReal WheelBase = 2.863219976425171f;
	const PxReal TrackWidth = 1.5510799884796143f;
	const PxReal Strength = 1.0f;

	bp->ackermannParams[0].wheelBase = WheelBase;
	bp->ackermannParams[0].trackWidth = TrackWidth;
	bp->ackermannParams[0].strength = Strength;

	const PxReal WheelIds[2] = { 0, 1 };
	for (PxU32 i = 0; i < 2; i++)
	{
		bp->ackermannParams[0].wheelIds[i] = WheelIds[i];
	}
}

void setSuspensionHelper(PxVehicleSuspensionParams& sp, PxVec3 SuspensionAttachment_Pos, PxQuat SuspensionAttachment_Quat, PxVec3 SuspensionTravelDir, PxReal SuspensionTravelDist, PxVec3 WheelAttachment_Pos, PxQuat WheelAttachment_Quat)
{
	sp.suspensionAttachment.p = SuspensionAttachment_Pos;
	sp.suspensionAttachment.q = SuspensionAttachment_Quat;
	sp.suspensionTravelDir = SuspensionTravelDir;
	sp.suspensionTravelDist = SuspensionTravelDist;
	sp.wheelAttachment.p = WheelAttachment_Pos;
	sp.wheelAttachment.q = WheelAttachment_Quat;
}
void VehicleParamHelper::setSuspensionParams()
{
	PxVec3 SuspensionAttachment_Pos = { -0.7952629923820496, -0.10795199871063233, 1.269219994544983 };
	PxQuat SuspensionAttachment_Quat = { 0.0, 0.0, 0.0, 1.0 };
	PxVec3 SuspensionTravelDir = { 0.0, -1.0,  0.0 };
	PxReal SuspensionTravelDist = { 0.221110999584198 };
	PxVec3 WheelAttachment_Pos = { 0.0, 0.0, 0.0 };
	PxQuat WheelAttachment_Quat = { 0.0, 0.0, 0.0, 1.0 };
	setSuspensionHelper(bp->suspensionParams[0], SuspensionAttachment_Pos, SuspensionAttachment_Quat, SuspensionTravelDir, SuspensionTravelDist, WheelAttachment_Pos, WheelAttachment_Quat);

	SuspensionAttachment_Pos[0] = 0.7952629923820496; SuspensionAttachment_Pos[1] = -0.10795000195503235; SuspensionAttachment_Pos[2] = 1.269219994544983;
	setSuspensionHelper(bp->suspensionParams[1], SuspensionAttachment_Pos, SuspensionAttachment_Quat, SuspensionTravelDir, SuspensionTravelDist, WheelAttachment_Pos, WheelAttachment_Quat);

	SuspensionAttachment_Pos[0] = -0.7952629923820496; SuspensionAttachment_Pos[1] = -0.10795199871063233; SuspensionAttachment_Pos[2] = -1.593999981880188;
	setSuspensionHelper(bp->suspensionParams[2], SuspensionAttachment_Pos, SuspensionAttachment_Quat, SuspensionTravelDir, SuspensionTravelDist, WheelAttachment_Pos, WheelAttachment_Quat);

	SuspensionAttachment_Pos[0] = 0.7952629923820496; SuspensionAttachment_Pos[1] = -0.10795299708843231; SuspensionAttachment_Pos[2] = -1.593999981880188;
	setSuspensionHelper(bp->suspensionParams[3], SuspensionAttachment_Pos, SuspensionAttachment_Quat, SuspensionTravelDir, SuspensionTravelDist, WheelAttachment_Pos, WheelAttachment_Quat);
}


void setSuspensionComplianceParamsHelper(PxVehicleSuspensionComplianceParams& scp, PxPair<PxReal, PxReal> WheelToeAngle, PxPair<PxReal, PxReal> WheelCamberAngle, PxPair<PxReal, PxVec3> SuspForceAppPoint, PxPair<PxReal, PxVec3> TireForceAppPoint)
{
	scp.wheelToeAngle.addPair(WheelToeAngle.first, WheelToeAngle.second);
	scp.wheelCamberAngle.addPair(WheelCamberAngle.first, WheelCamberAngle.second);
	scp.suspForceAppPoint.addPair(SuspForceAppPoint.first, SuspForceAppPoint.second);
	scp.tireForceAppPoint.addPair(TireForceAppPoint.first, TireForceAppPoint.second);
}

void VehicleParamHelper::setSuspensionComplianceParams()
{
	PxPair<PxReal, PxReal> WheelToeAngle = { 0.0f, 0.0f };
	PxPair<PxReal, PxReal> WheelCamberAngle = { 0.0f, 0.0f };
	PxPair<PxReal, PxVec3> SuspForceAppPoint = { 0.0f, PxVec3(0.0, 0.0, -0.11204999685287476) };
	PxPair<PxReal, PxVec3> TireForceAppPoint = { 0.0f, PxVec3(0.0, 0.0, -0.11204999685287476) };

	for(PxU32 i = 0; i < 4; i++)
	{
		setSuspensionComplianceParamsHelper(bp->suspensionComplianceParams[i], WheelToeAngle, WheelCamberAngle, SuspForceAppPoint, TireForceAppPoint);
	}
}

void VehicleParamHelper::setSuspensionForceParams()
{
	PxReal Damping = 8528.1201171875;
	PxReal Stiffness = 32833.30078125;
	PxReal SprungMass = 553.7739868164063;
	bp->suspensionForceParams[0].damping = Damping;
	bp->suspensionForceParams[0].stiffness = Stiffness;
	bp->suspensionForceParams[0].sprungMass = SprungMass;

	Damping = 8742.1904296875f;
	Stiffness = 33657.3984375f;
	SprungMass = 567.6749877929688f;
	bp->suspensionForceParams[1].damping = Damping;
	bp->suspensionForceParams[1].stiffness = Stiffness;
	bp->suspensionForceParams[1].sprungMass = SprungMass;

	Damping = 6765.97021484375f;
	Stiffness = 26049.0f;
	SprungMass = 439.3489990234375f;
	bp->suspensionForceParams[2].damping = Damping;
	bp->suspensionForceParams[2].stiffness = Stiffness;
	bp->suspensionForceParams[2].sprungMass = SprungMass;

	Damping = 6985.47998046875f;
	Stiffness = 26894.099609375f;
	SprungMass = 453.6029968261719f;
	bp->suspensionForceParams[3].damping = Damping;
	bp->suspensionForceParams[3].stiffness = Stiffness;
	bp->suspensionForceParams[3].sprungMass = SprungMass;
}

void setTireForceParamsHelper(PxVehicleTireForceParams& tfp,
	PxReal LongitudinalStiffness,
	PxReal LateralStiffnessX,
	PxReal LateraiStiffnessY,
	PxReal CamberStiffness,
	PxReal RestLoad,
	PxReal FrictionVsSlip[3][2],
	PxReal TireLoadFilter[2][2]
)
{
	tfp.longStiff = LongitudinalStiffness;
	tfp.latStiffX = LateralStiffnessX;
	tfp.latStiffY = LateraiStiffnessY;
	tfp.camberStiff = CamberStiffness;
	tfp.restLoad = RestLoad;
	for (PxU32 i = 0; i < 3; i++)
	{
		tfp.frictionVsSlip[i][0] = FrictionVsSlip[i][0];
		tfp.frictionVsSlip[i][1] = FrictionVsSlip[i][1];
	}

	for (PxU32 i = 0; i < 2; i++)
	{
		tfp.loadFilter[i][0] = TireLoadFilter[i][0];
		tfp.loadFilter[i][1] = TireLoadFilter[i][1];
	}
}

void VehicleParamHelper::setTireForceParams()
{
	PxReal LongitudinalStiffness = 24525.0;
	PxReal LateralStiffnessX = 0.009999999776482582;
	PxReal LateraiStiffnessY = 118699.637252138;
	PxReal CamberStiffness = 0.0;
	PxReal RestLoad = 5628.72314453125;
	PxReal FrictionVsSlip[3][2] = { { 0.0, 1.0 }, { 0.10000000149011612, 1.0 }, { 1.0, 1.0 } };
	PxReal TireLoadFilter[2][2] = { { 0.0, 0.23080000281333924 }, { 3.0, 3.0 } };
	setTireForceParamsHelper(bp->tireForceParams[0], LongitudinalStiffness, LateralStiffnessX, LateraiStiffnessY, CamberStiffness, RestLoad, FrictionVsSlip, TireLoadFilter);
	setTireForceParamsHelper(bp->tireForceParams[1], LongitudinalStiffness, LateralStiffnessX, LateraiStiffnessY, CamberStiffness, RestLoad, FrictionVsSlip, TireLoadFilter);

	LateraiStiffnessY = 143930.84033118;
	RestLoad = 4604.3134765625;
	setTireForceParamsHelper(bp->tireForceParams[2], LongitudinalStiffness, LateralStiffnessX, LateraiStiffnessY, CamberStiffness, RestLoad, FrictionVsSlip, TireLoadFilter);
	setTireForceParamsHelper(bp->tireForceParams[3], LongitudinalStiffness, LateralStiffnessX, LateraiStiffnessY, CamberStiffness, RestLoad, FrictionVsSlip, TireLoadFilter);
}

void VehicleParamHelper::setWheelParams()
{
	PxReal HalfWidth = 0.15768450498580934;
	PxReal Radius = 0.3432520031929016;
	PxReal Mass = 20.0f;
	PxReal MOI = 1.1716899871826172;
	PxReal DampingRate = 0.25;

	for (PxU32 i = 0; i < 4; i++)
	{
		bp->wheelParams[i].halfWidth = HalfWidth;
		bp->wheelParams[i].radius = Radius;
		bp->wheelParams[i].mass = Mass;
		bp->wheelParams[i].moi = MOI;
		bp->wheelParams[i].dampingRate = DampingRate;
	}
}

void VehicleParamHelper::setThrottleResponseParams(DirectDrivetrainParams& directDrivetrainParams)
{
	PxU32 MaxResponse = 750;
	std::array<PxU32, 4> WheelResponseMultiplier = { 1, 1, 1, 1 };
	
	for (PxU32 i = 0; i < WheelResponseMultiplier.size(); i++)
	{
		directDrivetrainParams.directDriveThrottleResponseParams.wheelResponseMultipliers[i] = WheelResponseMultiplier[i];
	}

	directDrivetrainParams.directDriveThrottleResponseParams.maxResponse = MaxResponse;
}

void VehicleParamHelper::setPhysXIntegrationParams(const PxVehicleAxleDescription& axleDescription,
	PxVehiclePhysXMaterialFriction* physXMaterialFrictions, PxU32 nbPhysXMaterialFrictions,
	PxReal physXDefaultMaterialFriction, PhysXIntegrationParams& physXParams)
{
	//The physx integration params are hardcoded rather than loaded from file.
	const PxQueryFilterData queryFilterData(PxFilterData(0, 0, 0, 0), PxQueryFlag::eSTATIC);
	PxQueryFilterCallback* queryFilterCallback = NULL;
	const PxTransform physxActorCMassLocalPose(PxVec3(0.0f, 0.55f, 1.594f), PxQuat(PxIdentity));
	const PxVec3 physxActorBoxShapeHalfExtents(0.84097f, 0.65458f, 2.46971f);
	const PxTransform physxActorBoxShapeLocalPose(PxVec3(0.0f, 0.830066f, 1.37003f), PxQuat(PxIdentity));

	physXParams.create(
		axleDescription,
		queryFilterData, queryFilterCallback,
		physXMaterialFrictions, nbPhysXMaterialFrictions, physXDefaultMaterialFriction,
		physxActorCMassLocalPose,
		physxActorBoxShapeHalfExtents, physxActorBoxShapeLocalPose
	);
}
