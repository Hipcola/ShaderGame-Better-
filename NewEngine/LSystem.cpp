/*
   This is the L-system core. Here rewriting is controlled
   It rules can be set manually inside the class or passed in

   Load order demands
   - Draw chars(example: drawChars = String("F")) before setting rules (example: lSys[1]->setRules(&rules))
*/

#include"LSystem.h"
#include<time.h>
#include<cmath>
#include"utility.h"

//remove
/*
#define DONT_SAVE_VSGLOG_TO_TEMP
#define VSG_DEFAULT_RUN_FILENAME L"A1.vsglog"
#include <vsgcapture.h>
//*/

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

//push_back
#include <boost/spirit/include/phoenix_stl.hpp>

//boost structs
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>

//bind func
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/bind.hpp>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

struct RuleInput
{
	unsigned int padSize;
	unsigned int pad1;
	unsigned int pad2;
	unsigned int pad3;
};

LSystem::LSystem() 
{
	output = "";//"A(10,1)";
	ignores = "+-/\&^F";
	numOfIters = 1;
	globalTime = 0;
	resultMax = 0;
	resultParaMax = 0;
	vertCount = 0;
	srand( time(NULL));
	for (int i = 0;i<10;i++)
		random = rand();
	rules = new vector<RuleData>();
	rulesParsed = new vector<RuleData>();
	hTable = new unordered_map<char,float>();
	maxRewrites = new unordered_map<char,fourUint>();
	rewrites = new unordered_map<string,pair<unsigned int, unsigned int>>();
	ldata = new vector<LData>();
	
	gpuDataSet = false;
	inData			 = NULL;
	inDataSRV		 = NULL;
	inDataUAV		 = NULL;
	param			 = NULL;
	paramSRV		 = NULL;
	paramUAV		 = NULL;
	rulesB			 = NULL;
	rulesSRV		 = NULL;
	ruleOffsets		 = NULL;
	ruleOffsetsSRV   = NULL;
	tableChars		 = NULL;
	tableCharsSRV	 = NULL;
	tableVals		 = NULL;
	tableValsSRV	 = NULL;
	rewriteChars	 = NULL;
	rewriteCharsSRV  = NULL;
	rewriteVals		 = NULL;
	rewriteValsSRV	 = NULL;

	versOutput		= NULL;
	versOutputSRV	= NULL;
	indsOutput		= NULL;
	indsOutputSRV	= NULL;

	gpuLIterator = NULL;
}

//Steals input data... lolwhy
LSystem::LSystem(LSystem* lcpy) 
{
	output.assign(*lcpy->getString());
	baseString.assign(*lcpy->getString());
	ignores = "+-/\F";
	numOfIters = 1;
	globalTime = 0;
	resultMax = 0;
	resultParaMax = 0;
	vertCount = 0;
	srand( time(NULL));
	for (int i = 0;i<10;i++)
		random = rand();
	rules = new vector<RuleData>();
	rulesParsed = new vector<RuleData>();
	hTable = new unordered_map<char,float>();
	maxRewrites = new unordered_map<char,fourUint>();
	rewrites = new unordered_map<string,pair<unsigned int, unsigned int>>();
	ldata = new vector<LData>();

	gpuDataSet = false;
	inData			 = NULL;
	inDataSRV		 = NULL;
	inDataUAV		 = NULL;
	param			 = NULL;
	paramSRV		 = NULL;
	paramUAV		 = NULL;
	rulesB			 = NULL;
	rulesSRV		 = NULL;
	ruleOffsets		 = NULL;
	ruleOffsetsSRV   = NULL;
	tableChars		 = NULL;
	tableCharsSRV	 = NULL;
	tableVals		 = NULL;
	tableValsSRV	 = NULL;
	rewriteChars	 = NULL;
	rewriteCharsSRV  = NULL;
	rewriteVals		 = NULL;
	rewriteValsSRV	 = NULL;

	versOutput		= NULL;
	versOutputSRV	= NULL;
	indsOutput		= NULL;
	indsOutputSRV	= NULL;

	rules->swap(*lcpy->getRules());
	rulesParsed->swap(*lcpy->getRulesParsed());
	hTable->swap(*lcpy->getTable());
	ldata->swap(*lcpy->ldata);
	maxRewrites->swap(*lcpy->getRWMax());
	rewrites->swap(*lcpy->getRW());
	drawChars.swap(lcpy->drawChars);

	gpuLIterator = NULL;
}

LSystem::~LSystem( )
{
	rules->clear();
	rulesParsed->clear();
	hTable->clear();
	ldata->clear();
	maxRewrites->clear();
	rewrites->clear();
	
	delete rewrites;
	delete maxRewrites;
	delete ldata;
	delete rules;
	delete rulesParsed;
	delete hTable;
	if (gpuLIterator != NULL)
		delete gpuLIterator;
}

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

vector<string> LSystem::parseValues(string* str)
{
	vector<std::string> n;
	using boost::spirit::qi::char_;
	using boost::spirit::ascii::space;
	using boost::spirit::qi::lit;
	string::iterator first = str->begin();
	string::iterator last = str->end();

	bool r = boost::spirit::qi::phrase_parse(first, last,
	(
		*(char_ - ',') % ','
	)
	,
	space,n);
	return n;
}

vector<string> LSystem::parseValues2(string* str, string::iterator first, string::iterator last)
{
	vector<string> n;
	using boost::spirit::qi::char_;
	using boost::spirit::ascii::space;
	using boost::spirit::qi::lit;

	bool r = boost::spirit::qi::phrase_parse(first, last,
	(
		//char_ % ','
		*(char_ - ',') % ','
	)
	,
	space,n);
	return n;
}

vector<char> LSystem::parseValues(string* str, string::iterator first, string::iterator last)
{
	vector<char> n;
	using boost::spirit::qi::char_;
	using boost::spirit::ascii::space;
	using boost::spirit::qi::lit;

	bool r = boost::spirit::qi::phrase_parse(first, last,
	(
		//char_ % ','
		*(char_ - ',') % ','
	)
	,
	space,n);
	return n;
}

typedef map<char, float> pairs_type;

