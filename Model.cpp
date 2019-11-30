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

int Mx; 
int My; 
int fullCheck; 


AnsaziModel::AnsaziModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm), Mcontext(comm)
{
	//Load predifined files 	
	maps = readcsv("src/map.csv");																//Save array to model
	hydro = readcsv("src/hydro.csv");	
	water = readcsv("src/water.csv");
	pdsi = readcsv("src/pdsi.csv");

	props = new repast::Properties(propsFile, argc, argv, comm);								//Initialise Repast property files 

	stopAt = repast::strToInt(props->getProperty("stop.at"));									//Fetch the stop at value from model.props file
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));					//Fetch the count of agents from the model.props file 
	boardSizex = repast::strToInt(props->getProperty("board.sizex"));								//Fetch the board size property from model.props 
	boardSizey = repast::strToInt(props->getProperty("board.sizey"));								//Fetch the board size property from model.props 
	currentYear = repast::strToInt(props->getProperty("startYear"));							//Set the start year for the simulation
	deathAge = repast::strToInt(props->getProperty("deathAge"));								//Get the death age 

    MaizeFieldData1 = 1;																		//repast::strToFloat(props->getProperty("harvest.adjustment.level"));
    MaizeFieldData2 = 0.1;																		//repast::strToFloat(props->getProperty("sigmaahv"));
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
	std::vector<int> countagent;
	//cout << "Start of tick ------------------------------------------" << endl; 
	//cout << "Current Year: " << currentYear << endl; 
	fullCheck = 0; 
	if(countOfAgents!= 0) 
	{
		removeAgent();
		fissionProcess();
	}
	//else 
		//std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!No Agents!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< std::endl;
	Mx = 0; 
	My= 0; 
	std::cout<< currentYear << "	:current year \n";
	if(currentYear == 800 || currentYear ==(800+stopAt-1))
		printToScreen();

	countagent.push_back(countOfAgents);
	if(currentYear ==(800+stopAt-1))
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

	int currentmin=0;                                                                                                                          															
	int currentmax=30;
	int fertilemin=16;
	int fertilemax=30;
	

	repast::IntUniformGenerator gen2 = repast::Random::instance()->createUniIntGenerator(currentmin, currentmax);
	repast::IntUniformGenerator gen3 = repast::Random::instance()->createUniIntGenerator(fertilemin, fertilemax);
	repast::IntUniformGenerator gen4 = repast::Random::instance()->createUniIntGenerator(0, boardSizex-1);
	repast::IntUniformGenerator gen5 = repast::Random::instance()->createUniIntGenerator(0, boardSizey-1);
	//std::cout<<"=====================================Test 1 ====================="<<std::endl;
	//std::cout<<"----------------------------------Initialisng Agents-------------"<<std::endl;

	for(int i=0 ;i<countOfAgents;i++)//13 agents initially then one made ourselves for testing
	{      
		//std::cout<<"NewAgent----------------------"<<std::endl;   
		std::vector<Agent*> agentList;
		std::vector<MaizeField*> MaizeFieldList;
		//Load random values to initilaise agents
		int initialAge=gen2.next();
		int infertileAge=30;
		int fertileAge = 16;
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

		Agent* agent = new Agent(id, initialAge, fertileAge, deathAge, infertileAge,xMLoc,yMLoc); //Create new agent with defined values
		MaizeField* maizeField = new MaizeField(id, MaizeFieldData1, MaizeFieldData2);    //MaizeFieldData
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
    	}
		else
		{
			//std::cout<<"Changing maizefield attributes"<<std::endl; 
			(*Mit)->getAttributes(MaizeFieldData1, MaizeFieldData2);
			(*it)->Maizeloc2str();
			x = droughtindex(currentYear,(*it)->Xval,(*it)->Yval);
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
	    		}
			}
		}
		it++;
	}
}                 						

