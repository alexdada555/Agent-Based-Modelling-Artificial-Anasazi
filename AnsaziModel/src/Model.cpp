#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <utility> // std::pair
#include <sstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <boost/mpi.hpp>

#include "repast_hpc/AgentId.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/initialize_random.h"
#include "repast_hpc/SVDataSetBuilder.h"
#include "repast_hpc/Point.h"
#include "repast_hpc/Random.h"

#include "Model.h"
std::vector<std::vector<std::string> > readcsv(std::string);
std::vector<std::vector<double> > conversion(std::vector<std::vector<std::string> >);


int Mx;
int My;
int Fx = 79;
int Fy = 119;
int fullCheck;


AnsaziModel::AnsaziModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm), Mcontext(comm)
{
	//Load predifined files
	maps = readcsv("src/map.csv");																//Save array to model
	hydro = readcsv("src/hydro.csv");
	water = readcsv("src/water.csv");
	pdsi = readcsv("src/pdsi_yield.csv");

	hydroint = conversion(hydro);
	waterint = conversion(water);
	pdsiint = conversion(pdsi);


	props = new repast::Properties(propsFile, argc, argv, comm);								//Initialise Repast property files

	stopAt = repast::strToInt(props->getProperty("stop.at"));									//Fetch the stop at value from model.props file
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));					//Fetch the count of agents from the model.props file
	boardSizex = repast::strToInt(props->getProperty("board.sizex"));								//Fetch the board size property from model.props
	boardSizey = repast::strToInt(props->getProperty("board.sizey"));								//Fetch the board size property from model.props
	currentYear = repast::strToInt(props->getProperty("startYear"));							//Set the start year for the simulation
	mindeathAge = repast::strToInt(props->getProperty("mindeathAge"));								//Get the death age
	maxdeathAge = repast::strToInt(props->getProperty("maxdeathAge"));								//Get the death age
	MaizeFieldData1 = repast::strToInt(props->getProperty("harvest.adjustment.level"));
	MaizeFieldData2 = repast::strToDouble(props->getProperty("sigmaahv"));
	minFertileAge = repast::strToInt(props->getProperty("minFertileAge"));
	maxFertileAge = repast::strToInt(props->getProperty("maxFertileAge"));
	fissionProb = repast::strToDouble(props->getProperty("fissionProb"));
	q = repast::strToDouble(props->getProperty("q"));
	//std::cout<<"model q: "<<q<<std::endl; 
	//soilquality = repast::strToDouble(props->getProperty("soilquality"));
	//std::cout<<"Fission prob: "<<fissionProb<<std::endl;

    //watertest= waterlocation(1000,"44","96");
    //cout << "water is : " <<  watertest << "\n" ;

	currentId = countOfAgents;

	initializeRandom(*props, comm);

	//Init the shared space
	repast::Point<double> origin(0,0);															//Shared Space Starts at (x,y)
	repast::Point<double> extent(boardSizex,boardSizey);										//Shared Space stops at (x,y)
	repast::GridDimensions gd(origin, extent);													//Shared Space defined as (start,stop)

	std::vector<int> processDims;
	processDims.push_back(1);																	//How the process is split across cores 1 by 1 currently
	processDims.push_back(1);

	discreteSpace = new repast::SharedDiscreteSpace<Agent, repast::StrictBorders, repast::SimpleAdder<Agent> >("AgentDiscreteSpace", gd, processDims, 0, comm); //Initialise the discrete space, "Strict border may change to wrap around borders"
	MdiscreteSpace = new repast::SharedDiscreteSpace<MaizeField, repast::StrictBorders, repast::SimpleAdder<MaizeField> >("MaizeFieldDiscreteSpace", gd, processDims, 0, comm); //Initialise the discrete space, "Strict border may change to wrap around borders"

	context.addProjection(discreteSpace);
	Mcontext.addProjection(MdiscreteSpace);
}

AnsaziModel::~AnsaziModel()
{
	delete props;
	//delete agent;
	//delete maizeField;
}

