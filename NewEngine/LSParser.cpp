/* 
// This is the parse. Here L-system strings are passed in
// and vertex positions are passed out to a given tree
*/

#include"LSParser.h"
#include<stack>
#include<sstream>

struct ParseData{
	XMFLOAT4 vector;
	float yaw;
	float pitch;
	float roll;
	XMFLOAT4 yawV;
	XMFLOAT4 pitchV;
	XMFLOAT4 rollV;
	DWORD indice;
	float width;
};

//const float DEG2RAD = 3.14159f / 180;

LSParser::LSParser()
{
	depthChange = 0;
	currNumOfIts = 0;
	bool applyingTrop = false;
	drawWidth = 1.0f;
	//timeSpacing.size();
};

LSParser::~LSParser( )
{
}

float rounder(float in)
{
	return floorf(in * 100000 + 0.5) / 100000;
}

void LSParser::vectorConstruction(XMVECTOR angles, XMVECTOR& up, XMVECTOR& left, XMVECTOR& forward)
{

	//const float DEG2RAD = XM_PI / 180;
	//0.0174532925
	float sx, sy, sz, cx, cy, cz, theta;

	// rotation angle about X-axis (pitch)
	theta = XMVectorGetX(angles);// * DEG2RAD;
	sx = sinf(theta);
	cx = cosf(theta);

	// rotation angle about Y-axis (yaw)
	theta = XMVectorGetY(angles);// * DEG2RAD;
	sy = sinf(theta);
	cy = cosf(theta);

	// rotation angle about Z-axis (roll)
	theta = XMVectorGetZ(angles);// * DEG2RAD;
	sz = sinf(theta);
	cz = cosf(theta);

	sx = rounder(sx); sy = rounder(sy); sz = rounder(sz); cx = rounder(cx); cy = rounder(cy); cz = rounder(cz);
	

	// determine left axis
	left = XMVectorSetX(left, cy*cz);
	left = XMVectorSetY(left, sx*sy*cz + cx*sz);
	left = XMVectorSetZ(left, cx*sy*cz + sx*sz);

	// determine up axis
	up = XMVectorSetX(up, -cy*sz);
	up = XMVectorSetY(up, -sx*sy*sz + cx*cz);
	up = XMVectorSetZ(up, cx*sy*sz + sx*cz);

	// determine forward axis
	forward = XMVectorSetX(forward, sy);
	forward = XMVectorSetY(forward, -sx*cy);
	forward = XMVectorSetZ(forward, cx*cy); 
}

/* Used to find parametric data */
/*pair<float,float> LSParser::valueFind(string* str, int* i, int length)
{
	float distance = 0;
	float width = 0;
	pair<float,float> p; 
	int endPoint = *i;
	if (*i+1 < length && str->at(*i+1) == '('){
		for (int c =*i+2;c<str->size();c++)
			if(str->at(c) == ')'){
				endPoint = c;
				break;
			}
		string value = str->substr(*i+2,endPoint-(*i+2));
		vector<string> values = parseValues(&value);
		distance = Utility::convertString(values.at(0));
		if (values.size()>1)
			width = Utility::convertString(values.at(1));
		p.first = distance;
		p.second = width;
	}else{distance = 0; }
	*i = endPoint;
	return p;
}*/