bool AnsaziModel::move(MaizeField* Mit, Agent* it)
{
	std::vector<MaizeField*> MaizeFieldList;
	std::string xx,yy; 
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
			xx = std::to_string(i);
			yy = std::to_string(j);
			if(agentList.size() == 0 && MaizeFieldList.size() == 0 && waterlocation(currentYear,xx, yy) == 0) //done need to move to closest agent 
			{
				x = droughtindex(currentYear,std::to_string(i),std::to_string(j));
				Mit->MaizeProduction(x);
				int yieldcurrent = Mit->currentYield;
				if(yieldcurrent > 800 )
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
							xx = std::to_string(xLoc);
							yy = std::to_string(yLoc);
							agentList.clear(); //
							MaizeFieldList.clear();
							discreteSpace->getObjectsAt(repast::Point<int>(xLoc, yLoc), agentList); 
							MdiscreteSpace->getObjectsAt(repast::Point<int>(xLoc, yLoc), MaizeFieldList);
							//std::cout<<"Xloc:"<<xLoc<<" Yloc:"<<yLoc<<std::endl; 
							//Convert i and j

							if(MaizeFieldList.size()==0 && waterlocation(currentYear, xx,yy) ==0 && agentList.size() ==0)
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
			repast::Point<int> agentLocation(gXloc[1], gYloc[1]);
			MdiscreteSpace->moveTo(it -> getId(), agentLocation);
			return false; 
		}
	}
	//std::cout<<"done"<<std::endl;
}

void AnsaziModel::fissionProcess()
{
	//Initialise vectors 
	std::vector<Agent*> agents;
	std::vector<Agent*> agentList;	
	std::vector<MaizeField*> MaizeFieldList;
	std::vector<MaizeField*> MaizeList;
	std::string xx,yy;


	int i=0; 
	context.selectAgents(repast::SharedContext<Agent>::LOCAL, countOfAgents, agents);	//Stores an array with all the agents on the map in 
	std::vector<Agent*>::iterator it = agents.begin();									//Makes the vector a pointer and allows iteration 
	it = agents.begin();																//Make "it" the statrting element

	//std::cout<<"----------------Fission Process----------------------------"<<std::endl; 
	while(it != agents.end())															//Loops through all the agents in list 
	{
		//std::cout<<"Cant create agent"<<std::endl; 
		//(*it) -> printAttributes();  
		if ((*it)->fissionReady())
		{

			repast::IntUniformGenerator gen4 = repast::Random::instance()->createUniIntGenerator(0, boardSizex-1);
			repast::IntUniformGenerator gen5 = repast::Random::instance()->createUniIntGenerator(0, boardSizey-1);
			int initialAge= 0;
			int infertileAge=30;
			int xLoc = gen4.next(); int yLoc = gen5.next();
			int xMLoc = gen4.next(); int yMLoc = gen5.next();
			int fertileAge = 16;
			int rank = repast::RepastProcess::instance()->rank();
				
			for(int i=79; i>-1; i--)
			{
				for(int j = 119; j >-1; j--)
				{
					//std::cout<<i<<j<<std::endl;
					xx = std::to_string(i);
					yy = std::to_string(j);
					MaizeList.clear();
					MdiscreteSpace->getObjectsAt(repast::Point<int>(i, j), MaizeList);
					if(MaizeFieldList.size() == 0 && waterlocation(currentYear,xx,yy)==0)
					{
						xLoc = i;
						yLoc = j; 
						goto skip_loop;
	
					}	
				}
			}
			skip_loop: 
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
							xx = std::to_string(xMLoc);
							yy = std::to_string(yMLoc);
							MaizeList.clear();
							agentList.clear();
							discreteSpace->getObjectsAt(repast::Point<int>(xMLoc, yMLoc), agentList);
							MdiscreteSpace->getObjectsAt(repast::Point<int>(xMLoc, yMLoc), MaizeList);
							if(MaizeList.size()==0&&agentList.size()==0 &&waterlocation(currentYear,xx,yy)==0)
							{
								goto skip_l;
							}
						}
					}
				}
			}
			skip_l:
			//Add info to simulation
			repast::Point<int> initialLocation(xLoc, yLoc);		//Give agent a location
			repast::Point<int> initialMaizeLocation(xMLoc, yMLoc);		//Give MaizeField a location

			repast::AgentId id(currentId, rank, 0);						//Give agent a unique ID
			repast::AgentId Maizeid(currentId, rank, 0);				//Give agent a unique ID

			id.currentRank(rank);

			int maizeLoc[2];
			maizeLoc[0] = (*it) -> getMaizeLocX();
			maizeLoc[1] = (*it) -> getMaizeLocY();

			Agent* agent = new Agent(id, initialAge, fertileAge, deathAge, infertileAge, maizeLoc[0], maizeLoc[1]); //Create new agent with defined values
			MaizeField* maizeField = new MaizeField(id, MaizeFieldData1, MaizeFieldData2);  

			Mcontext.addAgent(maizeField);
			context.addAgent(agent);

			discreteSpace->moveTo(id, initialLocation);
			MdiscreteSpace->moveTo(Maizeid, initialMaizeLocation);
			countOfAgents ++; 
			currentId ++;
    	}
    	//else
    		//std::cout<<"Agent doesn't want to fission"<<std::endl;	
		it++;
		i ++; 
	}
};

