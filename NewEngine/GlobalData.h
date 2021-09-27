#ifndef GLOBALDATA_H
#define GLOBALDATA_H

//#define SCREEN_WIDTH 1980
//#define SCREEN_HEIGHT 1020

const unsigned int SCREEN_WIDTH = 1280;//1980;
const unsigned int SCREEN_HEIGHT = 720;//1020;

// Design from http://stackoverflow.com/questions/2483978/c-best-way-to-implement-globally-scoped-data
#include<unordered_map>
#include<d3d11.h>

namespace GlobalData
{
	extern std::unordered_map<char,float>* gbMap;
	extern ID3D11RasterizerState* rasterizerState;
	//extern std::unordered_map<char,int>* gbMapString;
}

#endif GLOBALDATA_H