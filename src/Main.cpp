/* Demo_03_Main.cpp */

#include <boost/mpi.hpp>
#include "repast_hpc/RepastProcess.h"

#include "Model.h"


int main(int argc, char** argv){
	
	std::string configFile = argv[1]; // The name of the configuration file is Arg 1
	std::string propsFile  = argv[2]; // The name of the properties file is Arg 2
	//std::cout<<"ggggggg";
	boost::mpi::environment env(argc, argv);				//Creates MPI Enviroment 
	boost::mpi::communicator world;							//Creates a world 
	repast::RepastProcess::init(configFile);				//Initialises repast Process 
		
	AnsaziModel* model = new AnsaziModel(propsFile, argc, argv, &world);			//Initialises the model 
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();		//Creates the schedule runner 
	
	model->initAgents();															//Initialise Agents 
	model->initSchedule(runner);													//Initialise Scheduler 
	
	runner.run();																	//Run model 
	delete model;
	
	repast::RepastProcess::instance()->done();										//Shut of Simulation 
	
}