void AnsaziModel::printToScreen() 
{
//print board to screen
	std::vector<Agent*> agentList;
	std::vector<MaizeField*> MaizeFieldList;
	std::stringstream xs,ys;
	std::string xx,yy; 
 


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
				xx = std::to_string(i);
				yy = std::to_string(j);
				agentList.clear();
				MaizeFieldList.clear();

				discreteSpace->getObjectsAt(repast::Point<int>(j, i), agentList);
				MdiscreteSpace->getObjectsAt(repast::Point<int>(j, i), MaizeFieldList);
				//cout << "Total Amount of MaizeFields at end of year: "<<MaizeFieldList.size()<<endl;
			
				xx = std::to_string(j);
				yy = std::to_string(i);
				if (agentList.size() > 1 || MaizeFieldList.size() >1)
				{
					if(agentList.size() >1 )
						std::cout << agentList.size();
					else
						std::cout << MaizeFieldList.size();
				}
				

				if (agentList.size() == 0 && MaizeFieldList.size() == 0 && waterlocation(currentYear, xx, yy) == 0)
				{
						std::cout << " ";

				}
				if (MaizeFieldList.size() == 1)
				{
						std::cout << "0";

				}
				if (waterlocation(currentYear, xx, yy) == 1)
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
    	file << currentYear<<" " << value[0] << std::endl;
    //}
	file.close(); 
    
}    

//Calculates if the location contains water
bool AnsaziModel::waterlocation(int year1, std::string xaxis1, std::string yaxis1)
{
    
    std::string xax,yax, zone; //initalise variables 
    int startdateint, enddateint,typeint,existStreams=0, existAlluvium=0;
    int y=0,x=0, x1=0,y1=0;

    xax = xaxis1.c_str(); //concades string
    yax = yaxis1.c_str();


    while((water.size()-1)!=y) //calculates the location where x and y are stores 
    {
        y++;
        if(water[y][7] == yax && water[y][6] == xax)
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



    typeint = atoi(water[y][3].c_str()); //converts integer to a float
    startdateint = atoi(water[y][4].c_str());
    enddateint = atoi(water[y][5].c_str());
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
int AnsaziModel::droughtindex(int year, std::string xaxis, std::string yaxis)
{
    year =year - 799;
    std::string content, general, north, mid, natural, upland, kinbiko, yax, xax; // initialise the variables 
   	float generalint, northint, midint, naturalint, uplandint, kinikoint;
    int droughtindexgen=0, droughtindexnor=0, droughtindexmid=0, droughtindexnat=0, droughtindexupl=0, droughtindexkin=0;

    yax = yaxis.c_str(); //concades the yaxis 
    xax = xaxis.c_str(); 

    general = pdsi[year][1]; //stores the value of the PDSI
    north = pdsi[year][2];
    mid = pdsi[year][3];
    natural = pdsi[year][4];
    upland = pdsi[year][5];
    kinbiko = pdsi[year][6];

   
    generalint = atof(general.c_str()); //converts the string into a float
    northint = atof(north.c_str());
    midint = atof(mid.c_str());
    naturalint = atof(natural.c_str());
    uplandint = atof(upland.c_str());
    kinikoint = atof(kinbiko.c_str());

    //Calculates the drought index
    if(generalint <=-3){
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
        return droughtindexgen;
    }
    else if(maps[y][3]== "\"Kinbiko\""  )
    {
        return droughtindexkin;
    }
    else if(maps[y][3]== "\"Mid\"" )
    {
        return droughtindexmid;
    }
    else if(maps[y][3]== "\"Mid Dunes\"" )
    {
        return droughtindexmid;
    }
    else if(maps[y][3] == "\"Natural\"")
    {
        return droughtindexnat;
    }
    else if((maps[y][3]) == "\"North\"" )
    {
        return droughtindexnor;
    }
    else if(maps[y][3]== "\"North Dunes\"")
    {
        return droughtindexnor;
    }
    else if(maps[y][3]== "\"Uplands\"" )
    {
        return droughtindexupl;
    }
    else if(maps[y][3]== "\"Empty\"" )
    {
        return 0;
    }
}

