#ifndef SCHELLING_AGENT
#define SCHELLING_AGENT

#include "repast_hpc/AgentId.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include <string>

class Agent
{
	
private:
	repast::AgentId agentId;
    //repast::Properties* props;
    //repast::SharedContext<Agent> context;
    //std::vector<Agent*> agents;

    bool isSatisfied;
	double threshold;

	int agentType;
    int deathAge;
    int maizeStock;
    int maizeOwnership;
    int baseYield;
    int infertileAge;
    int fertileAge;
    int fissionIndex;

    int maizeLocX;
    int maizeLocY;

    int previousYield[3]={2000,0,0};
    int houseLocation[2];

    int tick=0; 
    
public:
	Agent(repast::AgentId agentId, int currentAge, int fertileAge, int deathAge, int infertileAge, int maizeLocX,int maizeLocY); //for init of agents 
	~Agent();
	
	/* Required Getters */
	virtual repast::AgentId& getId() { return agentId; }
	virtual const repast::AgentId& getId() const { return agentId; }
	
	/* Getters specific to this kind of Agent */
	int getType() { return agentType; }
	bool getSatisfiedStatus() { return isSatisfied; }
	
	/* Actions */
	void updateStatus(repast::SharedContext<Agent>* context,
			  repast::SharedDiscreteSpace<Agent, repast::StrictBorders, repast::SimpleAdder<Agent> >* space);
	void move(repast::SharedDiscreteSpace<Agent, repast::StrictBorders, repast::SimpleAdder<Agent> >* space);	

    std::string Xval;
	std::string Yval;

    int maizeID;
    int expectedYield = 0;
    
	//Program functions
	bool checkDeath();
    bool checkMaize();
    bool fissionReady(); 

	bool checkMove;
    int currentAge;
    int householdID;

    int getAttribute(std::string Attribute);
    int getMaizeLocX();
    int getMaizeLocY();
    
    void printAttributes();
    void initialiseInfo();
    void checkResidence();
    void killResidence();
    void fissionProcess();
    void updateMaizeStock(int Yield);
    void updateAge();
    void Maizeloc2str();
    void move(); 
};

#endif
