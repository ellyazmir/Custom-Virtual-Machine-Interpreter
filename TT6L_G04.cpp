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
*/



#include <iostream>
#include <stdexcept>
#include <string>

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

        // exception handling using throw std::overflow_error
        void push(signed char value, signed char &SI) override
        {
            if (SI >= STACK_MAX)
            {
                throw std::overflow_error("VM Stack Overflow! Maximum stack capacity of 8 bytes exceeded.");
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
                std::cout << "System Error: Stack Underflow detected! Virtual Machine crashed." << std::endl;
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
            std::cout << "Generic Register Value: " << (int)value << std::endl;
        }
};

// GeneralRegister inherits from Register (inheritance #2)
class GeneralRegister : public Register
{
    private:
        std::string registerName; // tracks register token identity (example: "R0")

    public:
        GeneralRegister() : Register(), registerName("UNKNOWN") {}
        GeneralRegister(std::string name) : Register(), registerName(name) {}

        std::string getName() const { return this->registerName; }

        // override keyword used to achieve polymorphism (polymorphism #2)
        void printStatus() const override
        {
            std::cout << registerName << " = " << (int)value << std::endl;
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
                throw std::out_of_range("Memory Access Error: Attempted to read outside valid range (0-63).");
            }
            return ram[address];
        }

        void write(int address, signed char value)
        {
            if (address < 0 || address >= MAX_SIZE)
            {
                throw std::out_of_range("Memory Access Error: Attempted to write outside valid range (0-63).");
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
            std::cout << "ZF=" << (zeroFlag ? "1" : "0") << " ";
            std::cout << "CF=" << (carryFlag ? "1" : "0") << " ";
            std::cout << "OF=" << (overflowFlag ? "1" : "0") << " ";
            std::cout << "UF=" << (underflowFlag ? "1" : "0") << std::endl;
        }

        // Get flags as formatted string
        std::string getFlagsString() const
        {
            return std::to_string(zeroFlag ? 1 : 0) + "#" +
                   std::to_string(carryFlag ? 1 : 0) + "#" +
                   std::to_string(overflowFlag ? 1 : 0) + "#" +
                   std::to_string(underflowFlag ? 1 : 0);
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
                throw std::runtime_error("ALU Error: Division by zero detected!");
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
            std::cout << "=== VM State ===" << std::endl;
            for (int i = 0; i < 4; i++) registers[i].printStatus();
            flags.displayFlags();
            std::cout << "PC: " << PC << ", SI: " << (int)SI << std::endl;
        }
};


int main()
{
   //goodluck gng

    return 0;
}