void AnsaziModel::doPerTick()
{
int currentTick = repast::RepastProcess::instance()->getScheduleRunner().currentTick(); 
	//repast::IntUniformGenerator gen2 = repast::Random::instance()->createUniIntGenerator(0, MaizeFieldData2); // initialise random number generator
	//sigmaahvNew = gen2.next(); 
	std::vector<int> countagent;
	if (currentTick==0 || currentTick==1|| currentTick==2){
			
			std::cout << "-------------------------------------------\n";
			std::cout << "Tick: " << currentTick << "." << std::endl;
		}
	if (currentTick==0 || currentTick==1 ||currentTick== 2){
			std::cout << "Number of Agents " << countOfAgents << std::endl;

	}
	if(countOfAgents<800)
	{
		//cout << "Start of tick ------------------------------------------" << endl;
		//cout << "Current Year: " << currentYear << endl;
		fullCheck = 0;
		if(countOfAgents!= 0)
		{
			removeAgent();
		}
		//else
			//std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!No Agents!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< std::endl;s
		Mx = 0;
		My= 0;
		//Fx = 79;
		//Fy = 119;
		std::cout<< "current year:"<< currentYear<<" Number Agents: "<<countOfAgents<<std::endl;
		//if(currentYear == 800 || currentYear == 900 ||currentYear == 1000||currentYear == 1100||currentYear == 1200||currentYear ==(800+stopAt-1))
		//printToScreen();
		
		//if(currentYear ==(800+stopAt-1))
	}
	countagent.push_back(countOfAgents);
	outputfile(countagent);
	currentYear++;
}

void AnsaziModel::initSchedule(repast::ScheduleRunner& runner)
{
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<AnsaziModel> (this, &AnsaziModel::doPerTick)));
	runner.scheduleStop(stopAt);
}

void AnsaziModel::initAgents()
{
	int rank = repast::RepastProcess::instance()->rank();


	int fertileAge = 16;
	repast::IntUniformGenerator gen2 = repast::Random::instance()->createUniIntGenerator(mindeathAge, maxdeathAge);
	repast::IntUniformGenerator gen3 = repast::Random::instance()->createUniIntGenerator(minFertileAge, maxFertileAge);
	repast::IntUniformGenerator gen4 = repast::Random::instance()->createUniIntGenerator(0, boardSizex-1);
	repast::IntUniformGenerator gen5 = repast::Random::instance()->createUniIntGenerator(0, boardSizey-1);
	repast::IntUniformGenerator gen = repast::Random::instance()->createUniIntGenerator(2000, 2400); // initialise random number generator
	//std::cout<<"=====================================Test 1 ====================="<<std::endl;
	//std::cout<<"----------------------------------Initialisng Agents-------------"<<std::endl;

	for(int i=0 ;i<countOfAgents;i++)//13 agents initially then one made ourselves for testing
	{
		//std::cout<<"NewAgent----------------------"<<std::endl;
		std::vector<Agent*> agentList;
		std::vector<MaizeField*> MaizeFieldList;
		//Load random values to initilaise agents
		//int initialAge=gen2.next();

		int xLoc = gen4.next(); int yLoc = gen5.next();
		int xMLoc = gen4.next(); int yMLoc = gen5.next();

		do{
			MaizeFieldList.clear();
			xLoc = gen4.next(); yLoc = gen5.next();
			MdiscreteSpace->getObjectsAt(repast::Point<int>(xLoc, yLoc), MaizeFieldList);
		}while (MaizeFieldList.size()!=0);


		for(int i = -10; i<=10; i++)
		{
			int xMLoc = xLoc+i;
			if(xLoc>=0&&xLoc<80)
			{
				for(int j = -10; j<=10 ; j++)
				{
					int yMLoc = yLoc+j;
					if(yLoc>=0&&yLoc<120)
					{
						MaizeFieldList.clear();
						agentList.clear();
						discreteSpace->getObjectsAt(repast::Point<int>(xMLoc, yMLoc), agentList);
						MdiscreteSpace->getObjectsAt(repast::Point<int>(xMLoc, yMLoc), MaizeFieldList);
						if(MaizeFieldList.size()==0&&agentList.size()==0)
						{
							goto skip_l;
						}
					}
				}
			}
		}
		skip_l:

		repast::Point<int> initialLocation(xLoc, yLoc);		//Give agent a location
		repast::Point<int> initialMaizeLocation(xMLoc, yMLoc);		//Give MaizeField a location

		repast::AgentId Maizeid(i, rank, 0);						//Give agent a unique ID
		repast::AgentId id(i, rank, 0);						//Give agent a unique ID


		id.currentRank(rank);
		Maizeid.currentRank(rank);

		int deathAge = gen2.next();
		repast::IntUniformGenerator gen1 = repast::Random::instance()->createUniIntGenerator(0, deathAge);
		int initialAge = gen1.next();
		int infertileAge = gen3.next();
		int initialMaize = gen.next(); 
		Agent* agent = new Agent(id, initialAge, fertileAge, deathAge, infertileAge,xMLoc,yMLoc,initialMaize); //Create new agent with defined values
		MaizeField* maizeField = new MaizeField(id, MaizeFieldData1, MaizeFieldData2,q);    //MaizeFieldData
		//std::cout<<"Agent ID: "<<id<<std::endl;
		//std::cout<<"MaizeID ID: "<<maizeField<<std::endl;
		//agent->maizeID = i;
		context.addAgent(agent);
		Mcontext.addAgent(maizeField);
		//std::cout<<"Agent ID: "<<id<<std::endl;
		//std::cout<<"Agent Loc: "<<initialLocation<<std::endl;
		//std::cout<<"Death Age: "<<deathAge<<std::endl;
		//cout<<"Initial location: "<<initialLocation<<endl;
		discreteSpace->moveTo(id, initialLocation);
		MdiscreteSpace->moveTo(Maizeid, initialMaizeLocation);
	}
}

