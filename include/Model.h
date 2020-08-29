#ifndef MODEL
#define MODEL

#include <boost/mpi.hpp>
#include "repast_hpc/Schedule.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"

#include "Agent.h"
#include "Maizefield.h"

class AnsaziModel
{
private:
	int stopAt;
	int countOfAgents;
	int boardSizex;
	int boardSizey;
	int currentYear;
	int currentId;
	int x;
	int mindeathAge; 
	int maxdeathAge; 
	int minFertileAge; 
	int maxFertileAge; 
	double fissionProb; 
	bool watertest;
	double q;
	//int Mx;
	//int My; 
	
	//Repast libraries ===========================================================
	repast::Properties* props;
	repast::SharedContext<Agent> context;
	repast::SharedContext<MaizeField> Mcontext;
	repast::SharedDiscreteSpace<Agent, repast::StrictBorders, repast::SimpleAdder<Agent> >* discreteSpace;
	repast::SharedDiscreteSpace<MaizeField, repast::StrictBorders, repast::SimpleAdder<MaizeField> >* MdiscreteSpace;

	void printToScreen(); 
	
public:
	double MaizeFieldData1;
	double MaizeFieldData2;
	double sigmaahvNew; 
	bool sametick = 0;
	
	AnsaziModel(std::string propsFile, int argc, char** argv, boost::mpi::communicator* comm);
	~AnsaziModel();
	void initAgents();
	void initSchedule(repast::ScheduleRunner& runner);
	void doPerTick();
	void removeAgent();
	void updateDeath();
	void fissionProcess(Agent* it); 
	void outputfile(int value);
	bool move(MaizeField* Mit, Agent* it); 
	bool waterlocation(int year1, int xaxis1, int yaxis1);
	int droughtindex(int year, int xaxis, int yaxis);
	std::vector<std::vector<std::string> > maps;
	std::vector<std::vector<std::string> > water;
	std::vector<std::vector<std::string> > hydro;
	std::vector<std::vector<std::string> > pdsi;
	std::vector<std::vector<double> > waterint;
	std::vector<std::vector<double> > hydroint;
	std::vector<std::vector<double> > pdsiint;
	std::vector<std::vector<double> > mapsint;

};

#endif
