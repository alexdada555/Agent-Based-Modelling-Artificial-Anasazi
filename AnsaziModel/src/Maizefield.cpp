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

MaizeField::MaizeField(repast::AgentId MaizeFieldID,float data1,float data2, double q):MaizeFieldID(MaizeFieldID), data1(data1), data2(data2), q(q)
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
    //repast::IntUniformGenerator gen1 = repast::Random::instance()->createUniIntGenerator(0, 1); // initialise random number generator
    repast::DoubleUniformGenerator gen2 = repast::Random::instance()->createUniDoubleGenerator(0.05, sigmaahv); // initialise random number generator
    //q = gen1.next();

    y = yieldFromFile;
    BY = y*q*Ha; // calculate base yield
    H0 = BY * (1 + gen2.next()); // calculate household harvest
    currentYield = H0; //set current yield to household yield.
    //std::cout<<"currentYield "<< currentYield<<std::endl;
    //std::cout<<"Calculated Current Yield = "<<currentYield<<std::endl; 
}


MaizeField::~MaizeField()
{
    //delete Maizeprops;
}