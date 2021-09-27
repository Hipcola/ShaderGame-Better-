#include"LSSpeedTesting.h"
#include"LSystem.h"
//#include <iostream>
#include<unordered_map>
#include<map>


const float DEG2RAD = 3.14159f / 180;

LSSpeedTesting::LSSpeedTesting()
{

}

LSSpeedTesting::~LSSpeedTesting()
{

}

bool LSSpeedTesting::Run()
{
	const int numSystems = 2;
	LSystem* lSys[numSystems];// = new LSystem;
	for (int i = 0; i < numSystems; i++)
		lSys[i] = new LSystem();

	vector<RuleData> rules;
	unordered_map<char, float> table;
	pair<float, XMFLOAT3>* trops = new pair<float, XMFLOAT3>[numSystems];

	RuleData r = RuleData("", "", "", "");

	lSys[0]->setString(&string("A(0.5,0.5,1)"));
	lSys[1]->setString(&string("A(0.5,0.5,1)"));

	r = RuleData("A(x,m,I)", "!(m)F(O,x,O)[&(a)B(x*R,m*w,O)]-(d)A(x*r,m*w,O)");
	rules.push_back(r);
	r = RuleData("B(x,m,I)", "!(m)F(O,x,O)[\\(c)$C(x*R,m*w,O)]C(x*r,m*w,O)");
	rules.push_back(r);
	r = RuleData("C(x,m,I)", "!(m)F(O,x,O)[/(c)$B(x*R,m*w,O)]B(x*r,m*w,O)");
	rules.push_back(r);
	r = RuleData("F(x,m,I)", "D(m,m,I)");
	rules.push_back(r);
	r = RuleData("F(x,m,t)", "F(t/I*m,m,t)");
	rules.push_back(r);

	lSys[0]->drawChars = string("FD");
	lSys[1]->drawChars = string("FD");
	table['r'] = 0.9;//0.9//0.9//0.9//0.9
	table['R'] = 0.9;//0.6//0.9//0.8//0.7
	table['a'] = 30 * DEG2RAD;//45; //45 //45 //45//30
	table['c'] = 30 * DEG2RAD;//45; //45 //45 //45//-30
							  //table['d'] = 120*DEG2RAD;//137.5;
	table['d'] = 90 * DEG2RAD;//137.5;
	table['b'] = 22.5*DEG2RAD;
	table['w'] = 0.707;
	table['O'] = 0;
	table['I'] = 1;
	lSys[0]->setParaData(&table);
	lSys[0]->setRules(&rules);
	lSys[1]->setParaData(&table);
	lSys[1]->setRules(&rules);
	table.clear();
	rules.clear();

	//std::vector<std::thread> threads;
	//threads.push_back(lSys[0]->iterateThread(0.1));
	//threads.push_back(lSys[1]->iterateThread(0.1));

	//threads[0].join();
	//threads[1].join();

	//threads.push_back(lSys[0]->iterateThread(0.6));
	//threads.push_back(lSys[1]->iterateThread(1.2));

	//threads[2].join();
	//threads[3].join();

	//threads.push_back(lSys[0]->iterateThread(0.6));
	//threads.push_back(lSys[1]->iterateThread(1.2));

	//threads[4].join();
	//threads[5].join();

	//threads.push_back(lSys[0]->iterateThread(0.6));
	//threads.push_back(lSys[1]->iterateThread(1.2));

	//threads[6].join();
	//threads[7].join();

	//lSys[1]->setString(&string("!(0.03)F(0,1,0)[&(a)B(1*R,0.03*w,0)]-(d)A(1*r,0.03*w,0)[^(b)L]"));
	//lSys[0]->setString(&string("!(0.01)R(10,10,1)"));
	//char * input;
	//std::cout << "Press any key to run";
	//std::cin >> input;
	return true;
}

