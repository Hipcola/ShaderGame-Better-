/*
    Beginning DirectX 11 Game Programming
    By Allen Sherrod and Wendy Jones

    FPSCamera - Demonstrates a camera with arc rotation.
*/


//#include<d3d11.h>
#include"FPSCamera.h"
#include"utility.h"

FPSCamera::FPSCamera() : DefaultForward( XMVectorSet(0.0f,0.0f,1.0f, 0.0f) ), DefaultRight( XMVectorSet(1.0f,0.0f,0.0f, 0.0f) ), 
	camForward(XMVectorSet(0.0f,0.0f,1.0f, 0.0f)), camRight( XMVectorSet(1.0f,0.0f,0.0f, 0.0f)), camUp(XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f )), 
	moveLeftRight (0.0f), moveBackForward (0.0f), xRotation_(0.0f), yRotation_(0.0f)
{
													//800.0f / 600.0f
	projMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV4, 800.0f / 600.0f, 0.01f, 10000.0f );
	camPosition = XMVectorSet(0.0f,0.0f,0.0f, 0.0f);
	xRotation_ = 0.0f;
    yRotation_ = 0.0f;
	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	//camPosition += moveLeftRight*camRight;
	//camPosition += moveBackForward*camForward;
	//OutputDebugString(utility::convertFloat().c_str());
	//OutputDebugString("\n");
	camRotationMatrix = XMMatrixRotationRollPitchYaw( xRotation_, yRotation_, 0.0f );
}

FPSCamera::FPSCamera(unsigned int screenWidth, unsigned int screenHeight  ) : DefaultForward( XMVectorSet(0.0f,0.0f,1.0f, 0.0f) ), DefaultRight( XMVectorSet(1.0f,0.0f,0.0f, 0.0f) ), 
	camForward(XMVectorSet(0.0f,0.0f,1.0f, 0.0f)), camRight( XMVectorSet(1.0f,0.0f,0.0f, 0.0f)), camUp(XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f )), 
	moveLeftRight (0.0f), moveBackForward (0.0f), xRotation_(0.0f), yRotation_(0.0f)
{
													//800.0f / 600.0f
	projMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV4, screenWidth / screenHeight, 0.01f, 10000.0f );
	camPosition = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
	xRotation_ = 0.0f;
    yRotation_ = 0.0f;
	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
	//camPosition += moveLeftRight*camRight;
	//camPosition += moveBackForward*camForward;
	//OutputDebugString(utility::convertFloat().c_str());
	//OutputDebugString("\n");
	camRotationMatrix = XMMatrixRotationRollPitchYaw( xRotation_, yRotation_, 0.0f );
}

void FPSCamera::SetRotation( float x, float y, float minY, float maxY )
{
    xRotation_ = x;
    yRotation_ = y;
    yMin_ = minY;
    yMax_ = maxY;

    if( yRotation_ < yMin_ ) yRotation_ = yMin_;
    if( yRotation_ > yMax_ ) yRotation_ = yMax_;
}

void FPSCamera::SetPosition( XMVECTOR& position )
{
    camPosition = position;
}

void FPSCamera::ApplyRotation( float pitchDelta, float yawDelta )
{
    xRotation_ += pitchDelta;
    yRotation_ += yawDelta;

	camRotationMatrix = XMMatrixRotationRollPitchYaw( xRotation_, yRotation_, 0.0f );

}

void FPSCamera::ApplyPosition( float fb, float lr )
{
    moveLeftRight += lr;
    moveBackForward += fb;

	camPosition += moveLeftRight*camRight;
	camPosition += moveBackForward*camForward;

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;
}


XMMATRIX FPSCamera::GetViewMatrix( )
{
	XMMATRIX camView;
	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix );
	camTarget = XMVector3Normalize(camTarget);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(xRotation_);

	camRight = XMVector3TransformCoord(DefaultRight, camRotationMatrix);
	camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, camRotationMatrix);

    camTarget = camPosition + camTarget;	

    camView = XMMatrixLookAtLH( camPosition, camTarget, camUp );

    return camView;
}

XMMATRIX FPSCamera::GetProjMatrix( )
{
	return projMatrix;
}

XMFLOAT4 FPSCamera::GetPosition( )
{
	XMFLOAT4 returnVal;
	XMStoreFloat4(&returnVal, camPosition);
    return returnVal;
}


