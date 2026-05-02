//Included headers
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <utility>
#include <cstdint>
#include <iomanip>
#include <cctype>

enum operandGroupID{
    Invalid,
    D_A,
    D_A_B,
    D_B,
    D_O,
    D_A_O,
    A_OO,
    A_B,
    A
};

std::string makeLowercase(std::string str)
{
    for(size_t i = 0; i < str.size(); i++)
    {
        str[i] = std::tolower(str[i]);
    }
    return(str);
}

std::vector<std::pair<std::string, std::pair<uint16_t, operandGroupID>>> simpleComputerInstructions = {
        {"mova", {0b0000000, D_A}},
        {"inc",  {0b0000001, D_A}},
        {"add",  {0b0000010, D_A_B}},
        {"sub",  {0b0000101, D_A_B}},
        {"dec",  {0b0000110, D_A}},
        {"and",  {0b0001000, D_A_B}},
        {"or",   {0b0001001, D_A_B}},
        {"xor",  {0b0001010, D_A_B}},
        {"not",  {0b0001011, D_A}},
        {"movb", {0b0001100, D_B}},
        {"shr",  {0b0001101, D_B}},
        {"shl",  {0b0001110, D_B}},
        {"ldi",  {0b1001100, D_O}},
        {"adi",  {0b1000010, D_A_O}},
        {"ld",   {0b0010000, D_A}},
        {"st",   {0b0100000, A_B}},
        {"brz",  {0b1100000, A_OO}},
        {"brn",  {0b1100001, A_OO}},
        {"jmp",  {0b1110000, A}}
    };


