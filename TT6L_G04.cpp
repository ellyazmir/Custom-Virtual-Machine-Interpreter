/*
=================================================================================
PART:             Custom Data Structures
Written by:       ELLY MAZLIN
Lectures Covered: 1, 2, 3, 4, 7, 8
Responsibility:   Simulating core VM hardware components (Memory, Stack, Registers)
=================================================================================
=================================================================================
PART:             Hardware Lead
Written by:       AMIRA SOFIA
Lectures Covered: 1,2,3,4,7,8
Responsibility:   Track ZF, CF, OF, UF flags
=================================================================================
=================================================================================
PART:             Logic Arithmatic Lead
Written by:       MUHAMMAD YUSOF BIN SHAHILAN
Lectures Covered: 1,2,3,4,7,8
Responsibility:   Implementing instruction set, opcode parsing, and execution logic
=================================================================================
=================================================================================
PART:             Runner (interpreter)
Written by:       NURSYAHIRAH AQILAH BINTI AINUL HISHAM
Lectures Covered: 1,2,3,4,7,8
Responsibility:   Read .asm file, convert each line into opcode + operand
                   bytes, load into VirtualMachine memory, run, dump state
=================================================================================
*/



#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

// CLASS TEMPLATES (applied to custom stack structure)
template <class T>
class BaseStack
{
    public:
        virtual void push(T value, signed char &SI) = 0;
        virtual T pop(signed char &SI) = 0;
        virtual ~BaseStack() {}
};

// VMStack inherits from BaseStack template (inheritance #1)
class VMStack : public BaseStack<signed char>
{
    private:
        static const int STACK_MAX = 8;
        signed char stackArray[STACK_MAX];

    public:
        VMStack()
        {
            for (int i = 0; i < STACK_MAX; i++) stackArray[i] = 0;
        }

        // exception handling using throw overflow_error
        void push(signed char value, signed char &SI) override
        {
            if (SI >= STACK_MAX)
            {
                throw overflow_error("VM Stack Overflow! Maximum stack capacity of 8 bytes exceeded.");
            }
            stackArray[(int)SI] = value;
            SI++; // increment Stack Index register after push operation
        }

        // exception handling using throw std::underflow_error
        signed char pop(signed char &SI) override
        {
            if (SI <= 0)
            {
                // underflow triggers system crash according to project specs
                cout << "System Error: Stack Underflow detected! Virtual Machine crashed." << endl;
                exit(1);
            }
            SI--; // decrement Stack Index register before popping to point to top element
            return stackArray[(int)SI];
        }
};

// INHERITANCE & POLYMORPHISM (#2)
class Register
{
    protected:
        signed char value;  // 1 byte, signed (-128 to 127)

    public:
        Register() : value(0) {}
        virtual ~Register() {} // virtual destructor for safe polymorphic cleanups

        void setValue(signed char val) { this->value = val; }
        signed char getValue() const { return this->value; }

        // virtual function for runtime polymorphism
        virtual void printStatus() const
        {
            cout << "Generic Register Value: " << (int)value << endl;
        }
};

// GeneralRegister inherits from Register (inheritance #2)
class GeneralRegister : public Register
{
    private:
        string registerName; // tracks register token identity (example: "R0")

    public:
        GeneralRegister() : Register(), registerName("UNKNOWN") {}
        GeneralRegister(string name) : Register(), registerName(name) {}

        string getName() const { return this->registerName; }

        // override keyword used to achieve polymorphism (polymorphism #2)
        void printStatus() const override
        {
            cout << registerName << " = " << (int)value << endl;
        }
};

// DATA ABSTRACTION (enforcing the wall via encapsulation)
class Memory
{
    private:
        static const int MAX_SIZE = 64; // ceiling at 64 bytes memory space
        signed char ram[MAX_SIZE];

    public:
        Memory()
        {
            // safe array initialization
            for (int i = 0; i < MAX_SIZE; i++) ram[i] = 0;
        }

        // exception handling for out of bounds access
        signed char read(int address)
        {
            if (address < 0 || address >= MAX_SIZE)
            {
                throw out_of_range("Memory Access Error: Attempted to read outside valid range (0-63).");
            }
            return ram[address];
        }