void AnsaziModel::removeAgent()
{
	std::vector<Agent*> agents;
	std::vector<MaizeField*> maizeFields;

	context.selectAgents(repast::SharedContext<Agent>::LOCAL, countOfAgents, agents);

	std::vector<Agent*>::iterator it = agents.begin();

	it = agents.begin();

	//std::cout<<"================================= TEST 5 ================================="<<std::endl;
	//std::cout<<"================================= TEST 4 ================================="<<std::endl;
	while(it != agents.end())
	{
		maizeFields.clear();
		Mcontext.selectAgents(repast::SharedContext<MaizeField>::LOCAL, countOfAgents, maizeFields);
		std::vector<MaizeField*>::iterator Mit = maizeFields.begin();
		Mit = maizeFields.begin();
		while((*Mit) -> getId()!= (*it) -> getId()){
			Mit++;
			//std::cout<<"Agent ID at run: "<<(*it) -> getId()<<std::endl;
	    	//std::cout<<"MaizeID ID at run: "<<(*Mit) -> getId()<<std::endl;
		}
		//(*it) -> printAttributes();
		//std::cout<<"Checking agent is dead-----------------------"<<std::endl;
		if ((*it)->checkDeath())
		{
			//std::cout<<"The Agent ID is: "<<(*it) -> getId().id()<<std::endl;
			//(*it) -> printAttributes();
			//std::cout <<"!Deleted Curent Age as death age met!-----------------------"<<std::endl;

			int maizeLoc[2];

			maizeLoc[0] = (*it) -> getMaizeLocX();
			maizeLoc[1] = (*it) -> getMaizeLocY();

			//MdiscreteSpace->getObjectsAt(repast::Point<int>(maizeLoc[0], maizeLoc[1]), maizeFields);

			//std::cout <<"!Deleted Curent MaizeField"<< (*Mit) -> getId().id()<<" as death age met!"<<std::endl;
			//std::cout <<"Agent was in location : "<<
			//int id = (*it) -> getId().id();

			repast::RepastProcess::instance()->agentRemoved((*it) -> getId());
    		context.removeAgent((*it) -> getId());
			Mcontext.removeAgent((*Mit) -> getId());
			//std::cout<<"Agent Deleted---"<<std::endl;
    		countOfAgents --;
    		//td::cout<<"Death Age met"<<std::endl;
    	}
		else
		{
			//std::cout<<"Changing maizefield attributes"<<std::endl;
			(*Mit)->getAttributes(MaizeFieldData1, MaizeFieldData2);
			std::vector<int> agentLocation;

			MdiscreteSpace->getLocation((*Mit) -> getId(),agentLocation);

			//std::cout << agentLocation[0] << std::endl;

			x = droughtindex(currentYear,agentLocation[0],agentLocation[1]);
			(*Mit)->MaizeProduction(x);
			(*it)->updateMaizeStock((*Mit)->currentYield);
			//std::cout<<"This is The current MaizeField Yield: "<<(*Mit)->currentYield<<std::endl;
			//std::cout<<"This is The Expected MaizeField Yield: "<<(*it)->expectedYield<<std::endl;

			if((*it)->checkMaize() == 1)
			{
				//std::cout<<"The Expected MaizeField Yield < 800: "<<(*it)->expectedYield<<std::endl;
				//std::cout<<"Trying to move agent and maize field: "<<(*it) -> getId()<<(*Mit) -> getId()<<std::endl;

				bool kill = false;
				if(fullCheck == 0)
				{
					//std::cout<<"Moving \n";
					kill = move((*Mit),(*it));
				}
				else
					kill=true;
				//std::cout<<"done"<<std::endl;
				if(kill)
				{
					//std::cout<<"Killing maize: "<<(*Mit) -> getId()<<std::endl;
					repast::RepastProcess::instance()->agentRemoved((*it) -> getId());
	    			context.removeAgent((*it) -> getId());
					Mcontext.removeAgent((*Mit) -> getId());
	    			countOfAgents --;
		    		//std::cout<<"Couldnt move anywhere"<<std::endl;
	    		}
			}
			if((*it)->fissionReady(fissionProb)&&fullCheck==0){
				//std::cout<<"Fission \n";
				fissionProcess((*it));
			}
		}
		it++;
	}
}

