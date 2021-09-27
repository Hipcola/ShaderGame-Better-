//#define SCREEN_WIDTH 1980
//#define SCREEN_HEIGHT 1020

//extern const unsigned int SCREEN_WIDTH = 1980;
//extern const unsigned int SCREEN_HEIGHT = 1020;

#include<Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include<memory>
//#include<iostream>
#include<stdio.h>
#include"GlobalData.h"
#include"utility.h"
//#include"ShaderGameV2.h"
#include"LSSpeedTesting.h"

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE prevInstance,
                   LPWSTR cmdLine, int cmdShow )
{	
	int m_fps, m_count;
	m_count = 0;
	unsigned long m_startTime = timeGetTime();
	unsigned long m_lastTime = timeGetTime();

    UNREFERENCED_PARAMETER( prevInstance );
    UNREFERENCED_PARAMETER( cmdLine );

    WNDCLASSEX wndClass = { 0 };
    wndClass.cbSize = sizeof( WNDCLASSEX ) ;
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
    wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;//( HBRUSH )( COLOR_WINDOW + 1 );
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = "MATTCDEMO";

    if( !RegisterClassEx( &wndClass ) )
        return -1;
					//640  480
					//800  600
    RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

 /*   HWND hwnd = CreateWindowA( "DX11BookWindowClass", "Blank Win32 Window",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
        rc.bottom - rc.top, NULL, NULL, hInstance, NULL ); */

	HWND hwnd = CreateWindowA( "MATTCDEMO", "GPU Rewriting", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
                                NULL, NULL, hInstance, NULL );

    if( !hwnd )
        return -1;

    ShowWindow( hwnd, cmdShow );

	//ShaderGameV2 demo;
	LSSpeedTesting demo;
    // Demo Initialize
    //bool result = demo.Initialize( hInstance, hwnd );
	bool result = demo.Run();
    if( result == false )
        return -1;

    MSG msg = { 0 };

  //  while( msg.message != WM_QUIT )
  //  {
  //      if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
  //      {
  //          TranslateMessage( &msg );
  //          DispatchMessage( &msg );
  //      }

  //      // Update and Draw

		////Check sufficient time has passed since last call
		////Change the value to change the fps limit
		////1   = 1000fps
		////10  = 100fps
		////100 = 10 fps
		////Cannot go faster than 1000fps. Strange behaviour at this level.
		////Delta time can become lost.
		//if (timeGetTime()  >= (m_lastTime/*+10*/))
		//{
		//	float dt = (float)(timeGetTime() - m_lastTime);
		//	dt = dt/1000;
		//	m_lastTime = timeGetTime();
		//	//OutputDebugString(utility::convertFloat(dt).c_str());
		//	//OutputDebugString("\n");
		//	demo.Update(dt);
		//	demo.Render( );
		//	m_count++;
		//}
		///*else{OutputDebugString("nope");
		//OutputDebugString("\n");}*/

		////Read fps
		//if(timeGetTime() >= (m_startTime + 1000))
		//{
		//	m_fps = m_count;
		//	m_count = 0;
		//	//OutputDebugString(utility::convertFloat(m_fps).c_str());
		//	//OutputDebugString("\n");
		//	m_startTime = timeGetTime();
		//}
  //  }

    // Demo Shutdown
    //demo.Shutdown( );

    return static_cast<int>( msg.wParam );
}

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT paintStruct;
    HDC hDC;

    switch( message )
    {
        case WM_PAINT:
            hDC = BeginPaint( hwnd, &paintStruct );
            EndPaint( hwnd, &paintStruct );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hwnd, message, wParam, lParam );
    }

    return 0;
}