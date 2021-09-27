#ifndef _FPS_CAM_H_
#define _FPS_CAM_H_
//#include"ArcCamera.h"

#include<DirectXMath.h>
using namespace DirectX;

class FPSCamera
{
    public:
		FPSCamera();
        FPSCamera(unsigned int screenWidth, unsigned int screenHeight );

        void SetRotation( float x, float y, float minY, float maxY );
		void SetPosition( XMVECTOR& position );
        void ApplyRotation( float yawDelta, float pitchDelta );
		void ApplyPosition( float forwardBack, float leftRight );

        XMMATRIX GetViewMatrix( );
		XMMATRIX GetProjMatrix( );
		XMFLOAT4 GetPosition( );

    private:
        XMVECTOR DefaultForward;
		XMVECTOR DefaultRight;
		XMVECTOR camForward;
		XMVECTOR camRight;
		
		XMVECTOR camPosition;
		XMVECTOR camTarget;
		XMVECTOR camUp;

		XMMATRIX camRotationMatrix;
		XMMATRIX groundWorld;

		float moveLeftRight;
		float moveBackForward;
		XMMATRIX projMatrix;

        float xRotation_, yRotation_, yMin_, yMax_;
};


#endif