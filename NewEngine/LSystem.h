/* Contains our Tree rendering code */
#ifndef _LSYS_H_
#define _LSYS_H_
#include<d3d11.h>
#include<stdlib.h>
#include<DirectXMath.h>

using namespace DirectX;

#include<string>
#include<vector>
#include<map>
#include<thread>
#include<boost/spirit/include/qi.hpp>
#include<boost/fusion/include/adapt_struct.hpp>

#include"LSystemGPU.h"

//#include<forward_list>
//#include<list>
#include<unordered_map>
using namespace std;

/*struct threeUint{
	unsigned int a;
	unsigned int b;
	unsigned int c;
	threeUint() : a(0),b(0),c(0) {}
};*/

/*struct twoUint{
	unsigned int a;
	unsigned int b;
	twoUint() : a(0),b(0) {}
};*/

struct RuleData{
	string LContext;
	string strPre;
	string RContext;
	string preCondition;
	string Condition;
	string Product;
	//float triggerAge;
	RuleData(string sp, string p) 
		: LContext(), strPre(sp), RContext(), preCondition(), Condition(), Product(p){}
	RuleData(string sp, string c, string p) 
		: LContext(), strPre(sp), RContext(), preCondition(), Condition(c), Product(p){}
	RuleData(string lc, string sp, string rc, string p) 
		: LContext(lc), strPre(sp), RContext(rc), preCondition(), Condition(), Product(p){}
	RuleData(string lc, string sp, string rc, string c, string p) 
		: LContext(lc), strPre(sp), RContext(rc), preCondition(), Condition(c), Product(p){}
	RuleData(string lc, string sp, string rc, string pc, string c, string p) 
		: LContext(lc), strPre(sp), RContext(rc), preCondition(pc), Condition(c), Product(p){}
};

/*
//Time values 
struct RuleData{
	string LContext;
	string strPre;
	string RContext;
	string preCondition;
	string Condition;
	string Product;
	float triggerAge;
	RuleData(string sp, string p, float t) 
		: LContext(), strPre(sp), RContext(), preCondition(), Condition(), Product(p), triggerAge(t){}
	RuleData(string sp, string c, string p, float t) 
		: LContext(), strPre(sp), RContext(), preCondition(), Condition(c), Product(p), triggerAge(t){}
	RuleData(string lc, string sp, string rc, string p, float t) 
		: LContext(lc), strPre(sp), RContext(rc), preCondition(), Condition(), Product(p), triggerAge(t){}
	RuleData(string lc, string sp, string rc, string c, string p, float t) 
		: LContext(lc), strPre(sp), RContext(rc), preCondition(), Condition(c), Product(p), triggerAge(t){}
	RuleData(string lc, string sp, string rc, string pc, string c, string p, float t) 
		: LContext(lc), strPre(sp), RContext(rc), preCondition(pc), Condition(c), Product(p), triggerAge(t){}
};*/
struct LData{
	unsigned int action;
	vector<float> data;
	//int depth; 
	//float data1;
	//float data2;
	//float time;
	LData() 
		: action('@'), data(){}
	LData(unsigned int r) 
		: action(r), data()
	{
		if (r == 91 || r == 93)
		{
			data.push_back(0);
		}
	}
	LData(unsigned int r, vector<float> d) 
		: action(r), data(d){}
	/*
	LData() 
		: action('@'), data1(0), data2(0), time(0){}
	LData(unsigned int r) 
		: action(r), data1(0), data2(0), time(0){}
	LData(unsigned int r, float d1) 
		: action(r), data1(d1), data2(0), time(0){}
	LData(unsigned int r, float d1, float d2) 
		: action(r), data1(d1), data2(d2), time(0){}
	LData(unsigned int r, float d1, float d2, float t) 
		: action(r), data1(d1), data2(d2), time(t){}
	*/
};


