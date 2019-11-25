#include "Agent.h"
#include "Maizefield.h"

#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Random.h"
#include "repast_hpc/Point.h"
#include <stdio.h> 
#include <vector>
#include <string>

Agent::Agent(repast::AgentId agentId, int currentAge, int fertileAge, int deathAge, int infertileAge, int maizeLocX, int maizeLocY): agentId(agentId), currentAge(currentAge), fertileAge(fertileAge), deathAge(deathAge), infertileAge(infertileAge), maizeLocX(maizeLocX), maizeLocY(maizeLocY)
{
	repast::IntUniformGenerator gen = repast::Random::instance()->createUniIntGenerator(2000, 2400); // initialise random number generator
	
	previousYield[0] = gen.next();
}
Agent::~Agent() {}


bool Agent::checkDeath() 
{
	//std::cout <<"Current age:"<< currentAge <<" Death Age: " <<deathAge<<std::endl;
	if (deathAge == currentAge)
	{
		return true;     
   	}
    else
	{
       	return false;
    }          
}

bool Agent::checkMaize()
{
	expectedYield = maizeStock + previousYield[0]+previousYield[1];

	if (expectedYield < 800)
	{
		checkMove = 1;
	}
	else
	{
		checkMove = 0;
	}
	return(checkMove);
}

bool Agent::fissionReady()
{
	//std::cout<<"Current Age: "<<currentAge<<std::endl;
	if(currentAge>16)
	{
		repast::IntUniformGenerator gen = repast::Random::instance()->createUniIntGenerator(0, 8);
		int fChance = gen.next();
		//std::cout<<"Random Fertile Chance(0->8): "<<fChance<<std::endl;
		if(fChance == 8)
		{
			updateAge(); 
			return true;
		}
		else 
		{
			updateAge();
			return false;
		}
	}
	else 
		updateAge();
		return false; 
};

void Agent::printAttributes()
{
	//std::cout <<"Printing Agent:" << std::endl; 
	//std::cout <<"Agent id:"<< AgentId id <<std::endl; 
	std::cout <<"Agent Current age: "<< currentAge <<std::endl; 
	std::cout << "Death Age: " <<deathAge<<std::endl;
	//std::cout << "infertile Age: " <<infertileAge<<std::endl;
}

int Agent::getMaizeLocX()
{

	return maizeLocX; 
}

int Agent::getMaizeLocY()
{

	return maizeLocY; 
}

void Agent::updateAge()
{
	currentAge++; 
}

void Agent::updateMaizeStock(int Yield)
{	
	if(tick > 2)
	{
		tick =0; 
	}
	else
	{
		maizeStock=Yield;
		if(Yield > 800)
		{
			previousYield[tick] = Yield-800;
		}
		else
		{
			previousYield[tick] = 0;
		}
		tick++; 
	}
}

void Agent::Maizeloc2str()
{
	int xval = getMaizeLocX();
	int yval = getMaizeLocY();
	Xval = std::to_string(xval);
	Yval = std::to_string(yval);
}

/*int Agent:: getAttribute(){
 // reads in the attributes for yield calculations from prop files

    props = new repast::Properties(propsFile);
    /*
     = repast::strToInt(props->getProperty(""));
     = repast::strToInt(props->getProperty(""));
     = repast::strToInt(props->getProperty(""));
	
    return 0;
}*/

/*
void Agent:: fissionReady(){
	if (currentAge>fertileAge&&currentAge<unfertileAge){
		if (repast::Random::instance()>fissionprobability){
		return true;
	}
	}
else return false;
}	
*/