bool AnsaziModel::move(MaizeField* Mit, Agent* it)
{
	std::vector<MaizeField*> MaizeFieldList;

	std::vector<Agent*> agentList;
	bool selectionA = false, selectionB = true; //Selection A makes sure there's a clear space and selection B makes sure there's enough maize being produced.
	//std::cout<<"Moving"<<std::endl;
	//Initialise random numbers
	//Move the miaze field
	int run = 0;

	int xMLoc = -1; int yMLoc = -1;
	bool end = false;

	for(int i=Mx; i<80; i++)
	{
		for(int j = 0; j <120; j++)
		{
			//int yieldcurrent = Mit->currentYield;
			if(run == 0)
			{
				if(My<119)
				{
					j = My+1;
				}
				run = 1;
			}
			//std::cout<<i<<j<<std::endl;
			agentList.clear(); //
			MaizeFieldList.clear();
			discreteSpace->getObjectsAt(repast::Point<int>(i, j), agentList);
			MdiscreteSpace->getObjectsAt(repast::Point<int>(i, j), MaizeFieldList);
			if(agentList.size() == 0 && MaizeFieldList.size() == 0 && waterlocation(currentYear,i, j) == 0) //done need to move to closest agent
			{
				x = droughtindex(currentYear,i,j);
				//This isn't write!!!!!!!============================================================================================
				Mit->MaizeProduction(x);
				int yieldcurrent = Mit->currentYield; //This gets the current yield stored for that year
				//int storedYield = it->storedYield();
				/*
				int BY = x*MaizeFieldData1*MaizeFieldData1
				int yieldcurrent;
				yieldcurrent = BY * (1 + MaizeFieldData2);*/
				if(yieldcurrent > 800)
				{

					xMLoc = i;
					yMLoc = j;
					Mx = i;
					My = j;
					//std::cout<<"Mx, MY:"<<Mx<<My<<std::endl;
					goto skip_loop;
				}
			}
		}
	}
	run = 0;
	skip_loop:

	if(xMLoc == -1)
	{
		Mx=0;
		My=0;
		fullCheck = 1;
		//std::cout<<"full check"<<std::endl;
		//std::cout<<"End of loop reached"<<std::endl;
		return true;
	}
	else
	{
		repast::Point<int> MaizeLocation(xMLoc, yMLoc);
		MdiscreteSpace->moveTo(Mit -> getId(), MaizeLocation);

		std::vector<int> gXloc;
		std::vector<int> gYloc;

		for(int i = -10; i<=10; i++)
		{
			int xLoc = xMLoc+i;
			if(xLoc>=0&&xLoc<80)
			{

				for(int j = -10; j<=10 ; j++)
				{
					int yLoc = yMLoc+j;
					if(yLoc>=0&&yLoc<120)
					{
						float a,b,c;
						c=sqrt((i^2)+(j^2));
						if (c<=10)
						{
							agentList.clear(); //
							MaizeFieldList.clear();
							discreteSpace->getObjectsAt(repast::Point<int>(xLoc, yLoc), agentList);
							MdiscreteSpace->getObjectsAt(repast::Point<int>(xLoc, yLoc), MaizeFieldList);
							if(MaizeFieldList.size()==0 && waterlocation(currentYear, xLoc,yLoc) ==0 && agentList.size() ==0)
							{
								gXloc.push_back(xLoc);
								gYloc.push_back(yLoc);
							}
						}
					}
				}
			}
		}

		if (gXloc.size() ==0)
		{
			return true;
		}
		else
		{
			repast::Point<int> agentLocation(gXloc[0], gYloc[0]);
			discreteSpace->moveTo(it -> getId(), agentLocation);
			return false;
		}
	}
	//std::cout<<"done"<<std::endl;
}

