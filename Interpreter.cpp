//Included headers
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <utility>
#include <cstdint>
#include <iomanip>
#include <cmath>

#include "SimpleComputerAssembler.hpp"

std::string removeLeadingWhitespace(std::string str)
{
    while(str.size() && str[0] <= ' ')
    {
        str.erase(str.begin());
    }
    return(str);
}

struct SimpleComputer
{
    //General Purpose Registers
    std::vector<uint16_t> registers;

    //Program Counter
    uint16_t pc;

    //Memory
    std::vector<uint16_t> instructionMemory;
    std::vector<uint16_t> dataMemory;
    std::vector<uint16_t> initialDataMemory;

    SimpleComputer(const std::string& instructionFilename = "", const std::string& dataFilename = "")
    {
        this->reset();
        try
        {
            if(instructionFilename.size())
            {
                this->loadInstructions(instructionFilename);
            }

            if(dataFilename.size())
            {
                this->loadData(dataFilename);
            }
        }
        catch(const std::exception& e)
        {
            std::cout << "Error loading files: " << e.what() << '\n';
        }
    }
    void reset()
    {
        this->pc = 0;
        this->registers = std::vector<uint16_t>(8, 0);
        this->dataMemory = this->initialDataMemory;
        return;
    }
    void loadData(const std::string& filename)
    {
        this->dataMemory = std::vector<uint16_t>(0xFFFF+1, 0);
        std::string line;
        std::ifstream dataFile(filename);
        size_t i = 0;
        while(std::getline(dataFile, line))
        {
            line = removeLeadingWhitespace(line);
            size_t com = line.find("//");
            if(com != line.npos)
            {
                line.erase(com);
            }
            if(line.size())
            {
                if(line[0] == '@')
                {
                    i = std::stoi(line.substr(1), nullptr, 16);
                }
                else
                {
                    this->dataMemory[i] = std::stoi(line, nullptr, 16);
                    i++;
                }
            }
        }
        this->initialDataMemory = this->dataMemory;
    }
    void loadInstructions(const std::vector<uint16_t>& machineCode)
    {
        this->instructionMemory = std::vector<uint16_t>(0xFFFF+1, 0);
        for(size_t i = 0; i < machineCode.size() && i < 0xFFFF+1; i++)
        {
            this->instructionMemory[i] = machineCode[i];
        }
    }
    void loadInstructions(const std::string& filename)
    {
        this->instructionMemory = std::vector<uint16_t>(0xFFFF+1, 0);
        std::string line;
        std::ifstream instructionFile(filename);
        size_t i = 0;
        while(std::getline(instructionFile, line))
        {
            line = removeLeadingWhitespace(line);
            size_t com = line.find("//");
            if(com != line.npos)
            {
                line.erase(com);
            }
            if(line.size())
            {
                if(line[0] == '@')
                {
                    i = std::stoi(line.substr(1), nullptr, 16);
                }
                else
                {
                    this->instructionMemory[i] = std::stoi(line, nullptr, 16);
                    i++;
                }
            }
        }
    }

    uint8_t getOpcode(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }

