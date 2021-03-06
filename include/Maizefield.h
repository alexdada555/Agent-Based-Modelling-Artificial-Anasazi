#ifndef __MAIZEFIELD_H__
#define __MAIZEFIELD_H__

#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"
#include "repast_hpc/AgentId.h"

#include <vector>
#include <boost/mpi.hpp>

class MaizeField
{   
    private:
        repast::Properties* Maizeprops;
        repast::AgentId MaizeFieldID;
        
        int agentType;
        bool isSatisfied;

        int y = 0;
        double q;   
        float Ha;

        int H0 = 0; // H0 = BY * (1 + n(0,sigmaahv)) = houshold harvest
        float BY = 0; // BY = y*q*Ha = bass yield     
        float sigmaahv = 0;
        int data = 0; 
        int data1;
        int data2;
        double pastsig = 0;
        double sig;

    public:
        int currentYield = 0;
        int droughtIndex = 0;
        bool tick;

        virtual repast::AgentId& getId() { return MaizeFieldID; }
        virtual const repast::AgentId& getId() const { return MaizeFieldID; }

        /* Getters specific to this kind of Agent */
        int getType() { return agentType; }
        bool getSatisfiedStatus() { return isSatisfied; }

        std::vector<int> maizeLoc;
        void updateStatus(repast::SharedContext<MaizeField>* context,
              repast::SharedDiscreteSpace<MaizeField, repast::StrictBorders, repast::SimpleAdder<MaizeField> >* space);
        void move(repast::SharedDiscreteSpace<MaizeField, repast::StrictBorders, repast::SimpleAdder<MaizeField> >* space); 
        
        MaizeField(repast::AgentId MaizeFieldID,float data1,float data2,double q);
        ~MaizeField();
        void getAttributes(float data1,float data2);
        void MaizeProduction(int yieldFromFile);
        //void moveMaize(repast::AgentId MaizeID,std::vector<int> newMaizeLoc,repast::SharedDiscreteSpace<MaizeField, repast::WrapAroundBorders, repast::SimpleAdder<MaizeField> >* space);
};
#endif