void AnsaziModel::fissionProcess(Agent* it)
{
	//Initialise vectors
	//std::vector<Agent*> agents;
	std::vector<Agent*> agentList;
	//std::vector<MaizeField*> MaizeFieldList;
	std::vector<MaizeField*> MaizeList;															//Make "it" the statrting element
	repast::IntUniformGenerator gen2 = repast::Random::instance()->createUniIntGenerator(mindeathAge, maxdeathAge);
	repast::IntUniformGenerator gen3 = repast::Random::instance()->createUniIntGenerator(minFertileAge, maxFertileAge);
	//repast::IntUniformGenerator gen = repast::Random::instance()->createUniIntGenerator(1000, 1600); // initialise random number generator

	int initialAge= 0;
	//int infertileAge=30;
	int xLoc = -1; int yLoc = -1;
	int xMLoc = -1; int yMLoc = -1;
	int found = false;
	int fertileAge = 16;
	int run = 0;
	int rank = repast::RepastProcess::instance()->rank();
	//std::cout<<"Fission starting"<<std::endl;
	for(int i=Fx; i>-1; i--)
	{
		for(int j = 119; j >-1; j--)
		{
			if(run == 0)
			{
				run = 1;
				j = Fy;
			}
			//std::cout<<i<<j<<std::endl;
			//std::cout<<"J"<<j<<std::endl;
			MaizeList.clear();
			agentList.clear();
			MdiscreteSpace->getObjectsAt(repast::Point<int>(i, j), MaizeList);
			discreteSpace->getObjectsAt(repast::Point<int>(i, j), agentList);
			if(MaizeList.size() == 0 && waterlocation(currentYear,i,j)==0 && agentList.size()==0)
			{
				xLoc = i;
				yLoc = j;
				//std::cout<<"Agent location found:"<<i<<j<<std::endl;
				for(int im = -10; im<=10; im++)
				{
					xMLoc = xLoc+im;
					if(xMLoc>=0&&xMLoc<80)
					{
						for(int jm = -10; jm<=10 ; jm++)
						{
							yMLoc = yLoc+jm;
							if(yMLoc>=0&&yMLoc<120)
							{
								MaizeList.clear();
								agentList.clear();
								discreteSpace->getObjectsAt(repast::Point<int>(xMLoc, yMLoc), agentList);
								MdiscreteSpace->getObjectsAt(repast::Point<int>(xMLoc, yMLoc), MaizeList);
								if(MaizeList.size()==0 && agentList.size()==0 && waterlocation(currentYear,xMLoc,yMLoc)==0)
								{
									//std::cout<<"Maize Location found: "<<xMLoc<<yMLoc<<std::endl;
									Fx = i;
									Fy = j;
									found = true;
									goto skip_l;
								}
							}
						}
					}
				}
			}
		}
	}
	skip_l:
	if(Fx<=0)
	{
		Fx = 79;
		Fy = 119;
	}
	if(found)
	{
		//Add info to simulation
		//std::cout<<"Adding agent and maizefield to map"<<std::endl;
		repast::Point<int> initialLocation(xLoc, yLoc);		//Give agent a location
		repast::Point<int> initialMaizeLocation(xMLoc, yMLoc);		//Give MaizeField a location

		repast::AgentId id(currentId, rank, 0);						//Give agent a unique ID
		repast::AgentId Maizeid(currentId, rank, 0);				//Give agent a unique ID

		id.currentRank(rank);

		int maizeLoc[2];
		maizeLoc[0] = 0;
		maizeLoc[1] = 0;

		int deathAge = gen2.next();
		int infertileAge = gen3.next();
		int initialMaize; 
		initialMaize = it->giveMaize();
		Agent* agent = new Agent(id, initialAge, fertileAge, deathAge, infertileAge, maizeLoc[0], maizeLoc[1], initialMaize); //Create new agent with defined values
		MaizeField* maizeField = new MaizeField(id, MaizeFieldData1, MaizeFieldData2,q);

		Mcontext.addAgent(maizeField);
		context.addAgent(agent);

		discreteSpace->moveTo(id, initialLocation);
		MdiscreteSpace->moveTo(Maizeid, initialMaizeLocation);
		countOfAgents ++;
		currentId ++;
	}
	//else
		//std::cout<<"Couldn't fission Agent"<<std::endl;
	//std::cout<<"Location not found"<<std::endl;
};