BOOST_FUSION_ADAPT_STRUCT(
	LData,
	(unsigned int, action)
    (std::vector<float>, data)
	
	//(unsigned int, action)
	//(float, data1)
	//(float, data2)
	//(float, time)
)


class LSystem
{
    public:
        LSystem();
		LSystem(LSystem*);
        virtual ~LSystem( );
		//void iterate();
		void iterate(float time);
		std::thread iterateThread(float time);
		void iterateGPU(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float time);
		string* getString();
		void reset();
		vector<RuleData>* getRules();
		vector<RuleData>* getRulesParsed();
		unordered_map<char,float>* getTable();
		unordered_map<char, fourUint>* getRWMax();
		unordered_map<string, pair<unsigned int, unsigned int>>* getRW();
		void setRules(vector<RuleData>* rules);
		void setString(string* input);
		void setParaData(unordered_map<char,float>* input);
		int getIterations();
		int getLength();
		unsigned int getVersCount();
		string getLDataString();
		void parseOnly(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext);
		ID3D11ShaderResourceView* getVers(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext);
		ID3D11ShaderResourceView* getInds();

		//TODO Test lists and other data types?
		//http://www.baptiste-wicht.com/2012/12/cpp-benchmark-vector-list-deque/
		//forward_list<LData>* ldataList;

		vector<LData>* ldata;
		string drawChars;
		LSystemGPU* gpuLIterator;
		//vector<LData>* getLData();


		unsigned int vertCount;
		unsigned int indCount;

    private:
		vector<string> parseValues(string* input);
		vector<string> parseValues2(string* str, string::iterator first, string::iterator last);
		vector<char> parseValues(string* str, string::iterator first, string::iterator last);

		void settingGPUData(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext);
		bool parsePreCon(string* str, map<char,float>& minput, map<char,float>& out);
		bool parseCond(string* str, map<char,float>& minput);
		bool parseProduct(string* str, map<char,float>& mInput, vector<LData>& out);
		bool parseRule(string* str, vector<LData>& out);

		float tableMath(string input, float firstValue, float secondValue);
		bool bracketSkip(string* temp, int* pos);
		float globalTime;

		//string rewriteProduct(string Product, vector<string> values, vector<string> chars, vector<LData>& out);
		vector<LData> calculateProduct(string* Product, map<char,float>);
		unordered_map<char,float>* hTable;

								//NumModules	//NumParams
		unordered_map<string,pair<unsigned int, unsigned int>>* rewrites;
		unordered_map<char,fourUint>*   maxRewrites;
		vector<RuleData>* rules;
		vector<RuleData>* rulesParsed;
        string output;
		string baseString;
		string ignores;
		int numOfIters;
		int random;

		unsigned int resultMax;
		unsigned int resultParaMax;
		bool gpuDataSet;
		ID3D11Buffer*               inData;
		ID3D11ShaderResourceView*   inDataSRV;
		ID3D11UnorderedAccessView*  inDataUAV;
		ID3D11Buffer*               param;
		ID3D11ShaderResourceView*   paramSRV;
		ID3D11UnorderedAccessView*  paramUAV;
		ID3D11Buffer*               rulesB;
		ID3D11ShaderResourceView*   rulesSRV;
		ID3D11Buffer*               ruleOffsets;
		ID3D11ShaderResourceView*   ruleOffsetsSRV;
		ID3D11Buffer*               tableChars;
		ID3D11ShaderResourceView*   tableCharsSRV;
		ID3D11Buffer*               tableVals;
		ID3D11ShaderResourceView*   tableValsSRV;
		ID3D11Buffer*               rewriteChars;
		ID3D11ShaderResourceView*   rewriteCharsSRV;
		ID3D11Buffer*               rewriteVals;
		ID3D11ShaderResourceView*   rewriteValsSRV;

		ID3D11Buffer*				versOutput;
		ID3D11ShaderResourceView*	versOutputSRV;
		ID3D11Buffer*				indsOutput;
		ID3D11ShaderResourceView*	indsOutputSRV;
};

#endif