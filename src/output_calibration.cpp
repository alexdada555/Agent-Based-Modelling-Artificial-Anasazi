
#include <stdio.h>
#include <iostream>
#include <string>
#include <math.h>
#include <cmath>
#include "repast_hpc/Utilities.h"
#include "repast_hpc/Properties.h"
#include <fstream>
#include <sstream>

using namespace std;

int main(void){
	// open calibration output file
	std::ofstream calibration_write;
    std::cout << "print 1" <<endl; 
	std::string fileName = "calibration/calibration_output.csv";
	calibration_write.open(fileName, std::ios_base::app);
	std::vector<std::string> data,datas;
	std::vector<int> model_data;
	std::vector<int> target_data;
	// data.clear();


    std::ofstream file_11;
    std::string output_file1 = "output";
    file_11.open(output_file1.c_str());

    std::ifstream file_22;
    std::string input_file1 = "household.csv";
    file_22.open(input_file1.c_str());

    if (!file_22.is_open())
        std::cout << "ERROR: File Open input\n";
    std::string value1;

    while(getline(file_22, value1,'\n') && file_22.good()){
        file_11<<value1;
    }

	// read input file
    std::ifstream file_1;
    std::string input_file = "input";
    file_1.open(input_file.c_str());
    if (!file_1.is_open())
    	std::cout << "ERROR: File Open input\n";
    std::string value;

    while(getline(file_1, value,'\n') && file_1.good()){
        data.push_back(value);

    }

    // read output file
    std::ifstream file_2;
    std::string output_file = "output";
    file_2.open(output_file.c_str());
    if (!file_2.is_open())
    	std::cout << "ERROR: File Open ouput\n";
std::cout << "print 1" <<endl; 
    while(getline(file_2, value,' ') && file_2.good()){
        model_data.push_back(repast::strToInt(value));
    }
//std::cout << "print 1" <<endl; 
    // read target file
    std::ifstream file_3;
    std::string target_file = "target_data.csv";
    file_3.open(target_file.c_str());
    if (!file_3.is_open())
    	std::cout << "ERROR: File Open\n";

    std::string line;
    while (getline(file_3, line)){
	    std::stringstream ss(line);
	    while(getline(ss, value,',') && file_3.good()){
	        target_data.push_back(repast::strToInt(value));
	    }
	}
//std::cout << "print 4" <<endl; 
    double RSME = 0;
    double MAE = 0;

    for (int i=0; i<model_data.size(); i++){
    	RSME = RSME + (model_data[i]-target_data[i])*(model_data[i]-target_data[i]);
    	MAE = MAE + abs(model_data[i]-target_data[i]);
    }

    RSME = sqrt(RSME/model_data.size());
    MAE = MAE/model_data.size();

    // compare output and target data
	// save result
//std::cout << "print 5" <<endl; 

	// write to output file
	calibration_write << RSME << ", ";
	calibration_write << MAE << ", ";
	calibration_write << repast::strToInt(data[0]) << ", ";
	calibration_write << repast::strToInt(data[1]) << ", ";
	calibration_write << repast::strToInt(data[2]) << ", ";
	calibration_write << repast::strToInt(data[3]) << ", ";
	calibration_write << repast::strToDouble(data[4]) << ", ";
	calibration_write << repast::strToDouble(data[5]) << ", ";
    calibration_write << repast::strToDouble(data[6]) << ", ";

    std::cout << "print 6" <<endl; 

	calibration_write.close();
}