//Parsing function
bool LSParser::parse(Tree* tree, Leaves* leaves, LSystem* inputData, float time)
{
	bool result = false;
	// Default angle rotation
	float angleAlter = 180.f;
	int numVerts = 0;

	ParseData currentRun = {tree->getBase(),0,0,0};
	XMVECTOR V;
	V = XMVectorSet(0.f,0.f,1.f,1.f);
	XMStoreFloat4(&currentRun.rollV,V);
	V = XMVectorSet(0.f,1.f,0.f,1.f);
	XMStoreFloat4(&currentRun.yawV,V);
	V = XMVectorSet(1.f,0.f,0.f,1.f);
	XMStoreFloat4(&currentRun.pitchV,V);
	stack<ParseData> pStack;

	// Parsing. Iterates over the string
	int ldatasize = inputData->ldata->size();
	for(int i = 0; i< ldatasize;i++)
	{
		LData ldata = inputData->ldata->at(i);
		//Check to see if the symbol represents drawing. Lsystems can have more than one drawing symbol
		if (inputData->drawChars.find_first_of(ldata.action) != string::npos)
		{
			//if (currDepth <= depth)
			//{
			float distance = ldata.data[0];
				//if (distance == 0) distance=1;
				distance = distance;// * currentGrowth[currDepth];
				XMVECTOR v = XMVectorSet(0.f, distance, 0.f, 1.0f);
				XMMATRIX N;
				XMVECTOR angles, up, left, forward;
				XMMATRIX m ;
				forward = XMLoadFloat4(&currentRun.rollV);
				up = XMLoadFloat4(&currentRun.yawV);
				left = XMLoadFloat4(&currentRun.pitchV);
				/* old
				m = XMMatrixSet( XMVectorGetX(left), XMVectorGetY(left), XMVectorGetZ(left), 0.f,
								XMVectorGetX(up), XMVectorGetY(up), XMVectorGetZ(up), 0.f,
								XMVectorGetX(forward), XMVectorGetY(forward), XMVectorGetZ(forward), 0.f,
								0.f, 0.f, 0.f, 1.f);
				//*/
				m = XMMatrixSet( XMVectorGetX(forward), XMVectorGetY(forward), XMVectorGetZ(forward), 0.f,
								XMVectorGetX(up), XMVectorGetY(up), XMVectorGetZ(up), 0.f,
								XMVectorGetX(left), XMVectorGetY(left), XMVectorGetZ(left), 0.f,
								0.f, 0.f, 0.f, 1.f);
				//*/
				v = XMVector3Transform(v, m);
				currentRun.vector.x = XMVectorGetX(v) + currentRun.vector.x; // GET(v)
				currentRun.vector.y = XMVectorGetY(v) + currentRun.vector.y;
				currentRun.vector.z = XMVectorGetZ(v) + currentRun.vector.z;
				currentRun.vector.w = drawWidth;// * currentGrowth[currDepth];
				tree->AddBranch(currentRun.vector,currentRun.indice);
				numVerts++;
				currentRun.indice = numVerts;
				if (applyingTrop)
				{
					//¦Á = e |H ¡ÁT|
					v = XMVectorSet(0.f,1.0f,0.f, 1.0f);
					v = XMVector3Transform(v, m);
					v = XMVector3Cross(v,tropism);
					XMVECTOR length =  XMVector3Length(v);
					//v = XMVectorMultiply(v,tropism);
					float a = branchStrength * XMVectorGetX(length);
					if (a>0){
						m = XMMatrixRotationAxis(v,a);
						XMStoreFloat4(&currentRun.rollV, XMVector3Transform(forward, m));
						XMStoreFloat4(&currentRun.yawV, XMVector3Transform(up, m));
						XMStoreFloat4(&currentRun.pitchV, XMVector3Transform(left, m));
					}
				}
			//}
		}
		//If it isn't a draw symbol then check for other actions
		else
		{
			XMVECTOR up, left, forward;
			XMMATRIX m;
			float angleToAlter = 0;
			switch(ldata.action)
			{
				case '[':
					//currDepth++;
					pStack.push(currentRun);
					break;
				case ']':
					//currDepth--;
					currentRun = pStack.top();
					pStack.pop();
					break;
				//case 'F':
				case '+':
					//if (currDepth <= depth)
					{
						angleToAlter = ldata.data[0];
						if (angleToAlter == 0) angleToAlter = angleAlter;
						forward = XMLoadFloat4(&currentRun.rollV);
						up = XMLoadFloat4(&currentRun.yawV);
						left = XMLoadFloat4(&currentRun.pitchV);
						m = XMMatrixRotationAxis(up,angleToAlter);
						XMStoreFloat4(&currentRun.rollV, XMVector3Transform(forward, m));
						XMStoreFloat4(&currentRun.yawV, XMVector3Transform(up, m));
						XMStoreFloat4(&currentRun.pitchV, XMVector3Transform(left, m));
					}
					break;
				case '-':
					//if (currDepth <= depth)
					{
						angleToAlter = ldata.data[0];
						if (angleToAlter == 0) angleToAlter = angleAlter;
						forward = XMLoadFloat4(&currentRun.rollV);
						up = XMLoadFloat4(&currentRun.yawV);
						left = XMLoadFloat4(&currentRun.pitchV);
						m = XMMatrixRotationAxis(up,-angleToAlter);
						XMStoreFloat4(&currentRun.rollV, XMVector3Transform(forward, m));
						XMStoreFloat4(&currentRun.yawV, XMVector3Transform(up, m));
						XMStoreFloat4(&currentRun.pitchV, XMVector3Transform(left, m));
					}
					break;
				case '&':
					//if (currDepth <= depth)
					{
						angleToAlter = ldata.data[0];
						if (angleToAlter == 0) angleToAlter = angleAlter;
						angleToAlter;// *= currentGrowth[currDepth];
						forward = XMLoadFloat4(&currentRun.rollV);
						up = XMLoadFloat4(&currentRun.yawV);
						left = XMLoadFloat4(&currentRun.pitchV);
						m = XMMatrixRotationAxis(left,angleToAlter);
						XMStoreFloat4(&currentRun.rollV, XMVector3Transform(forward, m));
						XMStoreFloat4(&currentRun.yawV, XMVector3Transform(up, m));
						XMStoreFloat4(&currentRun.pitchV, XMVector3Transform(left, m));
					}
					break;
				case '^':
					//if (currDepth <= depth)
					{
						angleToAlter = ldata.data[0];
						if (angleToAlter == 0) angleToAlter = angleAlter;
						angleToAlter;// *= currentGrowth[currDepth];
						forward = XMLoadFloat4(&currentRun.rollV);
						up = XMLoadFloat4(&currentRun.yawV);
						left = XMLoadFloat4(&currentRun.pitchV);
						m = XMMatrixRotationAxis(left,-angleToAlter);
						XMStoreFloat4(&currentRun.rollV, XMVector3Transform(forward, m));
						XMStoreFloat4(&currentRun.yawV, XMVector3Transform(up, m));
						XMStoreFloat4(&currentRun.pitchV, XMVector3Transform(left, m));
					}
					break;
				case '/':
					//if (currDepth <= depth)
					{
						angleToAlter = ldata.data[0];
						if (angleToAlter == 0) angleToAlter = angleAlter;
						forward = XMLoadFloat4(&currentRun.rollV);
						up = XMLoadFloat4(&currentRun.yawV);
						left = XMLoadFloat4(&currentRun.pitchV);
						m = XMMatrixRotationAxis(forward,angleToAlter);
						XMStoreFloat4(&currentRun.rollV, XMVector3Transform(forward, m));
						XMStoreFloat4(&currentRun.yawV, XMVector3Transform(up, m));
						XMStoreFloat4(&currentRun.pitchV, XMVector3Transform(left, m));
					}
					break;
				case '\\':
					//if (currDepth <= depth)
					{
						angleToAlter = ldata.data[0];
						if (angleToAlter == 0) angleToAlter = angleAlter;
						forward = XMLoadFloat4(&currentRun.rollV);
						up = XMLoadFloat4(&currentRun.yawV);
						left = XMLoadFloat4(&currentRun.pitchV);
						m = XMMatrixRotationAxis(forward,-angleToAlter);
						XMStoreFloat4(&currentRun.rollV, XMVector3Transform(forward, m));
						XMStoreFloat4(&currentRun.yawV, XMVector3Transform(up, m));
						XMStoreFloat4(&currentRun.pitchV, XMVector3Transform(left, m));
						/*
						//forward = XMLoadFloat4(&currentRun.rollV);
						//up = XMLoadFloat4(&currentRun.yawV);
						//left = XMLoadFloat4(&currentRun.pitchV);
						//float lx = XMVectorGetX(left);
						//float ly = XMVectorGetY(left);
						//float lz = XMVectorGetZ(left);
						//float ux = XMVectorGetX(up);
						//float uy = XMVectorGetY(up);
						//float uz = XMVectorGetZ(up);
						//float fx = XMVectorGetX(forward);
						//float fy = XMVectorGetY(forward);
						//float fz = XMVectorGetZ(forward);
						float lx = 0;
						float ly = -1;
						float lz = 0;
						float ux = 1;
						float uy = 0;
						float uz = 0;
						float fx = 0;
						float fy = 0;
						float fz = 1;
						float inx = 0;
						float iny = 0;
						float inz = 0;
						/*
					    XMMATRIX m1 = XMMatrixSet(XMVectorGetX(forward), XMVectorGetY(forward),	XMVectorGetZ(forward), 0.f,
										XMVectorGetX(up),		XMVectorGetY(up),		XMVectorGetZ(up), 0.f,
										XMVectorGetX(left),	XMVectorGetY(left),	XMVectorGetZ(left), 0.f,
										0.f, 0.f, 0.f, 1.f);
						//forward = XMLoadFloat4(&currentRun.rollV);
						//up = XMLoadFloat4(&currentRun.yawV);
						//left = XMLoadFloat4(&currentRun.pitchV);
						//*
						XMMATRIX m1 = XMMatrixSet(XMVectorGetX(left),		XMVectorGetY(left),		XMVectorGetZ(left), 0.f,
										XMVectorGetX(up),		XMVectorGetY(up),		XMVectorGetZ(up), 0.f,
										XMVectorGetX(forward),	XMVectorGetY(forward),	XMVectorGetZ(forward), 0.f,
										0.f, 0.f, 0.f, 1.f);
						/*
						XMMATRIX m2 = XMMatrixSet(fx,fy,fz, 0.f,
										ux,uy,uz, 0.f,
										lx,ly,lz, 0.f,
										0.f, 0.f, 0.f, 1.f);
						//*
						XMMATRIX m2 = XMMatrixSet(lx,ly,lz, 0.f,
										ux,uy,uz, 0.f,
										fx,fy,fz, 0.f,
										0.f, 0.f, 0.f, 1.f);
						//m2 = XMMatrixTranspose(m2);
						m2 = XMMatrixMultiply(m1,m2);
						//v = XMVector3Transform(v, m);

						float tempx = fx * lx + fy * ux + fz * fx + 1 * 0;
						float tempy = fx * ly + fy * uy + fz * fy + 1 * 0;
						float tempz = fx * lz + fy * uz + fz * fz + 1 * 0;
						XMVectorSetX(forward, tempx/1);
						XMVectorSetY(forward, tempy/1);
						XMVectorSetZ(forward, tempz/1);
							  tempx = ux * lx + uy * ux + uz * fx + 1 * 0;
							  tempy = ux * ly + uy * uy + uz * fy + 1 * 0;
							  tempz = ux * lz + uy * uz + uz * fz + 1 * 0;
						XMVectorSetX(up, tempx/1);
						XMVectorSetY(up, tempy/1);
						XMVectorSetZ(up, tempz/1);
							  tempx = lx * lx + ly * ux + lz * fx + 1 * 0;
							  tempy = lx * ly + ly * uy + lz * fy + 1 * 0;
							  tempz = lx * lz + ly * uz + lz * fz + 1 * 0;
						XMVectorSetX(left, tempx/1);
						XMVectorSetY(left, tempy/1);
						XMVectorSetZ(left, tempz/1);
						//*/
					}
					break;
				case '$':
					//if (currDepth <= depth)
					{
						//Faulty?
						forward = XMLoadFloat4(&currentRun.rollV);
						up = XMLoadFloat4(&currentRun.yawV);
						left = XMLoadFloat4(&currentRun.pitchV);
						left = XMVector3Cross(XMVectorSet(0.f,-1.f,0.f,1.f),up);
						forward = XMVector3Cross(up,left);
						XMStoreFloat4(&currentRun.rollV,forward);
						XMStoreFloat4(&currentRun.yawV, up);
						XMStoreFloat4(&currentRun.pitchV,left);//*/
					}
					break;
				case '!':
					{
						drawWidth = ldata.data[0];
					}
					break;
				case 'L':
					leaves->AddLeaf(XMFLOAT3(currentRun.vector.x,currentRun.vector.y,currentRun.vector.z),
										currentRun.pitchV,
										currentRun.yawV,
										currentRun.rollV);
					break;
				default:
					;
			}
		}
	}
	//currentGrowth.clear();
	pStack.empty();
	drawWidth = 1.0f;
	return result;
}

