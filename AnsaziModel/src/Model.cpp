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
int droughtindex(int year, std::string xaxis, std::string yaxis);

AnsaziModel::AnsaziModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm): context(comm), Mcontext(comm)
{
	props = new repast::Properties(propsFile, argc, argv, comm);								//Initialise Repast property files 

	stopAt = repast::strToInt(props->getProperty("stop.at"));									//Fetch the stop at value from model.props file
	countOfAgents = repast::strToInt(props->getProperty("count.of.agents"));					//Fetch the count of agents from the model.props file 
	boardSizex = repast::strToInt(props->getProperty("board.sizex"));								//Fetch the board size property from model.props 
	boardSizey = repast::strToInt(props->getProperty("board.sizey"));								//Fetch the board size property from model.props 
	currentYear = repast::strToInt(props->getProperty("startYear"));							//Set the start year for the simulation
	deathAge = repast::strToInt(props->getProperty("deathAge"));								//Get the death age 

    MaizeFieldData1 = 1;																		//repast::strToFloat(props->getProperty("harvest.adjustment.level"));
    MaizeFieldData2 = 0.1;																		//repast::strToFloat(props->getProperty("sigmaahv"));
	

	currentId = countOfAgents; 

	initializeRandom(*props, comm);																	

	//Init the shared space
	repast::Point<double> origin(1,1);															//Shared Space Starts at (x,y)
	repast::Point<double> extent(boardSizex+1,boardSizey+1);										//Shared Space stops at (x,y) 
	repast::GridDimensions gd(origin, extent);													//Shared Space defined as (start,stop)
	
	std::vector<int> processDims;
	processDims.push_back(1);																	//How the process is split across cores 1 by 1 currently 
	processDims.push_back(1);																	
	
	discreteSpace = new repast::SharedDiscreteSpace<Agent, repast::StrictBorders, repast::SimpleAdder<Agent> >("AgentDiscreteSpace", gd, processDims, 0, comm); //Initialise the discrete space, "Strict border may change to wrap around borders"
	MdiscreteSpace = new repast::SharedDiscreteSpace<MaizeField, repast::StrictBorders, repast::SimpleAdder<MaizeField> >("MaizeFieldDiscreteSpace", gd, processDims, 0, comm); //Initialise the discrete space, "Strict border may change to wrap around borders"
	
	context.addProjection(discreteSpace); 	
	Mcontext.addProjection(MdiscreteSpace);

	//Load predifined files 
	maps = readcsv("src/map.csv");																//Save array to model
	hydro = readcsv("src/hydro.csv");	
	water = readcsv("src/water.csv");
	pdsi = readcsv("src/pdsi.csv");
}

AnsaziModel::~AnsaziModel()
{
	delete props;
	//delete agent;
	//delete maizeField;
}