template <typename Iterator>
struct PreConParser : qi::grammar<Iterator, pairs_type()>
{
    PreConParser(map<char,float>& m) : PreConParser::base_type(start)
    {
		using qi::double_;
		using qi::char_;
		using qi::_val;
		using qi::_1;
		using qi::_2;

		start =  item % ';';
		item = key >> ('=' >> sum);
		key = char_-'=';
		sum = (plus | neg | div | mul | one)[_val = _1];

		plus = (num >> '+' >> num)[_val = _1 + _2];
		neg =  (num >> '-' >> num)[_val = _1 - _2];
		div =  (num >> '/' >> num)[_val = _1 / _2];
		mul =  (num >> '*' >> num)[_val = _1 * _2];
		one =  (num)[_val = _1];

		num = (bracket | d | c)[_val = _1];
		bracket = '(' >> sum[_val = _1] >> ')';
		d  = double_[_val = _1];
		c  = char_[_val = phoenix::ref(m)[_1]];
    }
    qi::rule<Iterator, pairs_type()> start;
	qi::rule<Iterator, pair<char, float>()> item;
	qi::rule<Iterator, char()> key;
	qi::rule<Iterator, float()> sum, plus, neg, div, mul, num, one, bracket, d, c;
};

bool LSystem::parsePreCon(string* str, map<char,float>& minput, map<char,float>& out)
{
	using ascii::space;
	string::iterator first = str->begin();
	string::iterator last = str->end();

	PreConParser<string::iterator> g(minput);
	//map<char,double> out2;
	bool r = qi::parse(first, last, g, out); //parse_phrase

	if (first != last) // fail if we did not get a full match
		return false;
	return r;
}

namespace ConditionalFunc
{
	void isLessThan(float const& i1, float const& i2, bool& val)
	{
		if (i1 < i2)
			val = true;
		else
			val = false;
	}

	void isLessEqThan(float const& i1, float const& i2, bool& val)
	{
		if (i1 <= i2)
			val = true;
		else 
			val = false;
	}

	void isGreaterThan(float const& i1, float const& i2, bool& val)
	{
		if (i1 > i2)
			val = true;
		else 
			val = false;
	}

	void isGreaterEqThan(float const& i1, float const& i2, bool& val)
	{
		if (i1 >= i2)
			val = true;
		else 
			val = false;
	}
}

template <typename Iterator>
struct ConditionalParse : qi::grammar<Iterator, vector<bool>()>
{
    ConditionalParse(std::map<char,float>& m) : ConditionalParse::base_type(start)
    {
		using qi::double_;
		using qi::char_;
		using qi::_val;
		using qi::_1;
		using qi::_2;
		start =  item % ';';
		item = (let | lt | get | gt)[_val = _1];

		let = (num >> "<=" >> num)[phoenix::bind(&ConditionalFunc::isLessEqThan,_1,_2,_val)];
		lt = (num >> '<' >> num)[phoenix::bind(&ConditionalFunc::isLessThan,_1,_2,_val)];
		get = (num >> ">=" >> num)[phoenix::bind(&ConditionalFunc::isGreaterEqThan,_1,_2,_val)];
		gt = (num >> '>' >> num)[phoenix::bind(&ConditionalFunc::isGreaterThan,_1,_2,_val)];
		num = (d | c)[_val = _1];
		d  = double_[_val = _1];
		c  = char_[_val = phoenix::ref(m)[_1]];
    }

    qi::rule<Iterator, std::vector<bool>()> start;
	qi::rule<Iterator, bool()> item, lt, gt, let, get;
	qi::rule<Iterator, float()> num, d, c;
};

bool LSystem::parseCond(string* str, map<char,float>& minput)
{
	using ascii::space;
	string::iterator first = str->begin();
	string::iterator last = str->end();
	vector<bool> out;
	ConditionalParse<string::iterator> g(minput);
	bool r = qi::parse(first, last, g, out); //parse_phrase

	if (first != last) // fail if we did not get a full match
		return false;

	for (int a = 0; a<out.size(); a++)
	{	
		if (out[a] == false)
		{
			r = false;
			break;
		}
	}
	out.clear();
	return r;
}

template <typename Iterator>
struct LDataparserOutput : qi::grammar<Iterator, vector<LData>()>
{
    LDataparserOutput(map<char,float>& m) : LDataparserOutput::base_type(start)
    {
		using qi::double_;
		using qi::char_;
		using qi::_val;
		using qi::_1;
		using qi::_2;

		start =  *(item);
		item = ( (rs >> '(' >> params >>')') | (rs) );
		params = op >> *(','>> op);
		rs = char_[_val = _1];

		op = num [_val = _1] 
			>> *( ('+' >> num[_val += _1])
			|	  ('-' >> num[_val -= _1])
			|	  ('/' >> num[_val /= _1])	 
			|	  ('*' >> num[_val *= _1]) );

		num = (d | c);
		d  = double_;
		c  = char_[_val = phoenix::ref(m)[_1]];
    }
    qi::rule<Iterator, vector<LData>()> start;
	qi::rule<Iterator, LData()> item;
	qi::rule<Iterator, vector<float>()> params;
	qi::rule<Iterator, unsigned int()> rs;
	qi::rule<Iterator, float()> num, d, c, op, plus, neg, div, mul, one;
};

bool LSystem::parseProduct(string* str, map<char,float>& minput, vector<LData>& out)
{
	using boost::spirit::ascii::space;
	string::iterator first = str->begin();
	string::iterator last = str->end();

	LDataparserOutput<string::iterator> g(minput);
	bool r = qi::parse(first, last, g, out); //parse_phrase

	if (first != last) // fail if we did not get a full match
		return false;
	return r;
}