void LSParser::setParseGrowthData(int numOfIts)
{
	//Here we calculate the time splicing that decides when new branch depths are rendered
	// and how face branches reach their desired angle.
	for(int a=0; a<timeSpacing.size();a++)
	{
		for (int b=0; b<timeSpacing[a].size(); b++)
		{
			timeSpacing[a].clear();
		}
	}
	timeSpacing.clear();
	timeSpacing.resize(numOfIts);
	timeSpacing[0].resize(numOfIts);

	float t = 1;
	float spacing = t/(numOfIts+1);
	for(int i=0; i<numOfIts;i++)
	{
		timeSpacing[0][i] = spacing*2;
		t = 1;
		for (int b=0; b<=i; b++)
		{
			t -= timeSpacing[0][b];
		}
		spacing = t/(numOfIts-i);
	}
	t = 1;
	for(int a=1; a<numOfIts;a++)
	{
		int size = numOfIts-a;
		timeSpacing[a].resize(size);
		t = t-timeSpacing[0][size];
		for(int i=0; i<size;i++)
		{
			timeSpacing[a][i] = timeSpacing[0][i]/t;
		}
	}
	currNumOfIts = numOfIts-1;
}

void LSParser::setTropism(XMFLOAT3 t, float e)
{
	tropism = XMLoadFloat4(&XMFLOAT4(t.x,t.y,t.z,1.0f));
	branchStrength = e;
	applyingTrop = true;
}

void LSParser::clearTropism()
{
	applyingTrop = false;
}