void AnsaziModel::doPerTick()
{

	cout << "Start of tick ------------------------------------------" << endl; 
	cout << "Current Year: " << currentYear << endl; 

	removeAgent();
	fissionProcess(); 
	if(currentYear==800)
	{
		printToScreen();
	}
	cout << "=End of tick --------------------------------------------" << endl;
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
	repast::IntUniformGenerator gen4 = repast::Random::instance()->createUniIntGenerator(1, boardSizex);
	repast::IntUniformGenerator gen5 = repast::Random::instance()->createUniIntGenerator(1, boardSizey);
	std::cout<<"=====================================Test 1 ====================="<<std::endl;
	std::cout<<"----------------------------------Initialisng Agents-------------"<<std::endl;

	for(int i=0 ;i<countOfAgents;i++)//13 agents initially then one made ourselves for testing
	{      
		std::cout<<"NewAgent----------------------"<<std::endl;   
		std::vector<Agent*> agentList;
		std::vector<MaizeField*> MaizeFieldList;

		//Load random values to initilaise agents
		int initialAge=gen2.next();
		int infertileAge=30;
		int fertileAge = 16;

		int xLoc = gen4.next(); int yLoc = gen5.next();
		int xMLoc = gen4.next(); int yMLoc = gen5.next();

		repast::Point<int> initialLocation(xLoc, yLoc);		//Give agent a location
		repast::Point<int> initialMaizeLocation(xMLoc, yMLoc);		//Give MaizeField a location

		repast::AgentId Maizeid(i, rank, 0);						//Give agent a unique ID
		repast::AgentId id(i, rank, 0);						//Give agent a unique ID
		
		id.currentRank(rank);
		Maizeid.currentRank(rank);

		Agent* agent = new Agent(id, initialAge, fertileAge, deathAge, infertileAge,xMLoc,yMLoc); //Create new agent with defined values
		MaizeField* maizeField = new MaizeField(Maizeid, MaizeFieldData1, MaizeFieldData2);    //MaizeFieldData
		
		context.addAgent(agent);
		Mcontext.addAgent(maizeField);
		std::cout<<"Agent ID: "<<id<<std::endl;
		std::cout<<"Agent Loc: "<<initialLocation<<std::endl; 
		std::cout<<"Death Age: "<<deathAge<<std::endl; 
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
	Mcontext.selectAgents(repast::SharedContext<MaizeField>::LOCAL, countOfAgents, maizeFields);

	std::vector<Agent*>::iterator it = agents.begin();
	std::vector<MaizeField*>::iterator Mit = maizeFields.begin();

	it = agents.begin();
	Mit = maizeFields.begin();
	std::cout<<"================================= TEST 5 ================================="<<std::endl;
	std::cout<<"================================= TEST 4 ================================="<<std::endl;
	while(it != agents.end()&& Mit != maizeFields.end())
	{
		//(*it) -> printAttributes();  
		if ((*it)->checkDeath())
		{
			//std::cout<<"The Agent ID is: "<<(*it) -> getId().id()<<std::endl;
			(*it) -> printAttributes(); 	
			std::cout <<"!Deleted Curent Age as death age met!-----------------------"<<std::endl;

			int maizeLoc[2];

			maizeLoc[0] = (*it) -> getMaizeLocX();
			maizeLoc[1] = (*it) -> getMaizeLocY();
			
			//MdiscreteSpace->getObjectsAt(repast::Point<int>(maizeLoc[0], maizeLoc[1]), maizeFields);
			
			//std::cout <<"!Deleted Curent MaizeField"<< (*Mit) -> getId().id()<<" as death age met!"<<std::endl; 
			//std::cout <<"Agent was in location : "<<
			//int id = (*it) -> getId().id(); 
    		repast::RepastProcess::instance()->agentRemoved((*it) -> getId());
    		repast::RepastProcess::instance()->agentRemoved((*Mit) -> getId());

    		context.removeAgent((*it) -> getId());
			Mcontext.removeAgent((*Mit) -> getId());
    		countOfAgents --;
    	}
		else
		{
			(*Mit)->getAttributes(MaizeFieldData1, MaizeFieldData2);
			(*it)->Maizeloc2str();
			x = 1000; //droughtindex(currentYear,(*it)->Xval,(*it)->Yval);
			(*Mit)->MaizeProduction(x);
			(*it)->updateMaizeStock((*Mit)->currentYield);
			std::cout<<"This is The Expected MaizeField Yield: "<<(*it)->expectedYield<<std::endl;
			if((*it)->checkMaize() == 1)
			{
				std::cout<<"This is The Expected MaizeField Yield < 800: "<<(*it)->expectedYield<<std::endl;

				repast::RepastProcess::instance()->agentRemoved((*it) -> getId());
    			repast::RepastProcess::instance()->agentRemoved((*Mit) -> getId());

    			context.removeAgent((*it) -> getId());
				Mcontext.removeAgent((*Mit) -> getId());
				
				//std::cout<<<<(*Mit) -> getId()<<std::endl;
    			countOfAgents --;
			}
		}
		it++;
		Mit++;
	}
}

void AnsaziModel::fissionProcess()
{
	std::vector<Agent*> agents;
	std::vector<MaizeField*> MaizeFieldList;

	int i=0; 
	context.selectAgents(repast::SharedContext<Agent>::LOCAL, countOfAgents, agents);
	std::vector<Agent*>::iterator it = agents.begin();
	it = agents.begin();
	//std::cout<<"----------------Fission Process----------------------------"<<std::endl; 
	while(it != agents.end())
	{
		//(*it) -> printAttributes();  
		if ((*it)->fissionReady())
		{
    		//std::cout<<"!Fission occurring Adding an agent!---------------------------"<<std::endl; 
			//std::vector<Agent*> agentList;
			
			//Load random values to initilaise agents
			repast::IntUniformGenerator gen4 = repast::Random::instance()->createUniIntGenerator(1, boardSizex);
			repast::IntUniformGenerator gen5 = repast::Random::instance()->createUniIntGenerator(1, boardSizey);

			int initialAge= 0;
			int infertileAge=30;
			int xLoc = gen4.next(); int yLoc = gen5.next();
			int xMLoc = gen4.next(); int yMLoc = gen5.next();
			int fertileAge = 16;
			int rank = repast::RepastProcess::instance()->rank();
			
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
			MaizeField* maizeField = new MaizeField(Maizeid, MaizeFieldData1, MaizeFieldData2);  

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

    // print out what was read in------------------------------------------------
    /*if(filename1=="src/water.csv")
	{
	    //std::cout << "test 2 \n" << endl; 
	    for (size_t x=0; x<array.size(); ++x)
	    {
	        for (size_t y=0; y<array[x].size(); ++y)
	        {
	            std::cout << array[x][y] << "|"; // (separate fields by |)
	        }
	        std::cout << "\n";
	    }
	    cout << array[1][1];
	}*/
    return array;
}

void AnsaziModel::printToScreen() 
{
	//print board to screen
	std::vector<Agent*> agentList;
	std::vector<MaizeField*> MaizeFieldList;
	std::cout<<"========================== TEST 2 =================================== "<<endl;
	std::cout<<"========================== TEST 5 ================================="<<endl;
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
				if (agentList.size() > 1) 
				{
					std::cout << agentList.size();
				}
				else if (agentList.size() == 0)
				{
					if(MaizeFieldList.size() == 0)
						std::cout << " ";
				}
				else if(agentList.size() != 0)
				{
					std::cout << "X";
				}
				if (MaizeFieldList.size() > 1) 
				{
					std::cout << MaizeFieldList.size();
				}

				else if(MaizeFieldList.size() != 0) 
				{
					std::cout << "O";
				}
			}
		}
		std::cout << std::endl;
	}
	outputfile(countOfAgents);
	cout << "Total Amount of Agents at end of year: "<<countOfAgents<<endl;
	//cout << "Total Amount of MaizeFields at end of year: "<<MaizeFieldList.size()<<endl;
}