        void write(int address, signed char value)
        {
            if (address < 0 || address >= MAX_SIZE)
            {
                throw out_of_range("Memory Access Error: Attempted to write outside valid range (0-63).");
            }
            ram[address] = value;
        }
};

// HARDWARE LEAD: Flag Register for ZF, CF, OF, UF
class FlagRegister
{
    private:
        bool zeroFlag;      // ZF - Zero Flag
        bool carryFlag;     // CF - Carry Flag
        bool overflowFlag;  // OF - Overflow Flag
        bool underflowFlag; // UF - Underflow Flag

    public:
        FlagRegister() : zeroFlag(false), carryFlag(false),
                         overflowFlag(false), underflowFlag(false) {}

        // Setter methods for flags
        void setZeroFlag(bool value) { zeroFlag = value; }
        void setCarryFlag(bool value) { carryFlag = value; }
        void setOverflowFlag(bool value) { overflowFlag = value; }
        void setUnderflowFlag(bool value) { underflowFlag = value; }

        // Getter methods for flags
        bool getZeroFlag() const { return zeroFlag; }
        bool getCarryFlag() const { return carryFlag; }
        bool getOverflowFlag() const { return overflowFlag; }
        bool getUnderflowFlag() const { return underflowFlag; }

        // Reset all flags
        void resetFlags()
        {
            zeroFlag = false;
            carryFlag = false;
            overflowFlag = false;
            underflowFlag = false;
        }

        // Update flags based on arithmetic operation result
        void updateArithmeticFlags(signed char result, signed char operand1, signed char operand2)
        {
            // ZF: Set if result is zero
            zeroFlag = (result == 0);

            // CF: Set for unsigned carry/borrow (simplified)
            int extendedResult = (int)operand1 + (int)operand2;
            carryFlag = (extendedResult > 127 || extendedResult < -128);

            // OF: Set if result exceeds 125 (box quota max)
            overflowFlag = (result > 125);

            // UF: Set if result is below -125 (box quota min)
            underflowFlag = (result < -125);
        }

        // Update flags for input validation
        void updateInputFlags(signed char value)
        {
            zeroFlag = (value == 0);
            overflowFlag = (value > 125);
            underflowFlag = (value < -125);
            carryFlag = false; // Clear carry for input operations
        }

        // Display current flag status
        void displayFlags() const
        {
            cout << "ZF=" << (zeroFlag ? "1" : "0") << " ";
            cout << "CF=" << (carryFlag ? "1" : "0") << " ";
            cout << "OF=" << (overflowFlag ? "1" : "0") << " ";
            cout << "UF=" << (underflowFlag ? "1" : "0") << endl;
        }

        // Get flags as formatted string
        string getFlagsString() const
        {
            return to_string(zeroFlag ? 1 : 0) + "#" +
                   to_string(carryFlag ? 1 : 0) + "#" +
                   to_string(overflowFlag ? 1 : 0) + "#" +
                   to_string(underflowFlag ? 1 : 0);
        }
};

class VirtualMachine
{
    private:
        Memory memory; // 64 bytes of memory space
        VMStack stack; // custom stack structure with push/pop and overflow/underflow handling
        GeneralRegister registers[4]; // 4 general-purpose registers (R0, R1, R2, R3)
        FlagRegister flags; // flag register for ZF, CF, OF, UF

        int PC;
        signed char SI;
        bool isRunning;

        void executeLDI()
        {
            signed char regIndex = memory.read(PC++);
            signed char value = memory.read(PC++);
            registers[regIndex].setValue(value);
            flags.updateInputFlags(value);
        }// Arithmetic operation with flag updates
        void executeADD()
        {
            signed char destReg = memory.read(PC++);
            signed char srcReg = memory.read(PC++);
            signed char val1 = registers[destReg].getValue();
            signed char val2 = registers[srcReg].getValue();
            signed char result = val1 + val2;
            registers[destReg].setValue(result);
            flags.updateArithmeticFlags(result, val1, val2);
        }// Stack operations with overflow/underflow handling