void AnsaziModel::printToScreen()
{
//print board to screen
	std::vector<Agent*> agentList;
	std::vector<MaizeField*> MaizeFieldList;
	std::stringstream xs,ys;

	for (int i=0; i<=boardSizey+1; i++)
	{
		for (int j=0; j<=boardSizex+1; j++)
		{
			if (i==0 || i==boardSizey+1)
				std::cout << "-";
			else if (j==0 || j==boardSizex+1)

			std::cout << "|";
			else
			{
				agentList.clear();
				MaizeFieldList.clear();

				discreteSpace->getObjectsAt(repast::Point<int>(j, i), agentList);
				MdiscreteSpace->getObjectsAt(repast::Point<int>(j, i), MaizeFieldList);
				//cout << "Total Amount of MaizeFields at end of year: "<<MaizeFieldList.size()<<endl;

				if (agentList.size() > 1 || MaizeFieldList.size() >1)
				{
					if(agentList.size() >1 )
						std::cout << agentList.size();
					else
						std::cout << MaizeFieldList.size();
				}


				if (agentList.size() == 0 && MaizeFieldList.size() == 0 && waterlocation(currentYear, j, i) == 0)
				{
						std::cout << " ";

				}
				if (MaizeFieldList.size() == 1)
				{
						std::cout << "0";

				}
				if (waterlocation(currentYear, j, i) == 1)
				{
						std::cout << "W";

				}
				if (agentList.size() == 1)
				{
						std::cout << "X";

				}



			}
		}
		std::cout << "\n";
	}
	cout << "Total Amount of Agents at end of year: "<<countOfAgents<<endl;
	cout << "Total Amount of MaizeFields at end of year: "<<MaizeFieldList.size()<<endl;
}

