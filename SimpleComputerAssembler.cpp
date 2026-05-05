/*
 * File Name: <codeFilename>
 * Programmer: <firstName> <lastName>
 * Course: <course>
 * Date: <currentDate>
 * Path: <pathOfFile>
 * Description: <descriptionOfProgram>
 * Compile: g++ <codeFilename> -o <outputFilename>
 * Execute: ./<outputFilename>
 * Additional Files: 
 * Generates File: 
 * Help Received: 
 */

//Included headers
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <utility>
#include <cstdint>
#include <iomanip>

#include "SimpleComputerAssembler.hpp"

//Main code
int main(int argc, char* args[])
{
    std::vector<uint16_t> machineCode;
    uint16_t machineLine = 0;
    std::string outputName = "a.mc";
    if(argc < 2)
    {
        std::cout << "Must pass a file to assemble!\n";
        return(-1);
    }
    machineCode = assembleFile(args[1]);
    if(argc > 2)
    {
        outputName = args[2];
    }
    

    std::ofstream assembledCode(outputName);
    assembledCode << std::hex << std::setfill('0');
    for(size_t i = 0; i < machineCode.size(); i++)
    {
        assembledCode << std::setw(4) << machineCode.first[i] << '\n';
    }
    return(0);
}