        void executeMOV()
        {
            signed char destReg = memory.read(PC++);
            signed char srcReg = memory.read(PC++);

            // Copy value from source to destination
            signed char valueToCopy = registers[srcReg].getValue();
            registers[destReg].setValue(valueToCopy);
            flags.updateInputFlags(valueToCopy);
        }
        void executeSUB()
        {
            signed char destReg = memory.read(PC++);
            signed char srcReg = memory.read(PC++);
            signed char val1 = registers[destReg].getValue();
            signed char val2 = registers[srcReg].getValue();
            signed char result = val1 - val2;
            registers[destReg].setValue(result);
            flags.updateArithmeticFlags(result, val1, -val2); // -val2 for subtraction logic
        }
        void executeMUL()
        {
            signed char destReg = memory.read(PC++);
            signed char srcReg = memory.read(PC++);
            signed char val1 = registers[destReg].getValue();
            signed char val2 = registers[srcReg].getValue();
            signed char result = val1 * val2;
            registers[destReg].setValue(result);
            flags.updateArithmeticFlags(result, val1, val2);
        }
        void executeDIV()
        {
            signed char destReg = memory.read(PC++);
            signed char srcReg = memory.read(PC++);
            signed char val1 = registers[destReg].getValue();
            signed char val2 = registers[srcReg].getValue();

            // Hardware safety check: Prevent divide by zero crash
            if (val2 == 0) {
                throw runtime_error("ALU Error: Division by zero detected!");
            }
            signed char result = val1 / val2;
            registers[destReg].setValue(result);
            flags.updateArithmeticFlags(result, val1, val2);
        }
        void executePUSH()
        {
            signed char regIndex = memory.read(PC++);
            stack.push(registers[regIndex].getValue(), SI);
        }// Stack operations with overflow/underflow handling

        void executePOP()
        {
            signed char regIndex = memory.read(PC++);
            registers[regIndex].setValue(stack.pop(SI));
        }// Additional instruction implementations (SUB, MUL, DIV, etc.) can be added here with appropriate flag updates

    public:
        // Opcode definitions for instruction set
        enum Opcodes {
            HLT = 0x00, LDI = 0x01, ADD = 0x02, PUSH = 0x03, POP = 0x04,
            MOV = 0x05, SUB = 0x06, MUL = 0x07, DIV = 0x08
        };

        VirtualMachine() : PC(0), SI(0), isRunning(false)
        {
            registers[0] = GeneralRegister("R0");
            registers[1] = GeneralRegister("R1");
            registers[2] = GeneralRegister("R2");
            registers[3] = GeneralRegister("R3");
        }// Method to load a program (array of signed char opcodes) into memory
        void loadProgram(const signed char* program, int size)
        {
            for (int i = 0; i < size; i++) memory.write(i, program[i]);
            PC = 0;
        }// Main execution loop of the virtual machine

        void run()
        {
            isRunning = true;
            while (isRunning)
            {
                signed char opcode = memory.read(PC++); // FETCH
                switch (opcode) // DECODE & EXECUTE
                {
                    case HLT:  isRunning = false; break;
                    case LDI:  executeLDI();      break;
                    case ADD:  executeADD();      break;
                    case PUSH: executePUSH();     break;
                    case POP:  executePOP();      break;
                    case MOV:  executeMOV();      break;
                    case SUB:  executeSUB();      break;
                    case MUL:  executeMUL();      break;
                    case DIV:  executeDIV();      break;
                    default:   isRunning = false; break;
                }
            }
        }
        void printState() const
        {
            cout << "=== VM State ===" << endl;
            for (int i = 0; i < 4; i++) registers[i].printStatus();
            flags.displayFlags();
            cout << "PC: " << PC << ", SI: " << (int)SI << endl;
        }
};

class Runner
{
private:
    VirtualMachine vm;
    vector<string> rawLines;