template <typename Iterator>
struct RuleParser : qi::grammar<Iterator, vector<LData>()>
{
    RuleParser() : RuleParser::base_type(start)
    {
		using qi::double_;
		using qi::char_;
		using qi::_val;
		using qi::_1;
		using qi::_2;

		start =  *(item);
		item = ((rs >> '(' >> params >>')') | (rs));
		params = sum >> *(','>> sum);
		sum = (num >> *(op >> num))[_val = 0];

		rs = char_[_val = _1];

		plus = '+';
		neg  = '-';
		div  = '/';
		mul  = '*';
		op	 = (plus | neg | div | mul)[_val = _1];
		//op = ('+' | '-' | '/' | '*')[_val = _1];

		num = (d | c)[_val = 0];
		d  = double_[_val = 0];
		c  = char_[_val = 0];
    }
    qi::rule<Iterator, vector<LData>()> start;
	qi::rule<Iterator, LData()> item;
	qi::rule<Iterator, vector<float>()> params;
	qi::rule<Iterator, unsigned int()> rs;
	qi::rule<Iterator, char()> bCheck, op, plus, neg, div, mul;
	qi::rule<Iterator, float()> sum, num, d, c;
};

bool LSystem::parseRule(string* str, vector<LData>& out)
{
	using boost::spirit::ascii::space;
	string::iterator first = str->begin();
	string::iterator last = str->end();

	//vector<float> temp;
	RuleParser<string::iterator> g;
	bool r = qi::parse(first, last, g, out); //parse_phrase

	if (first != last) // fail if we did not get a full match
		return false;
	return r;
}
//*/
bool LSystem::bracketSkip(string* temp, int* pos)
{
	bool success = false;
	int bracketCount = 0;
	if (temp->at(*pos) == ']')
	{
		for (int i=*pos-1;i>0;i--)
		{
			if (temp->at(i) == ']')
			{
				bracketCount++;
			}
			else if (temp->at(i) == '[')
			{
				if (bracketCount == 0)
				{
					*pos = i;
					success = true;
					break;
				}
				else bracketCount--;
			}
		}
	}
	else if (temp->at(*pos) == '[')
	{
		for (int i=*pos+1;i<temp->size();i++)
		{
			if (temp->at(i) == '[')
			{
				bracketCount++;
			}
			else if (temp->at(i) == ']')
			{
				if (bracketCount == 0)
				{
					*pos = i;
					success = true;
					break;
				}
				else bracketCount--;
			}
		}
	}
	return success;
}

//Production rule calculation for timed LSystems
vector<LData> LSystem::calculateProduct(string* product, map<char,float> minput)
{
	vector<LData> output = vector<LData>();
	parseProduct(product, minput, output);
	return output;
}

void LSystem::iterate(float time)
{
	clock_t t;
	t = clock();
	float duration;

	if (ldata->size() == 0)
	{
		map<char,float> mapD;
		for ( auto it = hTable->begin(); it != hTable->end(); ++it )
		{
			mapD[it->first] = it->second;
		}
		parseProduct(&output, mapD, *ldata);
	}
	int passes = ldata->size();

	vector<LData> temp (*ldata);
	vector<vector<LData>> inserts = vector<vector<LData>>();
	vector<int> insertPositions = vector<int>();

	//update time
	globalTime += time;

	//Why use an iterator? 
	//http://stackoverflow.com/questions/7396289/vector-iterators
	// (Dropped Iterator) ^ says iterator is bad (Or < is bad, should use !=. Fuck
	// (Picked Iterator back up) Because erase only accepts and iterator
	// (Dropped Iterator again) But you can just say begin() + Counter

	for(int i = 0; i<passes; i++)
	//for(vector<LData>::iterator it = temp->begin(); it<temp->end();it++)
	{
		//LData* thisPass =  it._Ptr;
		LData* thisPass = &temp.at(i);
		//if ((thisPass.currAge + time) >= thisPass.terminalAge)
		//{
		vector<int> matches;
		for (int a = 0; a<rules->size(); a++)
		{	
			if (thisPass->action == rules->at(a).strPre[0])
			{
				matches.push_back(a);
			}
		}
		map<char,float> mapD;
		for ( auto it = hTable->begin(); it != hTable->end(); ++it )
		{
			mapD[it->first] = it->second;
		}
		//mapD['~'] = time;
		for (int a=0;a<matches.size();a++)
		{
			bool lCheck = true;
			bool rCheck = true;
			int thisMatch = matches.at(a);
			RuleData rule = rules->at(thisMatch);

			//LeftContext Check
			if (rule.LContext.size()>0 && lCheck)
			{
			}

			//RightContext Check
			if (rule.RContext.size()>0 && rCheck && lCheck)
			{
			}

			//Conditional Checks
			bool conditionalCheck = true;
			if (rCheck && lCheck)
			{
				vector<char> chars;
				if (rule.strPre[1] == '(')
				{
					vector<string> pv = parseValues2(&rule.strPre, rule.strPre.begin()+2, rule.strPre.end()-1);
					for (unsigned int pvIter=0;pvIter<pv.size();pvIter++)
					{
						pair<char,float> p = make_pair(pv.at(pvIter)[0], thisPass->data.at(pvIter));
						if (pvIter == 2)
						{
							ldata->at(i-inserts.size()).data[2] = thisPass->data.at(2)+time;
							//mapD[pv.at(pvIter)[0]] = thisPass->data.at(2)+time;
							bool convTest = false;
							float charConv = Utility::convertString(pv.at(pvIter),convTest);
							if (convTest){
								if (thisPass->data.at(2)+time < charConv)
									rCheck = false;
							}
							else{
								mapD[pv.at(pvIter)[0]] = thisPass->data.at(2)+time;
							}
						}
						else{
							mapD.insert(p);
						}
					}
				}
				//duration = (clock() - t);
				//string a = "Para Read = ";
				//a.append(Utility::convertFloat(duration));
				//a.append("\n");
				//OutputDebugString(a.c_str());

				if (rule.preCondition.size()>0)
				{
					map<char,float> out;
					bool test = parsePreCon(&rule.preCondition, mapD, out);

					//duration = (clock() - t);
					//string a = "PrecondRead: ";
					//a.append(rule.preCondition);
					//a.append(" Time = "); 
					//a.append(Utility::convertFloat(duration));
					//a.append("\n");
					//OutputDebugString(a.c_str());

					for ( auto it = out.begin(); it != out.end(); ++it )
					{
						mapD[it->first] = it->second;
					}
					out.clear();
				}

				if(rule.Condition.size()>0)
				{
					//t = clock();
					//for (int test1 = 0; test1<1000; test1++)
					//{
						conditionalCheck = parseCond(&rule.Condition, mapD);
					//}
					//duration = (clock() - t);
					//string a = "PrecondRead: ";
					//a.append(rule.Condition);
				    //a.append(" Time = "); 
					//a.append(Utility::convertFloat(duration));
					//a.append("\n");
					//OutputDebugString(a.c_str());
				}
			}

			//Setting up the product
			if (conditionalCheck && rCheck && lCheck)
			{
				vector<LData> newProd;
				newProd = calculateProduct(&rule.Product, mapD);
				int inPos = i-insertPositions.size();
				insertPositions.push_back(inPos);
				vector<LData>::iterator it = ldata->begin() + inPos;
				ldata->erase(it);
				inserts.push_back(newProd);
				newProd.clear();
				break;
			}
		}
		mapD.clear();
		matches.clear();
	}

	//t = clock();

	//ldata->resize(ldata->size()+newSize);
	int sizeInc = 0;
	for (int i = 0; i<inserts.size();i++)
	{
		vector<LData>::iterator it = ldata->begin();
		it += insertPositions[i] + sizeInc;
		ldata->insert(it,inserts[i].begin(),inserts[i].end());
		sizeInc += inserts[i].size();
		inserts[i].clear();
	}
	numOfIters++;
	temp.clear();
	inserts.clear();
	insertPositions.clear();

	duration = (clock() - t);
	thread::id this_id = this_thread::get_id();
	stringstream a;
	a << "Thread " << this_id << " finished iterate = " << Utility::convertFloat(duration) << "\n";
	OutputDebugStringA(a.str().c_str());
	//storeLsystemData();
}

