/*
	Font Class based upon: http://www.rastertek.com/dx11tut13.html
*/

#ifndef _Sentence_H_
#define _Sentence_H_

#include"RawVerts.h"
//#include <d3d11.h>
//#include <d3dx10math.h>
#include <fstream>
#include <string>
using namespace std;

class Sentence : public RawVerts
{
private:

	struct FontType
	{
		float left, right;
		int size;
	};

public:
	Sentence();
	virtual ~Sentence( );
    void Render(ID3D11DeviceContext*, void*);
    void Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data);

	void InitializeSentence(ID3D11Device*,int,int,int);
	void UpdateSentence(string* text, int positionX, int positionY, 
		float red, float green, float blue);
	void LoadFontData(const char*);
	void ReleaseFontData();

private:
	void BuildVertexArray(void*,string*, float, float);
	FontType* m_Font;
	float red, green, blue;
	string txt;
	int posx, posy;
	bool toBeUpdated;
	int maxLength;
	int m_screenWidth, m_screenHeight;
};

#endif