//reading from file functions (Niri)===========================================================
std::vector<std::vector<std::string> > readcsv(std::string filename1)
{
    std::ifstream f;
    f.open(filename1);
    if(!f.is_open()) std::cout << "ERROR " ;
    std::string rows, values;

    std::vector< std::vector<std::string> > array;  // the 2D array
    std::vector<std::string> v;                // array of values for one line only

    //std::cout << "test 2 \n" << std::endl;
    getline(f,rows);

    while ( getline(f,rows) )    // get next line in file
    {
        v.clear();
        std::stringstream ss(rows);
        while (getline(ss,values,','))  // break line into comma delimitted fields
        {
            v.push_back(values);  // add each field to the 1D array
        }

        array.push_back(v);  // add the 1D array to the 2D array
    }


    return array;

}


//output to file
void AnsaziModel::outputfile(std::vector<int> value)
{
	std::ofstream file; //output stream

	if(currentYear == 800){
		std::remove("household.csv");
	}
    file.open("household.csv",std::ios::app);
    //for(int i=0, i<(value.size()), i++)
    //{
    file <<" " << value[0] << ",";
    //}
	file.close();

}

std::vector<std::vector<double> > conversion(std::vector<std::vector<std::string> > array)
{

	std::vector< std::vector<double> > val1;  // the 2D array
    std::vector<double> h1;                // array of values for one line only
	double h;

	//val1=repast::strToInt(array);
	for(size_t i=0; i<array.size();i++)
	{
		h1.clear();
		for(size_t j =0; j<array[0].size();j++)
		{

			h=repast::strToDouble(array[i][j]);
			h1.push_back(h);
		}
		val1.push_back(h1);
	}
	return val1;
}

//Calculates if the location contains water
bool AnsaziModel::waterlocation(int year1, int xaxis1, int yaxis1)
{

    std::string xax,yax, zone; //initalise variables
    int startdateint, enddateint,typeint,existStreams=0, existAlluvium=0;
    int y=0,x=0, x1=0,y1=0;

    stringstream ss,pp;
	ss << xaxis1;
	pp << yaxis1;
	xax = ss.str();
	yax = pp.str();
    while((waterint.size()-1)!=y) //calculates the location where x and y are stores
    {
        y++;
        if(waterint[y][7] == yaxis1 && waterint[y][6] == xaxis1)
        {
            break;
        }
    }

    while(maps.size()-1 != y1 )
    {
        y1++;
        if(maps[y1][1] == yax && maps[y1][0] == xax)
        {
        	break;
        }
    }



	typeint = waterint[y][3]; //converts integer to a float
	startdateint = waterint[y][4];
	enddateint = waterint[y][5];
	zone = maps[y1][3];

    if(typeint==3)
    {
        if(startdateint<= year1 && enddateint >=year1)
        {
            return true;
        }
        else
        {
        	return false;
        }
    }
    if(typeint==2)
    {
        return true;
    }


    if(typeint==1)
    {
        if((year1>=280 && year1 <360) || (year1>=800 && year1 <930) || (year1>=1300 && year1 <1450))
        {
            existStreams=1;
        }
        else
        {
            existStreams=0;
        }

        if(((year1>=420) && (year1 <560 )) || ((year1>=630) && (year1 <680)) || ((year1>=980) && (year1 <1120)) || ((year1>=1180) && (year1 <1230)))
        {
            existAlluvium=1;
        }
        else
        {
            existAlluvium=0;
        }

        if((existAlluvium==1) && ((zone == "\"General\"") || (zone =="\"North\"") || (zone == "\"Mid\"") || (zone == "\"Kinbiko\"")))
        {
        	return true;

        }
        if((existStreams == 1) && (zone =="\"Kinbiko\""))
        {
        	return true;
        }

    }

    return false;

}

