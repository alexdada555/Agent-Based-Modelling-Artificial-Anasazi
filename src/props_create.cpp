

#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <cmath>
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"

using namespace std;

int main(void){
	std::ofstream props_write;
	std::string fileName = "props/model.props";
	props_write.open(fileName);
	std::vector<std::string> data;
	data.clear();

	// read input file
    std::ifstream file;
    std::string input_file = "input";
    file.open(input_file.c_str());
    if (!file.is_open())
    	std::cout << "ERROR: File Open\n";
    std::string value;

    while(getline(file, value,'\n') && file.good()){
        data.push_back(value);
    }

	// write to props file
	props_write << "mindeathAge = " << repast::strToInt(data[0]) << endl;
	props_write << "maxdeathAge = " << repast::strToInt(data[1]) << endl;
	props_write << "minFertileAge = " << repast::strToInt(data[2]) << endl;
	props_write << "maxFertileAge= " << repast::strToInt(data[3]) << endl;
	props_write << "fissionProb = " << repast::strToDouble(data[4]) << endl;
	props_write << "harvest.adjustment.level = " << repast::strToDouble(data[5]) << endl;
	props_write << "sigmaahv = " << repast::strToDouble(data[6]) << endl;

	props_write << "global.random.seed = 2" << endl;
	props_write << "startYear = 800" << endl;	
	props_write << "stop.at = 551" << endl;
	props_write << "board.sizex = 80" << endl;
	props_write << "board.sizey = 120" << endl;
	props_write << "count.of.agents = 14" << endl;
	props_write << "q = 0.9" << endl;

	props_write.close();
	
	return 0;
}