std::thread LSystem::iterateThread(float time)
{
	return std::thread(&LSystem::iterate, this, time);
}

void LSystem::settingGPUData(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext)
{
	//t = clock();
	gpuDataSet = true;

	if (gpuLIterator == NULL)
	{
		gpuLIterator = new LSystemGPU;
		gpuLIterator->Load(d3dDevice);
	}

	SAFE_RELEASE(rulesB);
	SAFE_RELEASE(rulesSRV);
	SAFE_RELEASE(ruleOffsets);
	SAFE_RELEASE(ruleOffsetsSRV);
	SAFE_RELEASE(tableChars);
	SAFE_RELEASE(tableCharsSRV);
	SAFE_RELEASE(tableVals);
	SAFE_RELEASE(tableValsSRV);
	SAFE_RELEASE(rewriteChars);
	SAFE_RELEASE(rewriteCharsSRV);
	SAFE_RELEASE(rewriteVals);
	SAFE_RELEASE(rewriteValsSRV);
	SAFE_RELEASE(param);
	SAFE_RELEASE(paramSRV);
	SAFE_RELEASE(paramUAV);
	SAFE_RELEASE(inData);
	SAFE_RELEASE(inDataSRV);
	SAFE_RELEASE(inDataUAV);

	unsigned int pc = hTable->size();
	std::vector<threeUint> inDataVec;
	for (unsigned int i=0;i<ldata->size();i++)
	{
		threeUint ld;
		LData  l = ldata->at(i);
		ld.a = l.action;
		ld.b = pc;
		ld.c = l.data.size();
		pc +=  l.data.size();
		inDataVec.push_back(ld);
	}
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*3, inDataVec.size(), inDataVec.data(), &inData);
	Utility::CreateBufferSRV(d3dDevice, inData, &inDataSRV );
	Utility::CreateBufferUAV(d3dDevice, inData, &inDataUAV );
	inDataVec.clear();

	std::vector<float> paramData;
	vector<float> htv;
	for(auto kv : *hTable) 
		htv.push_back(kv.second);  
	for (unsigned int i=0;i<htv.size();i++)
		paramData.push_back(htv.at(i));
	htv.clear();

	for (unsigned int i=0;i<ldata->size();i++)
	{
		LData l = ldata->at(i);
		for (unsigned int p=0;p<l.data.size();p++)
			paramData.push_back(l.data[p]);
	}
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(float), paramData.size(), paramData.data(), &param );
	Utility::CreateBufferSRV(d3dDevice, param, &paramSRV );
	Utility::CreateBufferUAV(d3dDevice, param, &paramUAV );
	paramData.clear();

	unsigned int numRules = rulesParsed->size();
	vector<unsigned int> rd;
	vector<unsigned int> rod;
	for (int i=0; i<numRules; i++)
	{
		RuleData r = rulesParsed->at(i);
		unsigned int size1 = r.strPre.size();
		rod.push_back(rd.size());
		//rod.push_back(size1);
		for (int strPreI = 0; strPreI < size1; strPreI++)
		{
			rd.push_back(r.strPre[strPreI]);
		}
		unsigned int size2 = r.Product.size();
		rod.push_back(rd.size());
		//rod.push_back(size2);
		for (int ProductI = 0; ProductI < size2; ProductI++)
		{
			rd.push_back(r.Product[ProductI]);
		}
		rod.push_back(rd.size());
		//So shitty.
		string why = rules->at(i).strPre;
		rod.push_back(rewrites->at(why).first);
		rod.push_back(rewrites->at(why).second);
		rod.push_back(maxRewrites->at(r.strPre[0]).c);
		rod.push_back(maxRewrites->at(r.strPre[0]).d);
	}
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int), rd.size(), rd.data(), &rulesB);
	Utility::CreateBufferSRV(d3dDevice, rulesB, &rulesSRV );
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int), rod.size(), rod.data(), &ruleOffsets);
	Utility::CreateBufferSRV(d3dDevice, ruleOffsets, &ruleOffsetsSRV );

	rd.clear();
	rod.clear();

	/*duration = (clock() - t);
	std::string a = "rule setups = ";
	a.append(Utility::convertFloat(duration));
	a.append("\n");
	OutputDebugString(a.c_str());*/
}