void skipWhitespace(size_t& s, const std::string& str)
{
    while(s < str.size() && str[s] <= ' ')
    {
        s++;
    }
}
std::string disassembleLine(const uint16_t& line, std::vector<std::pair<std::string, std::pair<uint16_t, operandGroupID>>> instructions = simpleComputerInstructions)
{
    uint8_t opcode = (line & 0b1111111'000'000'000) >> 9;
    uint8_t regD   = (line & 0b0000000'111'000'000) >> 6;
    uint8_t regA   = (line & 0b0000000'000'111'000) >> 3;
    uint8_t regB   = (line & 0b0000000'000'000'111) >> 0;
    uint16_t ad    = (regD << 3) | regB;

    std::string instruction;
    operandGroupID operandGroup = Invalid;
    for(size_t i = 0; i < instructions.size(); i++)
    {
        if(opcode == instructions[i].second.first)
        {
            instruction = instructions[i].first;
            operandGroup = instructions[i].second.second;
            break;
        }
    }
    if(instruction.size())
    {
        instruction += " ";
        switch(operandGroup)
        {
            case D_A: //There is a destination and 1 source
                {
                    instruction += "r" + std::to_string(regD) 
                        + ", r" + std::to_string(regA);
                }
                break;

            case D_B:
                {
                    instruction += "r" + std::to_string(regD) 
                        + ", r" + std::to_string(regB);
                }
                break;

            case D_O:
                {
                    instruction += "r" + std::to_string(regD) 
                        + ", " + std::to_string(regB);
                }
                break;

            case D_A_B:
                {
                    instruction += "r" + std::to_string(regD) 
                        + ", r" + std::to_string(regA)
                        + ", r" + std::to_string(regB);
                }
                break;

            case D_A_O:
                {
                    instruction += "r" + std::to_string(regD) 
                        + ", r" + std::to_string(regA)
                        + ", " + std::to_string(regB);
                }
                break;

            case A_OO:
                {
                    instruction +=  "r" + std::to_string(regA)
                        + ", " + std::to_string(ad);
                }
                break;

            case A_B:
                {
                    instruction += "r" + std::to_string(regA)
                        + ", r" + std::to_string(regB);
                }
                break;

            case A:
                {
                    instruction += "r" + std::to_string(regA);
                }
                break;

            case Invalid:
                {
                    instruction += "XXX";
                }
                break;
        }
    }
    return(instruction);
}
//Main code
uint16_t assembleLine(std::string line, std::vector<std::pair<std::string, std::pair<uint16_t, operandGroupID>>> instructions = simpleComputerInstructions)
{
    line = makeLowercase(line);
    uint16_t machineLine = 0;

    std::string opcode;
    std::string p0;
    std::string p1;
    std::string p2;


    size_t s = line.find(' ');
    if(s != line.npos)
    {
        opcode = line.substr(0, s);
    }
    
    operandGroupID opcodeGroup;
    uint16_t opcodeVal = -1;
    for(size_t x = 0; x < instructions.size(); x++)
    {
        if(opcode == instructions[x].first)
        {
            opcodeVal   = instructions[x].second.first;
            opcodeGroup = instructions[x].second.second;
            opcodeVal <<= 9;
            break;
        }
    }

    if(opcodeVal == uint16_t(-1))
    {
        throw("");
    }

    machineLine = opcodeVal;
    switch(opcodeGroup)
    {
        case D_A: //There is a destination and 1 source
        case D_B:
        case D_O:
        case A_B: //Or two sources
            {
                //Gets the destination register
                skipWhitespace(s, line);
                size_t s2 = line.find(',', s);
                if(s2 != line.npos)
                {
                    p0 = line.substr(s, s2-s);
                }
                s = s2 + 1; //Skips the comma
                
                skipWhitespace(s, line);
                p1 = line.substr(s);

                if(opcodeGroup == D_O)
                {
                    p1.insert(0, "r");
                }
                //Checks for the r and removes it
                if(p0[0] != 'r' || (p1[0] == 'r'))
                {
                    int dest = std::stoi(p0.substr(1));
                    int src = std::stoi(p1.substr(1));
                    if(dest > 7 || dest < 0 || src > 7 | src < 0)
                    {
                        throw("");
                    }
                    dest <<= 3;
                    if(opcodeGroup != A_B)
                    {
                        dest <<= 3;
                    }
                    machineLine |= dest;
                    if(opcodeGroup == D_A)
                    {
                        src <<= 3;
                    }
                    machineLine |= src;
                }
                else
                {
                    throw("");
                }
            }
            break;

        case D_A_B:
        case D_A_O:
            {
                //Gets the destination register
                skipWhitespace(s, line);
                size_t s2 = line.find(',', s);
                if(s2 != line.npos)
                {
                    p0 = line.substr(s, s2-s);
                }
                s = s2 + 1; //Skips the comma

                //Gets the source register
                skipWhitespace(s, line);
                s2 = line.find(',', s);
                if(s2 != line.npos)
                {
                    p1 = line.substr(s, s2-s);
                }
                s = s2 + 1;

                skipWhitespace(s, line);
                p2 = line.substr(s);

                if(opcodeGroup == D_A_O)
                {
                    p2.insert(0, "r");
                }
                
                //Checks for the r and removes it
                if(p0[0] == 'r' && p1[0] == 'r' && p2[0] == 'r')
                {
                    try
                    {
                        int dest = std::stoi(p0.substr(1));
                        int srcA = std::stoi(p1.substr(1));
                        int srcB = std::stoi(p2.substr(1));
                        if(dest > 7 || dest < 0 || srcA > 7 | srcA < 0 || srcB > 7 || srcB < 0)
                        {
                            throw("");
                        }
                        machineLine |= dest << 6;
                        machineLine |= srcA <<= 3;
                        machineLine |= srcB;
                    }
                    catch(...)
                    {
                        throw("");
                    }
                }
                else
                {
                    throw("");
                }
            }
            break;

        case A_OO:
            {
                //Gets the destination register
                skipWhitespace(s, line);
                size_t s2 = line.find(',', s);
                if(s2 != line.npos)
                {
                    p0 = line.substr(s, s2-s);
                }
                s = s2 + 1; //Skips the comma

                //Gets the source register
                skipWhitespace(s, line);
                p1 = line.substr(s);
                
                p1.insert(0, "r");
                //Checks for the r and removes it
                if(p0[0] == 'r' && p1[0] == 'r')
                {
                    int dest = std::stoi(p0.substr(1));
                    int val = std::stoi(p1.substr(1));
                    //std::cout << val << '\n';
                    if(dest > 7 || dest < 0 || val > 31|| val < -32)
                    {
                        throw("");
                    }
                    machineLine |= (val & 0b0000000000111000) << 3;
                    machineLine |= dest << 3;
                    machineLine |= (val & 0b0000000000000111);
                }
                else
                {
                    throw("");
                }
            }
            break;
        case A:
            {
                //Gets the destination register
                skipWhitespace(s, line);
                p0 = line.substr(s);
                //Checks for the r and removes it
                //std::cout << p0[0] << '\n';
                if(p0[0] == 'r')
                {
                    int dest = std::stoi(p0.substr(1));
                    if(dest > 7 || dest < 0)
                    {
                        throw("");
                    }
                    machineLine |= dest << 3;
                }
                else
                {
                    throw("");
                }
            }
            break;
        case Invalid:
            {
                throw("");
            }
            break;
    }
    return(machineLine);
}

std::pair<std::vector<uint16_t>, std::vector<size_t>> assembleFile(const std::string& filename, std::vector<std::pair<std::string, std::pair<uint16_t, operandGroupID>>> instructions = simpleComputerInstructions)
{
    //Variables WOW
    std::ifstream assemblyFile(filename);
    std::string data;
    std::vector<std::string> lines;
    std::vector<uint16_t> machineCode;
    std::vector<size_t> fileCorrelations;

    //Opens the file
    if(assemblyFile.is_open())
    {
        //Loads each line into 
        size_t correlation = 0;
        while(getline(assemblyFile, data))
        {
            //Removes comments
            size_t s = data.find("//");
            if(s != data.npos)
            {
                data.erase(s);
            }

            s = 0;
            skipWhitespace(s, data);
            if(s != data.npos)
            {
                data = data.substr(s);
            }
            if(data.size())
            {
                lines.push_back(data);
                fileCorrelations.push_back(correlation);
            }
            correlation++;
        }
        for(size_t i = 0; i < lines.size(); i++)
        {
            machineCode.push_back(assembleLine(lines[i]));
        }
    }
    return(std::pair<std::vector<uint16_t>, std::vector<size_t>>{machineCode, fileCorrelations});
}