void AnsaziModel::outputfile(int value)
{
	std::ofstream file;
	if(currentYear == 800){
		std::remove("household.csv"); 
	}
    file.open("household.csv",std::ios::app);
    file << value << std::endl;
    file.close(); 
}

int droughtindex(int year, std::string xaxis, std::string yaxis)
{
    std::vector< std::vector<std::string> > pdsi, maps;
    pdsi = readcsv("src/pdsi.csv");
    //std::cout << "it went throught the first stage"<<year<<xaxis<<yaxis<<std::endl;
    year =year - 799;
    //std::cout << year << std::endl; 
    //std::cout << pdsi[year][0]<<std::endl;
    //std::cout << pdsi[2][1];
    std::string content;
    std::string general;
    std::string north;
    std::string mid;
    std::string natural;
    std::string upland;
    std::string kinbiko;
    //std::cout << "Its here now sir "<<std::endl;

    general = pdsi[year][1];
    north = pdsi[year][2];
    mid = pdsi[year][3];
    natural = pdsi[year][4];
    upland = pdsi[year][5];
    kinbiko = pdsi[year][6];

    float generalint, northint, midint, naturalint, uplandint, kinikoint;
    //std::cout << "Its here now sir "<<std::endl;

    generalint = atof(general.c_str());
    northint = atof(north.c_str());
    midint = atof(mid.c_str());
    naturalint = atof(natural.c_str());
    uplandint = atof(upland.c_str());
    kinikoint = atof(kinbiko.c_str());

    int droughtindexgen=0;
    int droughtindexnor=0;
    int droughtindexmid=0;
    int droughtindexnat=0;
    int droughtindexupl=0;
    int droughtindexkin=0;

    //using namespace std;
    //std::cout << "Its here now sir 3"<<std::endl;

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
    int x=0,y=0;
    maps = readcsv("src/map.csv");
        while(maps[y][1] != yaxis )
        {
            y++;
            if(maps[y][1] == yaxis && maps[y][0] == xaxis)
            {
                break;
            }
            else if(maps[y][1] == yaxis && maps[y][0] != xaxis)
            {
                y++;
            }

        }
    //std::cout << " \n this is y now: " << y <<std::endl << std::endl;
    //std::cout << maps[y][3]<<std::endl << std::endl;

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
        int droughtindexnull = 0;
        return droughtindexnull;
    }
}