//Just parses the data as is, no iteration
void LSystem::parseOnly(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext)
{
	if (!gpuDataSet)
		settingGPUData(d3dDevice,d3dContext);

	ID3D11Buffer*               paramRW = NULL;
	ID3D11ShaderResourceView*   paramRWSRV = NULL;
	ID3D11UnorderedAccessView*  paramRWUAV = NULL;

	//Unfortunetly, most machines don't zero the data when you give the buffers null.
	//So here we create the data to pass in.
	//std::vector<unsigned int> data;
	std::vector<sixUint> data2;
	data2.resize(resultMax*6);

	unsigned int sumTotal = resultMax/BLOCKSIZE;
	sumTotal++;

	//Could the draw chars not be permantly loaded?
	std::vector<unsigned int> charConversion;
	for (int i=0; i<drawChars.size(); i++)
		charConversion.push_back(drawChars[i]);
	std::sort( charConversion.begin(), charConversion.end() );
	ID3D11Buffer*               drawChar = NULL;
	ID3D11ShaderResourceView*   drawCharSRV = NULL;
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int), charConversion.size(), charConversion.data(), &drawChar);
	Utility::CreateBufferSRV(d3dDevice, drawChar, &drawCharSRV );

	ID3D11ShaderResourceView*  arry2SRV[3] = { drawCharSRV, inDataSRV, paramSRV };
	UINT verTotal = 0;

	gpuLIterator->parse(d3dContext, d3dDevice, arry2SRV, &versOutput, &indsOutput, &versOutputSRV, &indsOutputSRV, &paramUAV, ldata->size(), verTotal);
	vertCount = verTotal;
	indCount = (verTotal-1)*2;

	struct fourfloat{
		float a;
		float b;
		float c;
		float d;
		fourfloat() : a(0),b(0),c(0),d(0) {}
	};

	SAFE_RELEASE(paramRW);
	SAFE_RELEASE(paramRWSRV);
	SAFE_RELEASE(paramRWUAV);
	SAFE_RELEASE(drawChar);
	SAFE_RELEASE(drawCharSRV);
}