// Drought function is to find the drought index value from file PDSI
int AnsaziModel::droughtindex(int year, int xaxis, int yaxis)
{
    std::string content, general, north, mid, natural, upland, kinbiko, yax, xax; // initialise the variables
   	double generalint, northint, midint, naturalint, uplandint, kinikoint;
    int droughtindexgen=0, droughtindexnor=0, droughtindexmid=0, droughtindexnat=0, droughtindexupl=0, droughtindexkin=0;

    year=year-799;
    std::stringstream ss,pp;
	ss << xaxis;
	pp << yaxis;
	xax = ss.str();
	yax = pp.str();

    generalint = pdsiint[year][2]; //stores the value of the PDSI
    northint = pdsiint[year][4];
    midint = pdsiint[year][6];
    naturalint = pdsiint[year][8];
    uplandint = pdsiint[year][10];
    kinikoint = pdsiint[year][12];

    //Calculates the drought index
  /*  if(generalint <=-3){
        droughtindexgen=514;
    }
    else if(-3< generalint <=-1){
        droughtindexgen=599;
    }
    else if(-1< generalint <=1){
        droughtindexgen=684;
    }
    else if(1< generalint <=3){
        droughtindexgen=824;
    }
    else if(3< generalint){
        droughtindexgen=961;
    }

    if(northint<=-3){
        droughtindexnor=617;
    }
    else if(-3< northint<=-1){
        droughtindexnor=719;
    }
    else if(-1< northint <=1){
        droughtindexnor=821;
    }
    else if(1< northint<=3){
        droughtindexnor=988;
    }
    else if(3< northint){
        droughtindexnor=1153;
    }

    if(midint <=-3){
            droughtindexmid=617;
    }
    else if(-3<midint <=-1){
            droughtindexmid=719;
    }
    else if(-1<midint <=1){
            droughtindexmid=821;
    }
    else if(1<midint <=3){
            droughtindexmid=988;
    }
    else if(3<midint){
            droughtindexmid=1153;
    }

    if(naturalint<=-3){
            droughtindexnat=617;
    }
    else if(-3<naturalint<=-1){
            droughtindexnat=719; //check value again
    }
    else if(-1<naturalint<=1){
            droughtindexnat=821;
    }
    else if(1<naturalint<=3){
            droughtindexnat=988;
    }
    else if(3<naturalint){
            droughtindexnat=1153;
    }

    if(uplandint<=-3)
    {
        droughtindexupl=411;
    }
    else if(-3<uplandint<=-1)
    {
        droughtindexupl=479;
    }
    else if(-1<uplandint<=1)
    {
        droughtindexupl=547;
    }
    else if(1<uplandint<=3)
    {
        droughtindexupl=659;
    }
     else if(3<uplandint)
    {
        droughtindexupl=769;
    }

    if(kinikoint<=-3){
         droughtindexkin=617;
    }
    else if(-3<kinikoint<=-1){
         droughtindexkin=719;
    }
    else if(-1<kinikoint<=1){
         droughtindexkin=821;
    }
    else if(1<kinikoint<=3){
         droughtindexkin=988;
    }
    else if(3<kinikoint){
         droughtindexkin=1153;
    }
*/
    //identifies the the location of the x and y in the vecor
    int x=0,y=0;

        while(maps[y][1] != yax )
        {
            y++;
            if(maps[y][1] == yax && maps[y][0] == xax)
            {
             break;
  			}
            else if(maps[y][1] == yax && maps[y][0] != xax)
            {
                y++;
            }

        }

    //Calculates what to return
    if(maps[y][3]== "\"General\"" )
    {
        return generalint;
    }
    else if(maps[y][3]== "\"Kinbiko\""  )
    {
        return kinikoint;
    }
    else if(maps[y][3]== "\"Mid\"" )
    {
        return midint;
    }
    else if(maps[y][3]== "\"Mid Dunes\"" )
    {
        return midint;
    }
    else if(maps[y][3] == "\"Natural\"")
    {
        return naturalint;
    }
    else if((maps[y][3]) == "\"North\"" )
    {
        return northint;
    }
    else if(maps[y][3]== "\"North Dunes\"")
    {
        return northint;
    }
    else if(maps[y][3]== "\"Uplands\"" )
    {
        return uplandint;
    }
    else if(maps[y][3]== "\"Empty\"" )
    {
        return 0;
    }
}