        return((this->instructionMemory[i] & 0b1111111'000'000'000) >> 9);
    }

    uint8_t getRegD(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }

        return((this->instructionMemory[i] & 0b0000000'111'000'000) >> 6);
    }

    uint8_t getRegA(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }

        return((this->instructionMemory[i] & 0b0000000'000'111'000) >> 3);
    }

    uint8_t getRegB(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }

        return(this->instructionMemory[i] & 0b0000000'000'111'000);
    }

    uint8_t getAD(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }

        return(((this->instructionMemory[i] & 0b0000000'111'000'000) >> 3) 
                | this->instructionMemory[i] & 0b0000000'000'111'000);
    }

    std::vector<uint16_t> getRegisterReads(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }
        std::vector<uint16_t> readRegisters;
        uint16_t opcode = this->getOpcode(i);
        operandGroupID operandGroup = Invalid;
        for(size_t i = 0; i < simpleComputerInstructions.size(); i++)
        {
            if(opcode == simpleComputerInstructions[i].second.first)
            {
                operandGroup = simpleComputerInstructions[i].second.second;
                break;
            }
        }
        switch(operandGroup)
        {
            case Invalid:
            case D_O:
                break;
            case D_A_B:
                {
                    readRegisters.push_back(this->getRegA(i));
                    readRegisters.push_back(this->getRegB(i));
                }
                break;
            case D_A:
            case A_OO:
            case D_A_O:
            case A:
                {
                    readRegisters.push_back(this->getRegA(i));
                }
                break;
            case D_B:
                {
                    readRegisters.push_back(this->getRegB(i));
                }
                break;
        }

        return(readRegisters);
    }

    int16_t getRegisterWrite(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }
        int16_t writeRegister = -1;
        uint16_t opcode = this->getOpcode(i);
        operandGroupID operandGroup = Invalid;
        for(size_t i = 0; i < simpleComputerInstructions.size(); i++)
        {
            if(opcode == simpleComputerInstructions[i].second.first)
            {
                operandGroup = simpleComputerInstructions[i].second.second;
                break;
            }
        }
        switch(operandGroup)
        {
            case Invalid:
            case A_OO:
            case A:
                break;

            case D_O:
            case D_A_B:
            case D_A:
            case D_A_O:
            case D_B:
                {
                    writeRegister = this->getRegD(i);
                }
                break;
        }

        return(writeRegister);
    }

    int32_t getMemoryRead(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }
        int32_t readMemory = -1;
        uint16_t opcode = this->getOpcode(i);
        bool isLoad = false;
        for(size_t i = 0; i < simpleComputerInstructions.size(); i++)
        {
            if(opcode == simpleComputerInstructions[i].second.first)
            {
                if(simpleComputerInstructions[i].first == "ld")
                {
                    isLoad = true;
                    break;
                }
            }
        }
        if(isLoad)
        {
            readMemory = this->registers[this->getRegA(i)];
        }
        return(readMemory);
    }

    int32_t getMemoryWrite(int32_t i = -1)
    {
        if(i == -1)
        {
            i = this->pc;
        }
        int32_t writeMemory = -1;
        uint16_t opcode = this->getOpcode(i);
        bool isStore = false;
        for(size_t i = 0; i < simpleComputerInstructions.size(); i++)
        {
            if(opcode == simpleComputerInstructions[i].second.first)
            {
                if(simpleComputerInstructions[i].first == "st")
                {
                    isStore = true;
                    break;
                }
            }
        }
        if(isStore)
        {
            writeMemory = this->registers[this->getRegA(i)];
        }
        return(writeMemory);
    }
    //Divides the instruction into multiple parts
    void clock()
    {
        //Gets the current instruction
        uint16_t ir = this->instructionMemory[pc];

        //Divides the instruction into multiple parts
        uint8_t opcode = (ir & 0b1111111'000'000'000) >> 9;
        uint8_t regD   = (ir & 0b0000000'111'000'000) >> 6;
        uint8_t regA   = (ir & 0b0000000'000'111'000) >> 3;
        uint8_t regB   = (ir & 0b0000000'000'000'111) >> 0;
        uint16_t ad    = (regD << 3) | regB;

        //Sign extension :)
        if(ad & 0b100000)
        {
            ad |= 0b1111111111000000;
        }

        //This is a very brute force method for now, maybe over the summer I'll make it work for any theoretical opcode
        switch(opcode)
        {
            case 0b0000000: //MOVA
            {
                this->registers[regD] = this->registers[regA];
                this->pc++;
            }
            break;

            case 0b0000001: //INC
            {
                this->registers[regD] = this->registers[regA] + 1;
                this->pc++;
            }
            break;

            case 0b0000010: //ADD
            {
                this->registers[regD] = this->registers[regA] + this->registers[regB];
                this->pc++;
            }
            break;

            case 0b0000101: //SUB
            {
                this->registers[regD] = this->registers[regA] - this->registers[regB];
                this->pc++;
            }
            break;

            case 0b0000110: //DEC
            {
                this->registers[regD] = this->registers[regA] - 1;
                this->pc++;
            }
            break;

            case 0b0001000: //AND
            {
                this->registers[regD] = this->registers[regA] & this->registers[regB];
                this->pc++;
            }
            break;

            case 0b0001001: //OR
            {
                this->registers[regD] = this->registers[regA] | this->registers[regB];
                this->pc++;
            }
            break;

            case 0b0001010: //XOR
            {
                this->registers[regD] = this->registers[regA] ^ this->registers[regB];
                this->pc++;
            }
            break;

            case 0b0001011: //NOT
            {
                this->registers[regD] = ~this->registers[regA];
                this->pc++;
            }
            break;

            case 0b0001100: //MOVB
            {
                this->registers[regD] = this->registers[regB];
                this->pc++;
            }
            break;

            case 0b0001101: //SHR
            {
                this->registers[regD] = this->registers[regB] >> 1;
                this->pc++;
            }
            break;

            case 0b0001110: //SHL
            {
                this->registers[regD] = this->registers[regB] << 1;
                this->pc++;
            }
            break;

            case 0b1001100: //LDI
            {
                this->registers[regD] = regB;
                this->pc++;
            }
            break;

            case 0b1000010: //ADI
            {
                this->registers[regD] = this->registers[regA] + regB;
                this->pc++;
            }
            break;

            case 0b0010000: //L
            {
                this->registers[regD] = this->dataMemory[this->registers[regA]];
                this->pc++;
            }
            break;

            case 0b0100000: //ST
            {
                this->dataMemory[this->registers[regA]] = this->registers[regB];
                this->pc++;
            }
            break;

            case 0b1100000: //BRZ
            {
                if(this->registers[regA] == 0)
                {
                    this->pc += ad;
                }
                else
                {
                    this->pc++;
                }
            }
            break;

            case 0b1100001: //BRN
            {
                if(static_cast<int16_t>(this->registers[regA]) < 0)
                {
                    this->pc += ad;
                }
                else
                {
                    this->pc++;
                }
            }
            break;

            case 0b1110000: //JMP
            {
                this->pc = this->registers[regA];
            }
            break;

            default:
            {
                std::cout << "Invalid operand! " << std::hex << int(opcode) << '\n';
            }

        }
    }
};

void printIntro()
{
    std::cout << "Simple Computer Emulator. By Nathanial Keatts\n"
        << "Developed to debug assembly for ECE-2544 @ VT\n"
        << "Type 'h' for help on commands\n";
    std::cout << '\n';
}

void printHelp(std::string command)
{
    if(command == "")
    {
    std::cout << "Help:\n"
        << "Command              | Short Description\n"
        << "h <command>          | Shows general help or specific help for the input command <command>.\n"
        << "s                    | Step through 1 line of code.\n"
        << "r                    | Start execution until next breakpoint or trivial infinite loop.\n"
        << "b <pc value>         | Create a breakpoint at a specific pc value.\n"
        << "bm <machine code>    | Create a breakpoint for a specific machine code.\n"
        << "ba <action>          | Create a breakpoint on a specific memory/register read/write.\n"
        << "bl <source line>     | Create a breakpoint on a specific line of the assembly source code.\n"
        << "cb<category>         | Clears all breakpoints of a specific category.\n"
        << "p <variable>         | Prints register, program counter, and instruction register contents. <variable> selects a specific output.\n"
        << "d                    | Shows the disassembled line that the program counter is currently on.\n"
        << "c <bound>            | Shows multiple dissassembled lines with <bound> before and <bound> after the current line.\n"
        << "cs <bound>           | Shows source file lines <bound> distance from the current executing line.\n"
        << "v <variable> <value> | Sets the value of a variable.\n"
        << "lm <filename>        | Loads a raw machine code file into instruction memory.\n"
        << "la <filename>        | Loads an assembly file. Assembles and stores into instruction memory.\n"
        << "x                    | Resets the contents of the Simple Computer.\n"
        << "q                    | Closes the debugger.\n";

    }
    else if(command == "h")
    {
        std::cout << "h <command>: Help\n"
            << "    Without a parameter, shows general help for every command. If a command is included, more specific help is provided.\n";
    }
    else if(command == "s")
    {
        std::cout << "s: Step\n"
            << "    Executes a single line of machine code.\n";
    }
    else if(command == "r")
    {
        std::cout << "r: Run\n"
            << "    Starts executing machine code until either a breakpoint is reached or a trivial infinite loop is reached.\n"
            << "    A trivial infinite loop continually jumps to the same point making the program counter never iterate.\n";
    }
    else if(command == "b")
    {
        std::cout << "b <pc value>: Make Breakpoint\n"
            << "    Creates a breakpoint that stops execution if the program counter matches <pc value>.\n"
            << "    *For every breakpoint, the line that triggers the breakpoint is not executed.*\n"
            << "    *To trigger this line you can use the step command after the breakpoint is reached.*\n";
    }
    else if(command == "bm")
    {
        std::cout << "bm <machine code>: Create Machine Code Breakpoint\n"
            << "    Creates a breakpoint that stops execution if the machine instruction about to be executed matches <machine code>.\n"
            << "    *For every breakpoint, the line that triggers the breakpoint is not executed.*\n"
            << "    *To trigger this line you can use the step command after the breakpoint is reached.*\n";
    }
    else if(command == "ba")
    {
        std::cout << "ba <action>: Create Breakpoint on Action\n"
            << "    Creates a breakpoint that stops execution if the next line would trigger <action>.\n"
            << "    The format of <action> is given below\n"
            << "    ABn\n"
            << "    A is the area to monitor. It can be either r (registers) or m(memory).\n"
            << "    B is the action to listen for. It can be either r (read), w (write), or x (read or write).\n"
            << "    n is the index to monitor. It can be any number, but the ranges for each area are below:\n"
            << "    r [0, 7], m [0, 65535]\n"
            << "    Action will have these appended directly together with no spacing. Some examples:\n"
            << "        :ba mw6 //Break on memory write to address 6\n"
            << "        :ba rr3 //Break on read from register 3\n"
            << "        :ba rx0x10 //Break on memory write to address 0x10 (decimal 16)\n"
            << "    A value outside of these ranges will still create a breakpoint, however it will never trigger.\n"
            << "    *For every breakpoint, the line that triggers the breakpoint is not executed.*\n"
            << "    *To trigger this line you can use the step command after the breakpoint is reached.*\n";
    }
    else if(command == "bl")
    {
        std::cout << "bl <source line>: Create Breakpoint on Source Line\n"
            << "    Creates a breakpoint that stops execution if the line about to be executed is <source line> in the source assembly file.\n"
            << "    *For every breakpoint, the line that triggers the breakpoint is not executed.*\n"
            << "    *To trigger this line you can use the step command after the breakpoint is reached.*\n"
            << "    **Any command that reads the source assembly file requires the source file to be loaded. If this isn't the case, the command will give an error.**\n";

    }
    else if(command == "cb")
    {
        std::cout << "cb<type>: Clear Breakpoints\n"
            << "    Clears all breakpoints of a specific type matching <type>.\n"
            << "    The format for for the command is as follows:\n"
            << "    :cb     //Clears all pc breakpoints\n"
            << "    :cba    //Clears all breakpoints\n"
            << "    :cbl    //Clears all breakpoints for specific source lines\n"
            << "    :cbm    //Clears all breakpoints for specific machine code instructions\n"
            << "    :cbmr   //Clears all breakpoints for memory reads\n"
            << "    :cbmw   //Clears all breakpoints for memory writes\n"
            << "    :cbmx   //Clears all breakpoints for memory reads and writes\n"
            << "    :cbrr   //Clears all breakpoints for register reads\n"
            << "    :cbrw   //Clears all breakpoints for register writes\n"
            << "    :cbrx   //Clears all breakpoints for register reads and writes\n";
    }
    else if(command == "p")
    {
        std::cout << "p <variable>: Print\n"
            << "    Prints the specified variable. If no variable is specified, prints PC, IR, and R0-7\n"
            << "    Valid variables are:\n"
            << "    rx //Register x, where x is [0, 7]\n"
            << "    mx //Memory address x, where x is [0, 65535]\n"
            << "    pc //The program counter\n"
            << "    ir //The instruction register\n";
    }
    else if(command == "d")
    {
        std::cout << "d: Disassemble\n"
            << "    Disassembles the current line and prints it.\n"
            << "    This does not use the source file and can be performed on any machine code.\n";
    }
    else if(command == "c")
    {
        std::cout << "c <bound>: Context\n"
            << "    Disassembles the current line and <bound lines before and after the current line.\n"
            << "    This does not use the source file and can be performed on any machine code.\n";
    }
    else if(command == "cs")
    {
        std::cout << "s: Step\n"
            << "    Displays the current line from the source file and <bound> lines before and after the current line.\n"
            << "    *This uses the source assembly file. If the source assembly file was not loaded this command will not work.*\n";
    }
    else if(command == "v")
    {
        std::cout << "v <variable> <value>: Set Value\n"
            << "    Sets the value of a variable.\n"
            << "    The variable can be any of the following:\n"
            << "    rx //Register x, where x is [0, 7]\n"
            << "    mx //Memory address x, where x is [0, 65535]\n"
            << "    pc //The program counter\n"
            << "    <value> will be stored into <variable>.\n";
    }
    else if(command == "lm")
    {
        std::cout << "lm <filename>: Load Machine Code\n"
            << "    Opens a machine code file and loads it into the Simple Computer.\n"
            << "    This operation supports @ to change the location of the code and // for comments.\n"
            << "    *This command will clear any source file breakpoints and invalidate the use of cs until an assembly source file is loaded.*\n"
            << "    **If the file successfully opens, this will reset the program counter to 0.**\n";
    }
    else if(command == "la")
    {
        std::cout << "s: Step\n"
            << "    Opens an assembly code file, assembles it, and loads it into the Simple Computer.\n"
            << "    *This command will store the source file and allow use of the cs command.*\n"
            << "    **If the file successfully opens, this will reset the program counter to 0.**\n";
    }
    else if(command == "x")
    {
        std::cout << "x: Reset\n"
            << "    Resets the program counter to 0 and fills data memory with its initial values.\n";
    }
    else if(command == "q")
    {
        std::cout << "q: Quit\n"
            << "    Closes the debugger.\n";
    }
    else
    {
        std::cout << "Invalid command '" << command << "'. Type 'h' for a list of valid commands.\n";
    }


    std::cout << '\n';
}

//Main code
int main(int argc, char* args[])
{
    std::vector<uint16_t> pcBreakpoints;
    std::vector<uint16_t> mcBreakpoints;
    std::vector<uint16_t> mrBreakpoints;
    std::vector<uint16_t> mwBreakpoints;
    std::vector<uint8_t>  rrBreakpoints;
    std::vector<uint8_t>  rwBreakpoints;
    std::vector<size_t>   slBreakpoints;
                
    //The correlations with the assembly source
    std::vector<size_t> fileCorrelations;
    std::vector<std::string> rawAssembly;

    //Load instruction memory and data memory
    //Here I will interpret what the passed arguments are
    //std::vector<std::string> arguments(argc);
    SimpleComputer sc;

    bool loadedData = false;
    bool loadedInst = false;
    for(size_t i = 0; i < argc; i++)
    {
        std::string arg = args[i];
        if(arg.substr(0, 2) == "-a") //Load assembly file
        {
            int offset = 2;
            if(arg.size() <= 2)
            {
                offset = 0;
                i++;
            }
            if(i < argc)
            {
                std::ifstream assemblyFile(arg.substr(offset));
                if(assemblyFile.is_open())
                {
                    assemblyFile.close();
                    loadedInst = true;
                    std::string currentLine;
                    while(getline(assemblyFile, currentLine))
                    {
                        rawAssembly.push_back(currentLine);
                    }
                    std::pair<std::vector<uint16_t>, std::vector<size_t>> assembledCode = assembleFile(removeLeadingWhitespace(arg.substr(2)));
                    sc.loadInstructions(assembledCode.first);
                    fileCorrelations = assembledCode.second;
                }
                else
                {
                    std::cout << "Couldn't locate file '" << arg.substr(2) << '\n';
                }
            }
        }
        else if(arg.substr(0, 2) == "-m") //Load machine code
        {
            rawAssembly.clear();
            int offset = 2;
            if(arg.size() <= 2)
            {
                offset = 0;
                i++;
            }
            if(i < argc)
            {
                sc.loadInstructions(arg.substr(offset));
                loadedInst = true;
            }
        }
        else if(arg.substr(0, 2) == "-d") //Load data
        {
            loadedData = true;
            int offset = 2;
            if(arg.size() <= 2)
            {
                offset = 0;
                i++;
            }
            if(i < argc)
            {
                sc.loadData(arg.substr(offset));
            }
        }
    }

    printIntro();
    if(!loadedInst)
    {
        std::cout << "No instructions loaded. Use 'lm' or 'la' to load machine code or assembly.\n";
    }

    if(!loadedData)
    {
        std::cout << "No data loaded. Use 'ld' to load a raw data file.\n";
    }

    if(!loadedInst || !loadedData)
    {
        std::cout << '\n';
    }
    
    std::string command;
    std::string prevCommand;
    size_t clockCycle = 0;
    bool running = false;
    bool step = false;
    bool forceContinue = false;
    while(true)
    {
        if(!forceContinue && running)
        {
            /*if(sc.getMemoryWrite() != -1)*/
            /*{*/
                /*std::cout << "Memory write at " << sc.getMemoryWrite() << '\n';*/
            /*}*/
            //Check for any of the breakpoints
            for(size_t i = 0; i < pcBreakpoints.size(); i++)
            {
                if(sc.pc == pcBreakpoints[i])
                {
                    running = false;
                    std::cout << "Encountered a breakpoint at PC = " << sc.pc << '\n';
                }
            }
            for(size_t i = 0; i < mcBreakpoints.size(); i++)
            {
                if(sc.instructionMemory[sc.pc] == mcBreakpoints[i])
                {
                    running = false;
                    std::cout << "Encountered a breakpoint when IR = " << mcBreakpoints[i] << '\n';
                }
            }
            for(size_t i = 0; i < mrBreakpoints.size(); i++)
            {
                if(sc.getMemoryRead() == mrBreakpoints[i])
                {
                    running = false;
                    std::cout << "Encountered a breakpoint on memory read at address = " 
                        << std::hex << mrBreakpoints[i] << '\n';
                }
            }
            for(size_t i = 0; i < mwBreakpoints.size(); i++)
            {
                if(sc.getMemoryWrite() == mwBreakpoints[i])
                {
                    running = false;
                    std::cout << "Encountered a breakpoint on memory write at address = " 
                        << std::hex << mwBreakpoints[i] << '\n';
                }
            }
            for(size_t i = 0; i < slBreakpoints.size(); i++)
            {
                if(fileCorrelations[sc.pc] == slBreakpoints[i])
                {
                    running = false;
                    std::cout << "Encountered a breakpoint at source line = " 
                        << std::hex << slBreakpoints[i] << '\n';
                }
            }
        }
        forceContinue = false;


        if(running || step)
        {
            step = false;
            uint16_t ppc = sc.pc;
            clockCycle++;
            sc.clock();
            if(!step && sc.pc == ppc)
            {
                std::cout << "Trivial infinite loop encountered, stopping execution\n";
                running = false;
            }
        }
        else
        {
            std::cout << ":";
            std::getline(std::cin, command);
            if(!command.size())
            {
                command = prevCommand;
            }
            else
            {
                prevCommand = command;
            }
            std::string csCommand = command;
            command = makeLowercase(command);

            //Add a breakpoint
            if(command == "s")
            {
                step = true;
                forceContinue = true;
            }
            else if(command == "r")
            {
                running = true;
                forceContinue = true;
            }
            else if(command.substr(0, 2) == "b ")
            {
                try
                {
                    pcBreakpoints.push_back(std::stoi(removeLeadingWhitespace(command.substr(1))));
                    std::cout << "Added breakpoint at PC = " << std::hex
                        << pcBreakpoints[pcBreakpoints.size()-1] 
                        << std::dec << '\n';
                }
                catch(...)
                {
                    std::cout << "Breakpoint must be a number\n";
                }
            }
            else if(command.substr(0, 3) == "bm ") //Breakpoint on machine code
            {
                try
                {
                    mcBreakpoints.push_back(std::stoi(removeLeadingWhitespace(command.substr(1))));
                    std::cout << "Added breakpoint for machine code = " 
                        << std::hex << mcBreakpoints[mcBreakpoints.size()-1] 
                        << std::dec << '\n';
                }
                catch(...)
                {
                    std::cout << "Breakpoint must be a number\n";
                }
            }
            else if(command.substr(0, 3) == "ba ") //Breakpoint on action
            {
                std::string action = removeLeadingWhitespace(command.substr(2));
                if(action.size() > 2)
                {
                    if(action[0] == 'r')
                    {
                        if(action[1] == 'r')
                        {
                            rrBreakpoints.push_back(std::stoi(action.substr(2)));
                            std::cout << "Added breakpoint for register read for register "
                                << std::dec
                                << rrBreakpoints[rrBreakpoints.size() - 1] << '\n';
                        }
                        else if(action[1] == 'w')
                        {
                            rwBreakpoints.push_back(std::stoi(action.substr(2)));
                            std::cout << "Added breakpoint for register write for register "
                                << std::dec
                                << rwBreakpoints[rwBreakpoints.size() - 1] << '\n';
                        }
                        else if(action[1] == 'x')
                        {
                            rrBreakpoints.push_back(std::stoi(action.substr(2)));
                            rwBreakpoints.push_back(std::stoi(action.substr(2)));
                            std::cout << "Added breakpoint for register read and write for register "
                                << std::dec
                                << rrBreakpoints[rrBreakpoints.size() - 1] << '\n';
                        }
                        else
                        {
                            std::cout << "Invalid register action '" << action[1] << "'\n";
                        }
                    }
                    else if(action[0] == 'm')
                    {
                        if(action[1] == 'r')
                        {
                            mrBreakpoints.push_back(std::stoi(action.substr(2)));
                            std::cout << "Added breakpoint for memory read at address 0x" 
                                << std::hex << mrBreakpoints[mrBreakpoints.size() - 1] << std::dec << '\n';
                        }
                        else if(action[1] == 'w')
                        {
                            mwBreakpoints.push_back(std::stoi(action.substr(2)));
                            std::cout << "Added breakpoint for memory write at address 0x" 
                                << std::hex << mwBreakpoints[mwBreakpoints.size() - 1] << std::dec << '\n';
                        }
                        else if(action[1] == 'x')
                        {
                            mrBreakpoints.push_back(std::stoi(action.substr(2)));
                            mwBreakpoints.push_back(std::stoi(action.substr(2)));
                            std::cout << "Added breakpoint for memory read and write at address 0x" 
                                << std::hex << mwBreakpoints[mwBreakpoints.size() - 1] << std::dec << '\n';
                        }
                        else
                        {
                            std::cout << "Invalid register action '" << action[1] << "'\n";
                        }
                    }
                    else
                    {
                        std::cout << "Invalid type of action '" << action[0] << "'\n";
                    }
                }
            }
            else if(command.substr(0, 3) == "bl ")
            {
                if(!rawAssembly.size())
                {
                    std::cout << "Cannot create a source breakpoint without loading assembly source!\n";
                }
                try
                {
                    pcBreakpoints.push_back(std::stoll(removeLeadingWhitespace(command.substr(3))));
                    std::cout << "Added breakpoint at source line " << std::dec
                        << slBreakpoints[slBreakpoints.size()-1] 
                        << std::dec << '\n';
                }
                catch(...)
                {
                    std::cout << "Breakpoint must be a number\n";
                }
            }
            else if(command.substr(0, 2) == "cb")
            {
                if(command.substr(2) == "")
                {
                    pcBreakpoints.clear();
                }
                else if(command.substr(2) == "a")
                {
                    pcBreakpoints.clear();
                    mcBreakpoints.clear();
                    mrBreakpoints.clear();
                    mwBreakpoints.clear();
                    rrBreakpoints.clear();
                    rwBreakpoints.clear();
                    slBreakpoints.clear();
                }
                else if(command.substr(2) == "m")
                {
                    mcBreakpoints.clear();
                }
                else if(command.substr(2) == "amr")
                {
                    mrBreakpoints.clear();
                }
                else if(command.substr(2) == "amw")
                {
                    mwBreakpoints.clear();
                }
                else if(command.substr(2) == "amx")
                {
                    mrBreakpoints.clear();
                    mwBreakpoints.clear();
                }
                else if(command.substr(2) == "arr")
                {
                    rrBreakpoints.clear();
                }
                else if(command.substr(2) == "arw")
                {
                    rwBreakpoints.clear();
                }
                else if(command.substr(2) == "arx")
                {
                    rrBreakpoints.clear();
                    mrBreakpoints.clear();
                }
                else if(command.substr(2) == "l")
                {
                    slBreakpoints.clear();
                }
                
            }
            else if(command.substr(0, 1) == "p")
            {
                if(command.size() > 1)
                {
                    try
                    {
                        std::string action = removeLeadingWhitespace(command.substr(1));
                        if(action.substr(0, 1) == "m")
                        {
                            std::cout << sc.dataMemory[std::stoi(removeLeadingWhitespace(action.substr(1)))] << '\n';
                        }
                        else if(action.substr(0, 1) == "r")
                        {
                            std::cout << sc.registers[std::stoi(removeLeadingWhitespace(action.substr(1)))] << '\n';
                        }
                        else if(action == "pc")
                        {
                            std::cout << "PC: " << sc.pc << '\n';
                        }
                        else if(action == "ir")
                        {
                            std::cout << "IR: " << sc.instructionMemory[sc.pc] << '\n';
                        }
                    }
                    catch(...)
                    {
                        std::cout << "Invalid format for print command\n";
                    }
                }
                else
                {
                    std::cout << "PC: " << sc.pc << '\n';
                    std::cout << "IR: " << sc.instructionMemory[sc.pc] << '\n';
                    //std::cout << "Registers:\n";
                    for(size_t i = 0; i < 8; i++)
                    {
                        std::cout << "R" << i << ": " << std::hex << sc.registers[i] << '\n';
                    }
                }
            }
            else if(command == "d")
            {
                std::cout << disassembleLine(sc.instructionMemory[sc.pc]) << '\n';
            }
            else if(command.substr(0, 2) == "c ")
            {
                try
                {
                    int bounds = std::stoi(removeLeadingWhitespace(command.substr(1)));
                    for(int i = -bounds; i < bounds + 1; i++)
                    {
                        if(sc.pc + i >= 0 && sc.pc + i < 0xFFFF+1)
                        {
                            std::cout << std::setw(4) << std::hex << sc.pc + i << std::dec << " : ";
                            if(i == 0)
                            {
                                std::cout << '>';
                            }
                            else
                            {
                                std::cout << ' ';
                            }
                            std::cout << disassembleLine(sc.instructionMemory[sc.pc + i]) << '\n';
                        }
                    }
                }
                catch(...)
                {
                    std::cout << "Context requires a bound\n";
                }
            }
            else if(command.substr(0, 3) == "cs ")
            {
                if(rawAssembly.size())
                {
                    try
                    {
                        int bounds = std::stoi(removeLeadingWhitespace(command.substr(2)));
                        for(int i = -bounds; i < bounds + 1; i++)
                        {
                            std::cout << std::dec;
                            if(fileCorrelations[sc.pc] + i >= 0 && fileCorrelations[sc.pc] + i < rawAssembly.size())
                            {
                                std::cout << std::setw(std::log10(rawAssembly.size()) + 1) << fileCorrelations[sc.pc] + i << " : ";
                                if(i == 0)
                                {
                                    std::cout << '>';
                                }
                                else
                                {
                                    std::cout << ' ';
                                }
                                std::cout << rawAssembly[fileCorrelations[sc.pc] + i] << '\n';
                            }
                        }
                    }
                    catch(...)
                    {
                        std::cout << "Context requires a bound\n";
                    }
                }
                else
                {
                    std::cout << "Cannot use context with source command without including source. Use 'la <file>' to load source\n";
                }
            }
            else if(command.substr(0, 2) == "v ")
            {
                std::string action = removeLeadingWhitespace(command.substr(1));
                try
                {
                    if(action.substr(0, 1) == "r")
                    {
                        size_t sp = action.find(" ");
                        if(sp != action.npos)
                        {
                            uint16_t reg = std::stoi(action.substr(1, action.size()-sp));
                            uint16_t val = std::stoi(removeLeadingWhitespace(action.substr(sp)));
                            sc.registers[reg] = val;
                        }
                        else
                        {
                            throw("");
                        }
                    }
                    else if(action.substr(0, 1) == "m")
                    {
                        size_t sp = action.find(" ");
                        if(sp != action.npos)
                        {
                            uint16_t add = std::stoi(action.substr(1, action.size()-sp));
                            uint16_t val = std::stoi(removeLeadingWhitespace(action.substr(sp)));
                            sc.dataMemory[add] = val;
                        }
                        else
                        {
                            throw("");
                        }
                    }
                    else if(action.substr(0, 2) == "pc")
                    {
                        size_t sp = action.find(" ");
                        if(sp != action.npos)
                        {
                            uint16_t val = std::stoi(removeLeadingWhitespace(action.substr(sp)));
                            sc.pc = val;
                        }
                        else
                        {
                            throw("");
                        }
                    }
                    else
                    {
                        std::cout << "Value can only be used to set r, m, or pc\n";
                    }
                }
                catch(...)
                {
                    std::cout << "Invalid format for value command\n";
                }
            }
            else if(command.substr(0, 3) == "lm ") //Load machine code
            {
                std::string filename = removeLeadingWhitespace(csCommand.substr(2));
                std::ifstream test(filename);
                if(test.is_open())
                {
                    rawAssembly.clear();
                    fileCorrelations.clear();
                    sc.loadInstructions(filename);
                    sc.pc = 0;
                }
                else
                {
                    std::cout << "Unable to open file '" << filename << "'\n";
                }
            }
            else if(command.substr(0, 3) == "ld ") //Load machine code
            {
                std::string filename = removeLeadingWhitespace(csCommand.substr(2));
                std::ifstream test(filename);
                if(test.is_open())
                {
                    sc.loadData(filename);
                    sc.pc = 0;
                }
                else
                {
                    std::cout << "Unable to open file '" << filename << "'\n";
                }
            }
            else if(command.substr(0, 3) == "la ") //Load machine code
            {
                try
                {
                    std::ifstream assemblyFile(removeLeadingWhitespace(csCommand.substr(2)));
                    if(assemblyFile.is_open())
                    {
                        std::string currentLine;
                        while(getline(assemblyFile, currentLine))
                        {
                            rawAssembly.push_back(currentLine);
                        }
                        std::pair<std::vector<uint16_t>, std::vector<size_t>> assembledCode = assembleFile(removeLeadingWhitespace(csCommand.substr(2)));
                        sc.loadInstructions(assembledCode.first);
                        fileCorrelations = assembledCode.second;
                        sc.pc = 0;
                    }
                    else
                    {
                        std::cout << "Unable to open file '" << (removeLeadingWhitespace(csCommand.substr(2))) << "'\n";
                    }
                }
                catch(...)
                {
                    std::cout << "Error loading assembly, Simple Computer data may be corrupt. A reset is encouraged\n";
                }
            }
            else if(command == "x") //Reset
            {
                sc.reset();
            }
            else if(command == "q")
            {
                break;
            }
            else if(command == "h" || command.substr(0, 2) == "h ")
            {
                if(command.size() == 1)
                {
                    printHelp("");
                }
                else
                {
                    printHelp(removeLeadingWhitespace(command.substr(1)));
                }
            }
            else
            {
                std::cout << "Invalid command. Type 'h' for help\n";
            }
        }
    }
    
    return(0);
}