// vector implementation
// http://stackoverflow.com/questions/6096279/keeping-a-valid-vectoriterator-after-erase
void LSystem::iterateGPU(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext, float time)
{
	//clock_t t;
	//float duration;
	if (!gpuDataSet)
		settingGPUData(d3dDevice,d3dContext);

	ID3D11Buffer*               result = NULL;
	ID3D11ShaderResourceView*   resultSRV = NULL;
	ID3D11UnorderedAccessView*  resultUAV = NULL;
	ID3D11Buffer*               paramRW = NULL;
	ID3D11ShaderResourceView*   paramRWSRV = NULL;
	ID3D11UnorderedAccessView*  paramRWUAV = NULL;
	ID3D11Buffer*               sums = NULL;
	ID3D11ShaderResourceView*   sumsSRV = NULL;
	ID3D11UnorderedAccessView*  sumsUAV = NULL;
	ID3D11Buffer*               rwSize = NULL;
	ID3D11ShaderResourceView*   rwSizeSRV = NULL;
	ID3D11UnorderedAccessView*  rwSizeUAV = NULL;
	ID3D11Buffer*               rSize = NULL;
	ID3D11ShaderResourceView*   rSizeSRV = NULL;
	ID3D11UnorderedAccessView*  rSizeUAV = NULL;
	ID3D11Buffer*               rPtr = NULL;
	ID3D11ShaderResourceView*   rPtrSRV = NULL;
	ID3D11UnorderedAccessView*  rPtrUAV = NULL;

	//*
	//unsigned int rmTest = resultMax;
	if (resultMax == 0)
	{
		resultParaMax = hTable->size();
		unsigned int ldataSize = ldata->size();
		for(int i = 0; i< ldataSize; i++)
		{
			try
			{
				fourUint p = maxRewrites->at(ldata->at(i).action);
				resultMax += p.a;
				resultParaMax += p.b;
			}catch (const std::out_of_range& oor) 
			{
				resultMax++;
				resultParaMax += ldata->at(i).data.size();
			}
		}
	}

	//Unfortunetly, most machines don't zero the data when you give the buffers null.
	//So here we create the data to pass in.
	//std::vector<unsigned int> data;
	std::vector<sixUint> data2;
	data2.resize(resultMax*6);

	//HRESULT d3dResult;
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*3, resultMax, NULL, &result);
	Utility::CreateBufferSRV(d3dDevice, result, &resultSRV );
	Utility::CreateBufferUAV(d3dDevice, result, &resultUAV );

	Utility::CreateStructuredBuffer(d3dDevice, sizeof(float), resultParaMax, NULL, &paramRW );
	Utility::CreateBufferSRV(d3dDevice, paramRW, &paramRWSRV );
	Utility::CreateBufferUAV(d3dDevice, paramRW, &paramRWUAV );

	unsigned int sumTotal = resultMax/BLOCKSIZE;
	sumTotal++;

	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*2, sumTotal*BLOCKSIZE, data2.data(), &rSize );
	Utility::CreateBufferSRV(d3dDevice, rSize, &rSizeSRV );
	Utility::CreateBufferUAV(d3dDevice, rSize, &rSizeUAV );

	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*2, sumTotal, data2.data(), &sums );
	Utility::CreateBufferSRV(d3dDevice, sums, &sumsSRV );
	Utility::CreateBufferUAV(d3dDevice, sums, &sumsUAV );

	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*2, sumTotal, data2.data(), &rwSize );
	Utility::CreateBufferSRV(d3dDevice, rwSize, &rwSizeSRV );
	Utility::CreateBufferUAV(d3dDevice, rwSize, &rwSizeUAV );

	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*6, ldata->size(), data2.data(), &rPtr );
	Utility::CreateBufferSRV(d3dDevice, rPtr, &rPtrSRV );
	Utility::CreateBufferUAV(d3dDevice, rPtr, &rPtrUAV );

	ID3D11ShaderResourceView*  arrySRV[2] = { rulesSRV, ruleOffsetsSRV };
	ID3D11UnorderedAccessView* arryUAV[8] = { inDataUAV, resultUAV, paramUAV, paramRWUAV, sumsUAV, rSizeUAV, rPtrUAV, rwSizeUAV };

	/*
	ID3D11Buffer* debugbuf5 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, param );
	D3D11_MAPPED_SUBRESOURCE MappedResource5;
    d3dContext->Map( debugbuf5, 0, D3D11_MAP_READ, 0, &MappedResource5 );
	float* paraDD = (float*)MappedResource5.pData;	
	d3dContext->Unmap( debugbuf5, 0 );
	*/

	if (ldata->size() == 2813)
		float hi2 = 1324;

	//run shader			
	gpuLIterator->run(d3dContext, arrySRV, arryUAV, ldata->size(), rulesParsed->size()*7, hTable->size(), time);

	SAFE_RELEASE(inData);
	SAFE_RELEASE(inDataSRV);
	SAFE_RELEASE(inDataUAV);
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int)*3, resultMax, NULL, &inData );
	Utility::CreateBufferSRV(d3dDevice, inData, &inDataSRV );
	Utility::CreateBufferUAV(d3dDevice, inData, &inDataUAV );
	d3dContext->CopyResource(inData,result);

	SAFE_RELEASE(param);
	SAFE_RELEASE(paramSRV);
	SAFE_RELEASE(paramUAV);
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(float), resultParaMax, NULL, &param );
	Utility::CreateBufferSRV(d3dDevice, param, &paramSRV );
	Utility::CreateBufferUAV(d3dDevice, param, &paramUAV );
	d3dContext->CopyResource(param,paramRW);

	///*
	ID3D11Buffer* debugbuf2 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, result );
	D3D11_MAPPED_SUBRESOURCE MappedResource2;
    d3dContext->Map( debugbuf2, 0, D3D11_MAP_READ, 0, &MappedResource2 );
	threeUint* d = (threeUint*)MappedResource2.pData;	
	d3dContext->Unmap( debugbuf2, 0 );

	ID3D11Buffer* debugbuf4 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, paramRW );
	D3D11_MAPPED_SUBRESOURCE MappedResource4;
    d3dContext->Map( debugbuf4, 0, D3D11_MAP_READ, 0, &MappedResource4 );
	float* paraD = (float*)MappedResource4.pData;	
	d3dContext->Unmap( debugbuf4, 0 );
	//*/

	ID3D11Buffer* debugbuf = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, rSize );
	D3D11_MAPPED_SUBRESOURCE MappedResource;
    d3dContext->Map( debugbuf, 0, D3D11_MAP_READ, 0, &MappedResource );
	twoUint* ldataOut = (twoUint*)MappedResource.pData;	
	d3dContext->Unmap( debugbuf, 0 );

	ID3D11Buffer* debugbuf3 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, rwSize );
	D3D11_MAPPED_SUBRESOURCE MappedResource3;
    d3dContext->Map( debugbuf3, 0, D3D11_MAP_READ, 0, &MappedResource3 );
	twoUint* rwSizeOut = (twoUint*)MappedResource3.pData;	
	d3dContext->Unmap( debugbuf3, 0 );

	unsigned int lsize = 0;
	if (ldata->size() < BLOCKSIZE)
		lsize = ldataOut[0].a;
	else{
		unsigned int goToLast = (((ldata->size()/BLOCKSIZE)+1)*BLOCKSIZE)-1;
		lsize = ldataOut[goToLast].a;
	}

	unsigned int newResultMax = 0;
	unsigned int newParamMax = 0;
	//Crash on other machines fix
	//http://stackoverflow.com/questions/16696444/differences-between-running-an-executable-with-visual-studio-debugger-vs-without
	//new
	unsigned int sizeChecker = 0;
	sizeChecker = lsize/BLOCKSIZE;
	sizeChecker++;
	for (int i = 0; i < sizeChecker; i++)
	{
		newResultMax += rwSizeOut[i].a;
		newParamMax  += rwSizeOut[i].b;
	}
	if (newResultMax > resultMax)
		resultMax = newResultMax;
	if (newParamMax > resultParaMax)
		resultParaMax = newParamMax;

	if (resultMax     > 9999999) 
		float fucksdf = 123;
	if (resultParaMax > 9999999) 
		float fucksdf = 123;
	if (resultMax == 0) 
		float fucksdf = 123;
	if (resultParaMax == 0) 
		float fucksdf = 123;
	
	///*
	ldata->clear();
	for (unsigned int i=0; i<lsize; i++)
	{
		LData l;
		threeUint thisData = d[i];
		l.action = thisData.a;
		unsigned int pSize = thisData.c;
		unsigned int pStart = thisData.b;
		for (unsigned int ptr=0; ptr<pSize;ptr++)
			l.data.push_back(paraD[pStart+ptr]);
		ldata->push_back(l);
	}
	//*/

	//
	// FIRST STAGE END
	//
	std::vector<unsigned int> charConversion;
	for (int i=0; i<drawChars.size(); i++)
		charConversion.push_back(drawChars[i]);
	std::sort( charConversion.begin(), charConversion.end() );
	ID3D11Buffer*               drawChar = NULL;
	ID3D11ShaderResourceView*   drawCharSRV = NULL;
	Utility::CreateStructuredBuffer(d3dDevice, sizeof(unsigned int), charConversion.size(), charConversion.data(), &drawChar);
	Utility::CreateBufferSRV(d3dDevice, drawChar, &drawCharSRV );

	struct fourfloat{
		float a;
		float b;
		float c;
		float d;
		fourfloat() : a(0),b(0),c(0),d(0) {}
	};
	std::vector<fourfloat> data;
	data.resize(lsize);

	ID3D11ShaderResourceView*  arry2SRV[3] = { drawCharSRV, inDataSRV, paramSRV };
	UINT verTotal = 0;

	gpuLIterator->parse(d3dContext, d3dDevice, arry2SRV, &versOutput, &indsOutput, &versOutputSRV, &indsOutputSRV, &paramUAV, lsize, verTotal);
	vertCount = verTotal;
	indCount = (verTotal-1)*2;

	/*
	ID3D11Buffer* debugbuf22 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, versOutput );
	D3D11_MAPPED_SUBRESOURCE MappedResource22;
    d3dContext->Map( debugbuf22, 0, D3D11_MAP_READ, 0, &MappedResource22 );
	XMFLOAT4* d = (XMFLOAT4*)MappedResource22.pData;	
	d3dContext->Unmap( debugbuf22, 0 );

	ID3D11Buffer* debugbuf44 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, indsOutput );
	D3D11_MAPPED_SUBRESOURCE MappedResource44;
    d3dContext->Map( debugbuf44, 0, D3D11_MAP_READ, 0, &MappedResource44 );
	unsigned int* paraD = (unsigned int*)MappedResource44.pData;	
	d3dContext->Unmap( debugbuf44, 0 );
	*/
	SAFE_RELEASE(debugbuf2);
	SAFE_RELEASE(debugbuf4);
	SAFE_RELEASE(debugbuf);
	SAFE_RELEASE(debugbuf3);

	SAFE_RELEASE(result);
	SAFE_RELEASE(resultSRV);
	SAFE_RELEASE(resultUAV);
	SAFE_RELEASE(paramRW);
	SAFE_RELEASE(paramRWSRV);
	SAFE_RELEASE(paramRWUAV);
	SAFE_RELEASE(sums);
	SAFE_RELEASE(sumsSRV);
	SAFE_RELEASE(sumsUAV);
	SAFE_RELEASE(rSize);
	SAFE_RELEASE(rSizeSRV);
	SAFE_RELEASE(rSizeUAV);
	SAFE_RELEASE(rwSize);
	SAFE_RELEASE(rwSizeSRV);
	SAFE_RELEASE(rwSizeUAV);
	SAFE_RELEASE(rPtr);
	SAFE_RELEASE(rPtrSRV);
	SAFE_RELEASE(rPtrUAV);

	/*
	duration = (clock() - t);
	std::wstring a = L"Entire pass time = ";
	a.append(Utility::convertFloatW(duration));
	a.append(L"\n");
	OutputDebugString(a.c_str());
	//*/
}

