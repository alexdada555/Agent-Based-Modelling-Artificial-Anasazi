#include "Maizefield.h"

#include "repast_hpc/Properties.h"
#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedDiscreteSpace.h"
#include "repast_hpc/GridComponents.h"
#include "repast_hpc/Random.h"

#include <string>
#include <vector>
#include <boost/mpi.hpp>
#include <stdio.h>

MaizeField::MaizeField(repast::AgentId MaizeFieldID,float data1,float data2):MaizeFieldID(MaizeFieldID), data1(data1), data2(data2)
{
    // initialise maizefield
    getAttributes(data1,data2);   // read in constants from properties file
}

void MaizeField::getAttributes(float data1,float data2)
{
    // reads in the attributes for yield calculations from prop files
    Ha = data1;
    sigmaahv = data2;
}

void MaizeField::MaizeProduction(int yieldFromFile)
{
    repast::IntUniformGenerator gen1 = repast::Random::instance()->createUniIntGenerator(0, 1); // initialise random number generator
    repast::IntUniformGenerator gen2 = repast::Random::instance()->createUniIntGenerator(0, sigmaahv); // initialise random number generator
    q = gen1.next();

    y = yieldFromFile;
    BY = y*q*Ha; // calculate base yield
    H0 = BY * (1 + gen2.next()); // calculate household harvest
    currentYield = H0; //set current yield to household yield.
}

/*void MaizeField::moveMaize(repast::AgentId MaizeID,std::vector<int> newMaizeLoc,repast::SharedDiscreteSpace* space)
{
    //get agent location from the space
    space->getLocation(MazeFieldID, MazeLoc);

    if (MaizeID == MaizeFieldID)
    {
        MazeLoc.clear();
        MaizeLoc = newMaizeLoc;
        space->moveTo(MaizeID,newMaizeLoc);
    }
    return (0);
}*/

MaizeField::~MaizeField()
{
    //delete Maizeprops;
}