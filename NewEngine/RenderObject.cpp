#include"RenderObject.h"

RenderObject::RenderObject()
{
	loaded = false;
}

RenderObject::~RenderObject( )
{
	delete[] effectName;
}
void RenderObject::passInShader(Shader* s)
{
	shaders = s;
	loaded = true;
}

void RenderObject::setShaderTechniqueName(char* c)
{
	effectName = c;
}