float LSystem::tableMath(string input, float firstValue, float secondValue)
{
	float result = 0;
	switch (input.at(1))
	{
		case '+': 
			result = firstValue + secondValue;
			break;
		case '-': 
			result = firstValue - secondValue;
			break;
		case '/': 
			result = firstValue / secondValue;
			break;
		case '*': 
			result = firstValue * secondValue;
		default:
			break;
	}
	return result;
}

void LSystem::reset()
{
	output.clear();
	output.assign(baseString);
	ldata->clear();
	map<char,float> mapD;
	for ( auto it = hTable->begin(); it != hTable->end(); ++it )
	{
		mapD[it->first] = it->second;
	}
	parseProduct(&output, mapD, *ldata);
}

string* LSystem::getString()
{
	return &output;
}

int LSystem::getLength()
{
	return output.length();
}

vector<RuleData>* LSystem::getRules()
{
	return rules;
}

vector<RuleData>* LSystem::getRulesParsed()
{
	return rulesParsed;
}

unordered_map<char,float>* LSystem::getTable()
{
	return hTable;
}

unordered_map<char, fourUint>* LSystem::getRWMax()
{
	return maxRewrites;
}

unordered_map<string, pair<unsigned int, unsigned int>>* LSystem::getRW()
{
	return rewrites;
}