    //removes leading and trailing spaces
    string trim(string s)
    {
        int start = 0;
        int end = s.length() - 1;

        while(start <= end && s[start] == ' ')
            start++;

        while(end >= start && s[end] == ' ')
            end--;

        if (start > end)
            return "";

        return s.substr(start, end - start + 1);
    }

    //changes 'Rx' to 'x'
    int regIndex(string token)
    {
        char digitChar = token[1];
        int digit = digitChar - '0';
        return digit;
    }

    //check if the token is a valid register
    bool isRegister(string token)
    {
        return(token.length() >= 2 && token[0] == 'R');
    }

    //split instruction from 'Rx, y' into ['Rx', '5']
    vector<string> split(string text, char delimiter)
    {
        vector<string> result;
        stringstream stream(text);
        string piece;

        while(getline(stream, piece, delimiter))
        {
            result.push_back(trim(piece));
        }

        return result;
    }

public:
    //read .asm file, skip empty lines
    void loadProgram(string filename)
    {
        ifstream file(filename);

        if(!file.is_open())
        {
            cout << "Error: cannot open file " << filename << endl;
            exit(1);
        }

        string line;
        while(getline(file, line))
        {
            string cleanLine = trim(line);

            if(cleanLine == "")
                continue;

            rawLines.push_back(cleanLine);
        }

        file.close();
    }

    //comvert each line into opcode + operand bytes and lpad into VM's memory
    void assemble()
    {
        vector<signed char> bytecode;

        for(int i=0; i < (int)rawLines.size(); i++)
        {
            string line = rawLines[i];

            stringstream lineStream(line);
            string opcode;
            string restOfLine;

            lineStream >> opcode;
            getline(lineStream, restOfLine);

            vector<string> operands = split(trim(restOfLine), ',');

            if(opcode == "MOV")
            {
                int destReg = regIndex(operands[0]);
                string secondOperand = operands[1];

                if(isRegister(secondOperand))
                {
                    bytecode.push_back(VirtualMachine::MOV);
                    bytecode.push_back((signed char)destReg);
                    bytecode.push_back((signed char)regIndex(secondOperand));
                }
                else
                {
                    bytecode.push_back(VirtualMachine::LDI);
                    bytecode.push_back((signed char)destReg);
                    bytecode.push_back((signed char)stoi(secondOperand));
                }
            }
            else if(opcode == "ADD")
            {
                bytecode.push_back(VirtualMachine::ADD);
                bytecode.push_back((signed char)regIndex(operands[0]));
                bytecode.push_back((signed char)regIndex(operands[1]));
            }
            else if(opcode == "SUB")
            {
                bytecode.push_back(VirtualMachine::SUB);
                bytecode.push_back((signed char)regIndex(operands[0]));
                bytecode.push_back((signed char)regIndex(operands[1]));
            }
            else if(opcode == "MUL")
            {
                bytecode.push_back(VirtualMachine::MUL);
                bytecode.push_back((signed char)regIndex(operands[0]));
                bytecode.push_back((signed char)regIndex(operands[1]));
            }
            else if(opcode == "DIV")
            {
                bytecode.push_back(VirtualMachine::DIV);
                bytecode.push_back((signed char)regIndex(operands[0]));
                bytecode.push_back((signed char)regIndex(operands[1]));
            }
            else if(opcode == "PUSH")
            {
                bytecode.push_back(VirtualMachine::PUSH);
                bytecode.push_back((signed char)regIndex(operands[0]));
            }
            else if(opcode == "POP")
            {
                bytecode.push_back(VirtualMachine::POP);
                bytecode.push_back((signed char)regIndex(operands[0]));
            }
            else
            {
                cout << "Error: unknown instruction '" << opcode << "'" << endl;
                exit(1);
            }
        }

        bytecode.push_back(VirtualMachine::HLT);

        vm.loadProgram(bytecode.data(), (int)bytecode.size());
    }

    //run the program
    void run()
    {
        vm.run();
    }

    //print final state of the virtual machine
    void dumpState()
    {
        vm.printState();
    }

};

int main()
{
   Runner runner;

   runner.loadProgram("VirtualMachine.asm");
   runner.assemble();
   runner.run();
   runner.dumpState();

    return 0;
}
