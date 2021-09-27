/* Contains our Tree rendering code */
#ifndef _LP_H_
#define _LP_H_

#include"Tree.h"
#include"Leaves.h"
#include"utility.h"
#include"LSystem.h"
#include<string>
#include<vector>
using namespace std;

class LSParser
{
    public:
        LSParser();
        virtual ~LSParser( );
		void vectorConstruction(XMVECTOR angles, XMVECTOR& up, XMVECTOR& left, XMVECTOR& forward);

		//LData Parsing (Growth)
		bool parse(Tree* tree, Leaves* leaves, LSystem* lsys, float time);

		void setParseGrowthData(int numOfIts);
		void setTropism(XMFLOAT3 t, float e);
		void clearTropism();
    private:
		//pair<float,float> valueFind(string* str, int* i, int length);
		vector<vector<float>> timeSpacing;
		XMVECTOR tropism;
		float branchStrength;
		float drawWidth;
		bool applyingTrop;
		int currNumOfIts;
		int depthChange;
	
};

#endif