/*
	This is more than a simple set function.
	Here the rules are all parsed and a new set of rules is created where instead of
	tokens representing data, each rules parameters are integers refering to a point in a data
	table that's used on the gpu.

	It also fills two other tables which hold the sizes of various data 
	relating to each rules productions
*/
void LSystem::setRules(vector<RuleData>* input)
{
	rules->clear();
	delete rules;
	rulesParsed->clear();
	delete rulesParsed;
	rules = new vector<RuleData> (*input);
	rulesParsed = new vector<RuleData>;

	//Here we iterate the hTable to find the order the values will be passed to the gpu
	numOfIters = 1;
	vector<LData> temp;
	vector<unsigned int> htv;
	for (auto kv : *hTable)
		htv.push_back(kv.first);  

	for (int i=0; i<rules->size(); i++)
	{
		//Curr rule
		RuleData r = rules->at(i);
		//the new parsed rule
		RuleData rNew = r;
		temp.clear();
		//This simple parse is used to detect output sizes, used later
		parseRule(&rules->at(i).Product,temp);

		//This entire loop is checking each parameter and replacing it with an integer relating to its position
		//in a data table later used on the gpu
		vector<string> pv = parseValues2(&r.strPre, r.strPre.begin()+2, r.strPre.end()-1);
		//unsigned int size = r.strPre.size();
		unsigned int offSet = 0;
		//Rule preposition parse
		for(unsigned int rParser = 0; rParser<r.strPre.size()/*size*/; rParser++)
		{
			//char c = r.strPre[rParser];
			string c = r.strPre.substr(rParser,1);
			if ( c[0] == 41 || c[0] == 40 ){}
			else
			{
				if (drawChars.find_first_of(c) != string::npos) {}
				else
				{
					try{
						float test = hTable->at(c[0]);
						unsigned int pos = 0;
						for ( auto it = hTable->begin(); it != hTable->end(); ++it ){
							if (it->first == c[0])
								break;
							pos++;
						}
						c = ((unsigned int)'0')+pos;
					}catch (const std::out_of_range& oor) {
						for (unsigned int pVals=0; pVals<pv.size(); pVals++){
							string test = pv.at(pVals);
							if (test[0] == c[0])
							{
								unsigned int numTracked = hTable->size()+pVals;
								if (numTracked>9)
								{
									//numTracked = 123;
									while (numTracked>9)
									{
										unsigned int val = numTracked;
										unsigned int counter = 0;
										while(val>9)
										{
											val /= 10;
											counter++;
										}
										//val = numTracked/10;
										c = ((unsigned int)'0')+val;
										//rNew.strPre[rParser] = c;
										rNew.strPre.insert(rParser+offSet,c);
										offSet++;
										//size++;
										//rParser++;
										while (counter != 0)
										{
											val *= 10;
											counter--;
										}
										numTracked -= val;
									}
									c = ((unsigned int)'0')+numTracked;
								}
								else{
									c = ((unsigned int)'0')+numTracked;
									break;
								}
							}
						}
					}
				}
			}
			rNew.strPre[rParser+offSet] = c[0]; 
		}

		//Rule product parse
		offSet = 0;
		for(unsigned int rParser = 0; rParser < r.Product.size(); rParser++)
		{
			//char c = r.Product[rParser];
			string c = r.Product.substr(rParser,1);
			if ( c[0] == 41 || c[0] == 40 ){}
			else
			{
				if (drawChars.find_first_of(c) != string::npos) {}
				else
				{
					try{
						float test = hTable->at(c[0]);
						unsigned int pos = 0;
						for ( auto it = hTable->begin(); it != hTable->end(); ++it ){
							if (it->first == c[0])
								break;
							pos++;
						}
						c = ((unsigned int)'0')+pos;
					}catch (const std::out_of_range& oor) 
					{
						for (unsigned int pVals=0; pVals<pv.size(); pVals++)
						{
							string test = pv.at(pVals);
							if (test[0] == c[0])
							{
								unsigned int numTracked = hTable->size()+pVals;
								if (numTracked>9)
								{
									//numTracked = 123;
									while (numTracked>9)
									{
										unsigned int val = numTracked;
										unsigned int counter = 0;
										while(val>9)
										{
											val /= 10;
											counter++;
										}
										//val = numTracked/10;
										c = ((unsigned int)'0')+val;
										//rNew.strPre[rParser] = c;
										rNew.Product.insert(rParser+offSet,c);
										offSet++;
										//size++;
										//rParser++;
										while (counter != 0)
										{
											val *= 10;
											counter--;
										}
										numTracked -= val;
									}
									c = ((unsigned int)'0')+numTracked;
								}
								else
								{
									c = ((unsigned int)'0')+numTracked;
									break;
								}
							}
						}
					}
				}
			}
			rNew.Product[rParser+offSet] = c[0]; 
		}
		rulesParsed->push_back(rNew);

		//Here we're setting up the rewrite data
		unsigned int count = 0;
		for(unsigned int paramParser=0; paramParser<temp.size(); paramParser++)
			count += temp[paramParser].data.size();
		rewrites->insert(pair<string,pair<unsigned int,unsigned int>>(rules->at(i).strPre,pair<unsigned int, unsigned int>(temp.size(),count)));

		//We then we find the largest possible for each character, some have multiple rules 
		try{
			fourUint last = maxRewrites->at(rules->at(i).strPre[0]);
			if (last.a < temp.size())
				maxRewrites->at(rules->at(i).strPre[0]) = fourUint(temp.size(),count,0,0);
		}catch (const std::out_of_range& oor) {
			maxRewrites->insert(pair<char,fourUint>(rules->at(i).strPre[0],fourUint(temp.size(),count,0,0)));
		}
	}

	//We then need to parse the rules again and use are previous rewrite data to find the second max rewrite out put.
	//For example, F(a)->F(a)F(a). maxRewrites['F'] = fourUint (2,2,4,4);
	for (int i=0; i<rules->size(); i++)
	{
		RuleData r = rules->at(i);
		temp.clear();
		parseRule(&rules->at(i).Product,temp);
		unsigned int count = 0;
		unsigned int countP = 0;
		for(unsigned int parser=0; parser<temp.size(); parser++)
		{
			fourUint pData;
			try{ 
				pData = maxRewrites->at(temp[parser].action);
				count  += pData.a;
				countP += pData.b;
			}
			catch (const std::out_of_range& oor) {
				count++;
				countP += temp[parser].data.size();
			}
		}
		try{
			fourUint last = maxRewrites->at(rules->at(i).strPre[0]);
			if (last.c < count)
				maxRewrites->at(rules->at(i).strPre[0]) = fourUint(temp.size(),last.b,count,countP);
		}catch (const std::out_of_range& oor) {
			//Shouldn't ever hit this
			maxRewrites->insert(pair<char,fourUint>(rules->at(i).strPre[0],fourUint(99,99,99,99)));
		}
	}
	temp.clear();
	gpuDataSet = false;
}

void LSystem::setString(string* input)
{
	output.clear();
	baseString.clear();
	baseString.assign(*input);
	output.assign(*input);
	ldata->clear();
	map<char,float> mapD;
	for ( auto it = hTable->begin(); it != hTable->end(); ++it )
	{
		mapD[it->first] = it->second;
	}
	parseProduct(&output, mapD, *ldata);
	numOfIters = 1;
	//testing125 = true;
}

void LSystem::setParaData(unordered_map<char,float>* input)
{
	hTable->clear();
	delete hTable;
	hTable = new unordered_map<char,float> (*input);
	gpuDataSet = false;

	/*unordered_map<char,float>::iterator it = hTable->begin();
	while (it != hTable->end())
		bTable.setVariable(string(it->first,1),it->second);*/
}

int LSystem::getIterations()
{
	return numOfIters;
}

/*vector<LData>* LSystem::getLData();
{
	return Ldata;
}*/

unsigned int LSystem::getVersCount()
{
	return vertCount;
}

ID3D11ShaderResourceView* LSystem::getVers(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext)
{
	if(versOutputSRV == NULL)
	{
		parseOnly(d3dDevice, d3dContext);
	}
	/*
	ID3D11Buffer* debugbuf5 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, versOutput );
	D3D11_MAPPED_SUBRESOURCE MappedResource5;
    d3dContext->Map( debugbuf5, 0, D3D11_MAP_READ, 0, &MappedResource5 );
	XMFLOAT4* dSize = (XMFLOAT4*)MappedResource5.pData;	
	d3dContext->Unmap( debugbuf5, 0 );
	ID3D11Buffer* debugbuf6 = Utility::CreateAndCopyToDebugBuf( d3dDevice, d3dContext, indsOutput );
	D3D11_MAPPED_SUBRESOURCE MappedResource6;
    d3dContext->Map( debugbuf6, 0, D3D11_MAP_READ, 0, &MappedResource6 );
	unsigned int* indstest = (unsigned int*)MappedResource6.pData;	
	d3dContext->Unmap( debugbuf6, 0 );
	*/
	return versOutputSRV;
}

ID3D11ShaderResourceView* LSystem::getInds()
{
	return indsOutputSRV;
}

