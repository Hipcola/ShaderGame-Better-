/* Contains our Tree rendering code */
#ifndef _TREE_H_
#define _TREE_H_

#include"LitRawVerts.h"
#include"GPUTreeBuilder.h"
#include<vector>
using namespace std;

#define SIDES 6

//We add one to the split point, as an the maxverts/sides gives you the number of verts(Or branches) 
// required to fill a buffer, plus one means our split point is only reached when we require two buffers.
#define SPLITPOINT (MAXVERT/SIDES)+1
#define MAXINDS  (SPLITPOINT-2)*(SIDES*6)

class Tree : public LitRawVerts
{
    public:
		Tree();
		Tree(XMFLOAT4 b);
        virtual ~Tree( );
		void AddBranch(XMFLOAT4 v, DWORD indice);
		void Build(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, ID3D11ShaderResourceView* inVers, ID3D11ShaderResourceView* inInds);
		void Update(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float delta, void* data);
		void Render(ID3D11DeviceContext* dc, void* d);

		//Does not run lower lever clear
		//Just clears the tree data
		void Clear ();

		unsigned int getNumVerts();
		unsigned int getNumInds();
		void setNumVerts(unsigned int verts);
		XMFLOAT4 getBase();
		vector<XMFLOAT4>* getVerts();
		//vector<WORD>* getIndices();
		bool isWire;
		bool isGPUBased;
		GPUTreeBuilder* treeBuilder;
		void updateIndices(ID3D11Device* d3dDevice);

		unsigned int numVerts;
		unsigned int numInds;
    private:
		unsigned int sectionNumVerts;
		unsigned int sectionNumInds;
		XMFLOAT3* buildTrunkVerts(int position);

		//Called by constructors
		void setDefaults();

		void runCPUBuild(ID3D11Device* d3dDevice);
		void runGPUBuild(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, ID3D11ShaderResourceView* inVers, ID3D11ShaderResourceView* inInds);

		unsigned int vecBuffCounter;
		vector<vector<XMFLOAT4>> vertices;
		vector<vector<unsigned int>> indices;
		
		unsigned buffOffSet;
		vector<vector<unsigned int>> multiBuffOffsets;
		XMFLOAT4 base;
		bool setBuffers;

		unsigned int numBuffers;
		unsigned int newNumBuffers;
		vector<ID3D11Buffer*>              resultVers;
		vector<ID3D11ShaderResourceView*>  resultVersSRV;
		vector<ID3D11UnorderedAccessView*> resultVersUAV;
		vector<ID3D11Buffer*>              resultInds;
		vector<ID3D11ShaderResourceView*>  resultIndsSRV;
		vector<ID3D11UnorderedAccessView*> resultIndsUAV;
		vector<std::pair<unsigned int, unsigned int>> vecsinds;

		//ID3D11Buffer*	inVers;
		//ID3D11Buffer*	inInds;
		//ID3D11ShaderResourceView*	inVersSRV;
		//ID3D11ShaderResourceView*	inIndsSRV;
};

#endif