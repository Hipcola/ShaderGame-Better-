/* Contains our Tree rendering code */
#ifndef _TREESYSTEM_H_
#define _TREESYSTEM_H_

#include"RawVerts.h"
#include"LSystem.h"
#include"LSParser.h"
#include"Tree.h"
#include"Leaves.h"
#include<memory>
//#include"

struct LoadDataTreeSystem
{
	LSystem** lsystemPointer;
	pair<float,XMFLOAT3>* tropisms;
	shared_ptr<LSParser> parserPointer;
	int numSystems;
	Shader* leafShader;
	//char* leafName;
};

class TreeSystem : public RawVerts
{
    public:
		TreeSystem();
        virtual ~TreeSystem();
		void Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data);

		//It's highly important that with the tree system
		//the shader data is passed in before the load function, as this data is used in the load function
		//where it's passed on to the trees that the system will be using.
		//This is pretty bad
		void Load(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, LoadDataTreeSystem*);

		void Render(ID3D11DeviceContext* dc, void* d);

    private:
		void setTree(unsigned int treeSetting, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext);
		void clearTrees();
		pair<float,XMFLOAT3>* tropisms;
		LSystem** lSys;
		shared_ptr<LSParser> parser;
		//Or Tree**
		//Or do we instance?
		Tree* trees;
		GPUTreeBuilder* treeBuilder;
		LSystemGPU* lsysBuilder;
		Leaves* leaves;
		bool isGrowing;
		bool isPaused;
		bool renderTree;
		bool renderLeaves;
		bool treeAltered;
		float startTime;
		float totalTime;

		//clock_t testingClock;
		//float testingDuration;
		//float deltaTime;
		int numOfParses;
		int currLSys;
		int numTrees;

		//Time L Testing 
		int currLSize;
		vector<float> testingTimesRw;
		vector<float> testingTimesParse;
		vector<float> testingTimesBuild;
};

#endif