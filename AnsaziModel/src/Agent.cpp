#include "Agent.h"
#include "Maizefield.h"

#include "repast_hpc/Moore2DGridQuery.h"
#include "repast_hpc/Random.h"
#include "repast_hpc/Point.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Properties.h"
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
	expectedYield = maizeStock + previousYield[0] + previousYield[1];

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
	{
		updateAge();
		return false; 
	}
}

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
		tick = 0; 
	}
	//else
	//{
	maizeStock=Yield;
	if(Yield > 800)
	{
		previousYield[tick] += Yield-800;
		if (previousYield[tick] < 0)
		{
			previousYield[tick-1] += previousYield[tick];
		}
	}
	else
	{
		previousYield[tick] -= 800;
		if (previousYield[tick] < 0)
		{
			previousYield[tick] = 0;
		}
	}
	if((previousYield[0] + previousYield[1])>1600)
	{
		previousYield[tick] = 1600-previousYield[tick-1];

	}
	tick=tick+1; 
}

void Agent::Maizeloc2str()
{
	int xval = getMaizeLocX();
	int yval = getMaizeLocY();
	Xval = std::to_string(xval);
	Yval = std::to_string(yval);
}


//void Agent::storedYield 



/*
void Agent::relocateField()
{
	std::vector<MaizeField*> MaizeFieldList;
	std::vector<Agent*> agentList;

	//double currentTick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	int counter, waterSize, waterX, waterY;
	int counterVector = 0;
	int farmMinX, farmMaxX;
	int farmMinY, farmMaxY;
	int farmCounter1, farmCounter2;
	int tempMaize = 0;
	int tempX, tempY;
	int bestX, bestY, bestCounter;

	int farmSize;

	int tempDistance;
	int bestDistance = 0;
   

	bool noFarms = true;
	


	farmSize=MaizeFieldList.size();
	farmSize=currentFarms.size();

	
	for(counter = 0; counter<farmSize; counter++)
	{
		tempX = currentFarms[counter].x;
		tempY = currentFarms[counter].y;
		//std::cout << "Farm x and y" << tempX << "  __  " << tempY << std::endl;

		tempDistance = calcDistance(houseX, houseY, tempX, tempY);

		if( (currentFarms[counter].maizeProduction>tempMaize) && (agentList.size() == 0)  )
		{
			tempMaize = currentFarms[counter].maizeProduction;
			bestX = tempX;
			bestY = tempY;
			bestCounter = counter;
			bestDistance = tempDistance;
			noFarms = false;
		}
		else if ( (currentFarms[counter].maizeProduction == tempMaize) && (agentList.size() == 0) && (tempDistance<bestDistance) )
		{
			tempMaize = currentFarms[counter].maizeProduction;
			bestX = tempX;
			bestY = tempY;
			bestCounter = counter;
			bestDistance = tempDistance;
		}
	}

	if (noFarms)
	{
		isDeadLeft = true;
	//	std::cout << "NO FARMS AVALIABLE" << "   Farm size is: " << farmSize << std::endl;
	}
	else
	{
		maizeLocX = bestX;
		maizeLocY = bestY;
		maizeProduction = tempMaize;

		//std::cout << "x and y" << bestX << " _____ "<< bestY << std::endl;
		//std::cout << "Maize Prod found: " << maizeProduction << std::endl;
		agentList.size() == 0;	
		currentFarms.erase(currentFarms.begin()+bestCounter);
		farmSize=currentFarms.size();
		//std::cout << "Current farms size" << farmSize << std::endl;
	}
}

int Agent::calcDistance(int x1, int y1, int x2, int y2)
{
	int temp1, temp2, temp3;
	temp1=abs(x1-x2);
	temp2=abs(y1-y2);
	temp3 = sqrt( (temp1*temp1) + (temp2*temp2));

	return temp3;
}*/
