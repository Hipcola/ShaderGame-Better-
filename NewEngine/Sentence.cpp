#include"Sentence.h"
#include"Text.h"
#include<fstream>

Sentence::Sentence()
{
};

Sentence::~Sentence( )
{
}


void Sentence::Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data)
{
}

void Sentence::InitializeSentence(ID3D11Device* device, int maxLength, int scrHeight, int scrWidth)
{
	D3D11_SUBRESOURCE_DATA resourceData;

	this->m_screenHeight = scrHeight;
	this->m_screenWidth = scrWidth;

	// Set the maximum length of the sentence.
	this->maxLength = maxLength;

	// Set the number of vertices in the vertex array.
	this->vertexCount[0] = 6 * maxLength;

	// Set the number of indexes in the index array.
	this->indicesCount = vertexCount;

	VertexPosRVTex* vertexPtr = new VertexPosRVTex[vertexCount[0]];
	unsigned long* indices = new unsigned long[indicesCount[0]];

	// Set up the description of the dynamic vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexPosRVTex) * vertexCount[0];
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Initialize vertex array to zeros at first.
	memset(vertexPtr, 0, (sizeof(VertexPosRVTex) * vertexCount[0]));

	// Give the subresource structure a pointer to the vertex data.
    resourceData.pSysMem = vertexPtr;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
    device->CreateBuffer(&vertexBufferDesc, &resourceData, &vertexBuffers[0]);

	// Initialize the index array.
	for(int i=0; i<indicesCount[0]; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * indicesCount[0];
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    resourceData.pSysMem = indices;
	resourceData.SysMemPitch = 0;
	resourceData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &resourceData, &indexBuffers[0]);

	// Release the index array as it is no longer needed.
	delete [] indices;
	indices = 0;
}

void Sentence::LoadFontData(const char* filename)
{
	ifstream fin;
	int i;
	char temp;

	// Create the font spacing buffer.
	m_Font = new FontType[95];
	if(!m_Font)
	{
		OutputDebugStringA( "fuck knows" );
	}

	// Read in the font size and spacing between chars.
	fin.open(filename);
	if(fin.fail())
	{
		OutputDebugStringA( "font file failed to load" );
	}

	// Read in the 95 used ascii characters for text.
	for(i=0; i<95; i++)
	{
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}
		fin.get(temp);
		while(temp != ' ')
		{
			fin.get(temp);
		}

		fin >> m_Font[i].left;
		fin >> m_Font[i].right;
		fin >> m_Font[i].size;
	}

	// Close the file.
	fin.close();
}


void Sentence::ReleaseFontData()
{
	// Release the font data array.
	if(m_Font)
	{
		delete [] m_Font;
		m_Font = 0;
	}
}

void Sentence::UpdateSentence(string* text, int positionX, int positionY, float red, float green, float blue)
{
	// Store the color of the sentence.
	if (this->red != red && this->green != green && this->blue != blue)
	{
		this->red = red;
		this->green = green;
		this->blue = blue;	
		toBeUpdated = true;
	}
	
	if (txt.compare(*text) != 0){
		txt = string(*text);
		toBeUpdated = true;
	}

	if (posx != positionX && posy != positionY)
	{
		posx = positionX;
		posy = positionY;
		toBeUpdated = true;
	}
}
void Sentence::BuildVertexArray(void* vertices, string* sentence, float drawX, float drawY)
{
	int numLetters, index, i, letter;
	VertexPosRVTex* vertexPtr;
	// Get the number of letters in the sentence.
	//numLetters = (int)strlen(sentence);
	numLetters = sentence->size();
	// Coerce the input vertices into a VertexType structure.
	vertexPtr = (VertexPosRVTex*)vertices;

	// Initialize the index to the vertex array.
	index = 0;

	// Draw each letter onto a quad.
	for(i=0; i<numLetters; i++)
	{
		letter = ((int)sentence->at(i)) - 32;

		// If the letter is a space then just move over three pixels.
		if(letter == 0)
		{
			drawX = drawX + 3.0f;
		}
		else
		{
			// First triangle in quad.
			vertexPtr[index].pos = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].tex0 = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].pos = XMFLOAT3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].tex0 = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			vertexPtr[index].pos = XMFLOAT3(drawX, (drawY - 16), 0.0f);  // Bottom left.
			vertexPtr[index].tex0 = XMFLOAT2(m_Font[letter].left, 1.0f);
			index++;

			// Second triangle in quad.
			vertexPtr[index].pos = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].tex0 = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].pos = XMFLOAT3(drawX + m_Font[letter].size, drawY, 0.0f);  // Top right.
			vertexPtr[index].tex0 = XMFLOAT2(m_Font[letter].right, 0.0f);
			index++;

			vertexPtr[index].pos = XMFLOAT3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].tex0 = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			// Update the x location for drawing by the size of the letter and one pixel.
			drawX = drawX + m_Font[letter].size + 1.0f;
		}
	}
}

void Sentence::Render(ID3D11DeviceContext* d3dContext, void* data)
{
	if(toBeUpdated)
	{
		// Get the number of letters in the sentence.
		int numLetters;
		//numLetters = (int)strlen(txt);
		numLetters = txt.size();

		VertexPosRVTex* verticesPtr, *vertices;
		vertices = new VertexPosRVTex[vertexCount[0]];

		// Initialize vertex array to zeros at first.
		memset(vertices, 0, (sizeof(VertexPosRVTex) * vertexCount[0]));

		// Calculate the X and Y pixel position on the screen to start drawing to.
		float drawX, drawY;
		drawX = (float)(((m_screenWidth / 2) * -1) + posx);  //-1
		drawY = (float)((m_screenHeight / 2) - posy);       //- pos

		// Use the font class to build the vertex array from the sentence text and sentence draw location.
		BuildVertexArray((void*)vertices,&txt, drawX, drawY);

		// Lock the vertex buffer so it can be written to.
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		d3dContext->Map(vertexBuffers[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		// Get a pointer to the data in the vertex buffer.
		verticesPtr = (VertexPosRVTex*)mappedResource.pData;

		// Copy the data into the vertex buffer.
		memcpy(verticesPtr, (void*)vertices, (sizeof(VertexPosRVTex) * vertexCount[0]));

		// Unlock the vertex buffer.
		d3dContext->Unmap(vertexBuffers[0], 0);

		delete [] vertices;
		vertices = 0;
		toBeUpdated = false;
	}
	ShaderDataText d;
	d.colour = &XMFLOAT4(red,green,blue,0.f);
	d.indCount = &indicesCount[0];

	unsigned int stride, offset;
	bool result;


	// Set vertex buffer stride and offset.
    stride = sizeof(VertexPosRVTex); 
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	d3dContext->IASetVertexBuffers(0, 1, &vertexBuffers[0], &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	d3dContext->IASetIndexBuffer(indexBuffers[0], DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	((Text*)&shaders[0])->preRender(d3dContext,&d);
	//RawVerts::Render(d3dContext, data);
}