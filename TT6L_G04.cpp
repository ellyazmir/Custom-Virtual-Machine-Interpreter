/*
===========================================================================================
PART             :Custom Data Structures
Written by       :ELLY MAZLIN BINTI MOHD AZMIR (252UC241RN)
Responsibility   :Simulating core VM hardware components:
                  - Memory (64 bytes with bounds checking)
                  - Stack (8 bytes with overflow/underflow handling)
                  - Register (base class for all registers)
                  - GeneralRegister (derived class for R0-R7)
                  - FlagRegister (ZF, CF, OF, UF flags)
                  - VirtualMachine (main controller - coordinate all components)

OOP Concepts Covered:
1. Encapsulation      :All class members are private, access via public getters/setters
2. Inheritance        :VMStack inherits from BaseStack<T> (template inheritance)
                       GeneralRegister inherits from Register (class inheritance)
3. Polymorphism       :Virtual functions (printStatus(), isEqual()) with override keyword
4. Composition        :VirtualMachine owns Memory, VMStack, FlagRegister (strong ownership)
5. Aggregation        :VirtualMachine contains array of GeneralRegister objects
6. Abstraction        :Abstract BaseStack<T> class with pure virtual functions
7. Exception Handling :throw/catch for overflow, underflow, out_of_range
8. Deep Copy          :Proper copy constructors & assignment operators implemented
===========================================================================================
PART:             Hardware Lead
Written by:       AMIRA SOFIA
Lectures Covered: 1,2,3,4,7,8
Responsibility:   Track ZF, CF, OF, UF flags
===========================================================================================
===========================================================================================
PART:             Logic Arithmatic Lead
Written by:       MUHAMMAD YUSOF BIN SHAHILAN
Lectures Covered: 1,2,3,4,7,8
Responsibility:   Implementing instruction set, opcode parsing, and execution logic
===========================================================================================
===========================================================================================
PART:             Runner (interpreter)
Written by:       NURSYAHIRAH AQILAH BINTI AINUL HISHAM
Lectures Covered: 1,2,3,4,7,8
Responsibility:   Read .asm file, convert each line into opcode + operand
                   bytes, load into VirtualMachine memory, run, dump state
===========================================================================================
*/



#include <iostream>
#include <stdexcept>    // exception classes: overflow_error, underflow_error, out_of_range
#include <cstring>      // c-string functions (strcpy, strlen, strncpy)
#include <cstdlib>      // atoi, exit, NULL

using namespace std;


/*  To abstract base class for stack data structure
    template class (provide interface for stack operations)
    By: ELLY MAZLIN
*/
template <class T>
class BaseStack
{
    public:
        // pure virtual functions (makes this an abstract class)
        virtual void push(T value, signed char &SI) = 0;
        virtual T pop(signed char &SI) = 0;
        virtual ~BaseStack() {} // for safe polymorphic cleanup
};

/*  Custom stack implementation for VM
    inherits from BaseStack<signed char> (INHERITANCE #1)
        - uses composition with fixed-size array
        - throws exceptions for overflow/underflow
    By: ELLY MAZLIN
*/
class VMStack : public BaseStack<signed char>
{
    private:
        static const int STACK_MAX = 8;
        signed char stackArray[STACK_MAX];

    public:
        VMStack()   // initialize stack to zero
        {
            for (int i = 0; i < STACK_MAX; i++) stackArray[i] = 0;
        }

        /*  push a value onto the stack
            value -the signed char to push, SI -Stack Index register (passed by reference)
        */
        void push(signed char value, signed char &SI) override
        {
            if (SI >= STACK_MAX)            // check for stack overflow
            {
                throw overflow_error("VM Stack Overflow! Maximum stack capacity of 8 bytes exceeded.");
            }
            stackArray[(int)SI] = value;    // store value at current SI position
            SI++;                           // increment Stack Index after push
        }

        /*  pop a value from the top of the stack
            SI -Stack Index register (passed by reference)
            system crashes on underflow
        */
        signed char pop(signed char &SI) override
        {
            if (SI <= 0)    // check for underflow
            {
                cout << "System Error: Stack Underflow detected! Virtual Machine crashed." << endl;
                exit(1);
            }
            SI--;                       // decrement SI before popping to point to top element
            return stackArray[(int)SI]; // return value at the new SI position
        }

        /*  view value at specific index without modifying SI
            index -position in stack
        */
        signed char peek(int index) const
        {
            if (index < 0 || index >= STACK_MAX)
            {
                throw out_of_range("Stack index out of range"); // index invalid
            }
            return stackArray[index];
        }

        // check if stack is empty
        bool isEmpty() const
        {
            for (int i = 0; i < STACK_MAX; i++)
            {
                if (stackArray[i] != 0) return false;
            }
            return true;    // all stack elements = 0
        }

        // check if stack is full
        bool isFull(signed char SI) const {return (SI >= STACK_MAX);}
};

/*  Encapsulates 8-bit signed value
    base class for GeneralRegister
        - contains protected member 'value'
        - provides virtual functions for polymorphism
    By: ELLY MAZLIN
*/
class Register  // base class
{
    protected:
        signed char value;  // 1 byte, signed (-128 to 127)

    public:
        Register() : value(0) {}                    // default: initialize value to 0
        Register(signed char val) : value(val) {}   // parameterized: initialize value to specified value
        virtual ~Register() {}                      // for safe polymorphic cleanup

        /*  set the register value
            val -signed char to store
        */
        void setValue(signed char val) {this->value = val;}

        // get the register value
        signed char getValue() const {return this->value;}

        /*  display register status
            overridden in derived classes (POLYMORPHISM #1)
        */
        virtual void printStatus() const
        {
            cout << "Generic Register Value: " << (int)value << endl;
        }

        /*  compare 2 registers
            overridden in derived classes (POLYMORPHISM #2)
        */
        virtual bool isEqual(const Register& other) const
        {
            return this->value == other.value;
        }
};

/*  Represents R0-R7 general purpose registers
    inherit from Register (INHERITANCE #2)
        - adds register name
        - overrides virtual functions (POLYMORPHISM #1 & #2)
        - implement deep copy (copy constructor + assignment)
    By: ELLY MAZLIN
*/
class GeneralRegister : public Register
{
    private:
        char registerName[20];

    public:

        GeneralRegister() : Register() {strcpy(registerName, "UNKNOWN");} // default: name "UNKNOWN", value 0
        
        /*  parameterized :initialize with given name, value 0
            parameter     :name (c-string name for the register)
        */
        GeneralRegister(const char* name) : Register()
        {
            if (strlen(name) < 20) {strcpy(registerName, name);} // safe string copy with bounds checking
            else
            {
                strncpy(registerName, name, 19);
                registerName[19] = '\0';    // null terminate
            }
        }
        
        // copy: deep copy of another GeneralRegister
        GeneralRegister(const GeneralRegister& other) : Register()
        {
            this->value = other.value;
            strcpy(registerName, other.registerName);
        }
        
        // deep copy assignment
        GeneralRegister& operator=(const GeneralRegister& other)
        {
            if (this != &other)  // check for self-assignment
            {
                this->value = other.value;
                strcpy(registerName, other.registerName);
            }
            return *this;
        }

        ~GeneralRegister() {} // clean up (no dynamic memory, but included for completeness)

        
        //  get the register name
        const char* getName() const {return registerName;}

        /*  set the register name
            name (c-string name)
        */
        void setName(const char* name)
        {
            if (strlen(name) < 20)
            {
                strcpy(registerName, name);
            }
            else
            {
                strncpy(registerName, name, 19);
                registerName[19] = '\0';
            }
        }        
        
        /*  display register name & value
            overrides base class virtual function (POLYMORPHISM #1)
        */
        void printStatus() const override
        {
            cout << registerName << " = " << (int)value << endl;
        }

        /*  compare 2 GeneralRegister objects
            overrides base class virtual function (POLYMORPHISM #2)
        */
        bool isEqual(const Register& other) const override
        {
            // try to cast to GeneralRegister
            const GeneralRegister* otherPtr = dynamic_cast<const GeneralRegister*>(&other);
            if (otherPtr)
            {
                return (this->value == otherPtr->value &&
                       strcmp(this->registerName, otherPtr->registerName) == 0);    // return true = value & name match
            }
            return false;
        }
};

/*  64-byte main memory with bounds checking
    composed into VirtualMachine (COMPOSITION - strong ownership)
        - fixed-size array (NO STL VECTOR)
        - throws out_of_range for invalid addresses
        - implements deep copy (copy constructor + assignment)
    By: ELLY MAZLIN
*/
class Memory
{
    private:
        static const int MAX_SIZE = 64; // max memory size
        signed char ram[MAX_SIZE];

    public:
        Memory()  // default: all memory to 0
        {
            for (int i = 0; i < MAX_SIZE; i++) {ram[i] = 0;}
        }

        // copy: deep copy of another Memory object
        Memory(const Memory& other)
        {
            for (int i = 0; i < MAX_SIZE; i++) { ram[i] = other.ram[i]; }
        }   

        // deep copy assignment
        Memory& operator=(const Memory& other)
        {
            if (this != &other)
            {
                for (int i = 0; i < MAX_SIZE; i++) { ram[i] = other.ram[i]; }
            }
            return *this;
        }

        ~Memory() {} // clean up (no dynamic memory, but included for completeness)
    
    
        /*  read a byte from memory at specified address
            address -memory address (0-63)
        */
        signed char read(int address) const
        {
            if (address < 0 || address >= MAX_SIZE)
            {
                throw out_of_range("Memory Access Error: Attempted to read outside valid range (0-63).");
            }
            return ram[address];
        }

        /*  write a byte to memory at specified address
            address -memory address (0-63), value -signed char to write
        */
        void write(int address, signed char value)
        {
            // bounds checking
            if (address < 0 || address >= MAX_SIZE)
            {
                throw out_of_range("Memory Access Error: Attempted to write outside valid range (0-63).");
            }
            ram[address] = value;
        }

        // get memory size
        int getSize() const { return MAX_SIZE; }

        void clear() // set all memory to 0
        {
            for (int i = 0; i < MAX_SIZE; i++) { ram[i] = 0; }
        }

        /*  reset a specific memory region to 0
            start -starting address, end -ending address
        */
        void resetRegion(int start, int end)
        {
            if (start < 0 || end >= MAX_SIZE || start > end)
            {
                throw out_of_range("Invalid memory region");
            }
            
            for (int i = start; i <= end; i++) { ram[i] = 0; }
        }

        // display memory contents (for debugging)
        void dumpMemory() const
        {
            for (int i = 0; i < MAX_SIZE; i++)
            {
                cout << (int)ram[i] << " ";
                if ((i + 1) % 8 == 0)
                   cout << endl;
            }
        }
};

/*  Manages ZF, CF, OF, UF flags
    composed into VirtualMachine (COMPOSITION - strong ownership)
        - 4 boolean flags
        - methods to update flags for different operations
        - reset & display capabilities
    By: ELLY MAZLIN (Structure: variables, getters, setters, display)
        AMIRA SOFIA (Hardware Lead - updateArithmeticFlags, updateInputFlags,
                                     updateShiftFlags, updateIncrementFlags,
                                     resetFlag)
*/
class FlagRegister
{
    private:
        bool zeroFlag;      // ZF - result = 0
        bool carryFlag;     // CF - carry/borrow
        bool overflowFlag;  // OF - result > 125
        bool underflowFlag; // UF - result < -125

    public:
        // default: all flags to false
        FlagRegister() : zeroFlag(false), carryFlag(false),
                         overflowFlag(false), underflowFlag(false) {}

        // copy flags from another FlagRegister
        FlagRegister(const FlagRegister& other) // deep copy
        {
            zeroFlag = other.zeroFlag;
            carryFlag = other.carryFlag;
            overflowFlag = other.overflowFlag;
            underflowFlag = other.underflowFlag;
        }

        // assignment operator
        FlagRegister& operator=(const FlagRegister& other)  // deep copy
        {
            if (this != &other)
            {
                zeroFlag = other.zeroFlag;
                carryFlag = other.carryFlag;
                overflowFlag = other.overflowFlag;
                underflowFlag = other.underflowFlag;
            }
            return *this;
        }

        ~FlagRegister() {}  // clean up (no dynamic memory)

        // SET individual flag values
        void setZeroFlag(bool value) { zeroFlag = value; }
        void setCarryFlag(bool value) { carryFlag = value; }
        void setOverflowFlag(bool value) { overflowFlag = value; }
        void setUnderflowFlag(bool value) { underflowFlag = value; }

        // GET individual flag values
        bool getZeroFlag() const { return zeroFlag; }
        bool getCarryFlag() const { return carryFlag; }
        bool getOverflowFlag() const { return overflowFlag; }
        bool getUnderflowFlag() const { return underflowFlag; }

        void resetFlags()   // reset all flags to false
        {
            zeroFlag = false;
            carryFlag = false;
            overflowFlag = false;
            underflowFlag = false;
        }

        /*  reset a specific flag to false
            flagType - 'Z', 'C', 'O', or 'U'
        */
        void resetFlag(char flagType)
        {
            switch(flagType)
            {
                case 'Z': zeroFlag = false; break;
                case 'C': carryFlag = false; break;
                case 'O': overflowFlag = false; break;
                case 'U': underflowFlag = false; break;
                default:
                    throw invalid_argument("Unknown flag type. Use Z, C, O, or U.");
            }
        }

        /*  update flags based on arithmetic operation result
            result -result of arithmetic operation, operand1 -first operand, operand2 -second operand
        */
        void updateArithmeticFlags(signed char result, signed char operand1, signed char operand2)
        {
            zeroFlag = (result == 0);
            int extendedResult = (int)operand1 + (int)operand2;
            carryFlag = (extendedResult > 127 || extendedResult < -128);    // exceeds 8-bit signed range
            overflowFlag = (result > 125);
            underflowFlag = (result < -125);
        }

        /*  update flags based on input value
            value -input value
        */
        void updateInputFlags(signed char value)
        {
            zeroFlag = (value == 0);
            overflowFlag = (value > 125);
            underflowFlag = (value < -125);
            carryFlag = false; // clear carry for input operations
        }

        /*  update flags for shift/rotate operations
            result -result of shift/rotate operation
        */
        void updateShiftFlags(signed char result) {zeroFlag = (result == 0);} // only ZF is affected by shift

        /*  update flags for increment/decrement operations
            result -result of increment/decrement operation, original -original value before operation
        */
        void updateIncrementFlags(signed char result, signed char original)
        {
            zeroFlag = (result == 0);
            overflowFlag = (result > 127);
            underflowFlag = (result < -128);
            // carry on wrap-around
            carryFlag = ((original == 127 && result == -128) ||
                        (original == -128 && result == 127));
        }

        // display all flag values in readable format
        void displayFlags() const
        {
            cout << "ZF=" << (zeroFlag ? "1" : "0") << " ";
            cout << "CF=" << (carryFlag ? "1" : "0") << " ";
            cout << "OF=" << (overflowFlag ? "1" : "0") << " ";
            cout << "UF=" << (underflowFlag ? "1" : "0") << endl;
        }

        /*  get flags as formatted string for file output
            buffer -character buffer to store output, bufferSize -size of buffer
            format - OF#0#UF#0#CF#0#ZF#0#
        */
        void getFlagsString(char* buffer, int bufferSize) const
        {
            if (buffer && bufferSize >= 16)
            {
                snprintf(buffer, bufferSize, "%d#%d#%d#%d#",
                         overflowFlag ? 1 : 0,
                         underflowFlag ? 1 : 0,
                         carryFlag ? 1 : 0,
                         zeroFlag ? 1 : 0);
            }
        }

        // check if any flag is set
        bool anyFlagSet() const
        {
            return zeroFlag || carryFlag || overflowFlag || underflowFlag; // true = at least one flag true
        }

        // check if all flags are clear
        bool allFlagsClear() const
        {
            return !zeroFlag && !carryFlag && !overflowFlag && !underflowFlag; // true = all flag false
        }
};

/*  Main controller (coordinates all VM components)
    COMPOSITION: owns Memory, VMStack, FlagRegister
    AGGREGATION: contains array of GeneralRegister
    coordinates: PC, SI, & execution flow
    By: ELLY MAZLIN     (Core structure, constructors, getters, 
                        reset, loadProgram, formatted output, printState)
        MUHAMMAD YUSOF  (ALU instruction methods:
                        executeLDI, executeADD, executeMOV, executeSUB, executeMUL,
                        executeDIV, executePUSH, executePOP, executeINC, executeDEC,
                        executeLOAD, executeSTORE, executeROL, executeROR,
                        executeSHL, executeSHR)
        AMIRA SOFIA     (RESET instruction - executeRESET)
        NURSYAHIRAH     (I/O instructions: executeINPUT, executeDISPLAY, file I/O)            
*/
class VirtualMachine
{
    private:
        // HARDWARE COMPONENTS (composition - strong ownership)
        Memory memory;                // 64 bytes of main memory
        VMStack stack;                // custom stack with overflow/underflow protection
        GeneralRegister registers[8]; // 8 general-purpose registers (R0-R7)
        FlagRegister flags;           // flag register for ZF, CF, OF, UF

        // CONTROL REGISTERS
        int PC;         // Program Counter (starts at 0)
        signed char SI; // Stack Index (starts at 0)
        bool isRunning; // execution state

        /*  These methods implement individual instructions
            By  -  YUSOF   :ALU instructions (ADD, SUB, MUL, DIV, INC, DEC,
                            ROL, ROR, SHL, SHR, LOAD, STORE, MOV)
                - AMIRA    :RESET instruction (flag handling)
                - SYAHIRAH :INPUT, DISPLAY instructions
        */
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
        /*  default: all VM components to initial state
                - PC = 0, SI = 0, isRunning = false
                - initialize 8 registers (R0-R7)
                - memory & flags already initialized via their constructors
        */
        VirtualMachine() : PC(0), SI(0), isRunning(false)
        {
            registers[0] = GeneralRegister("R0");
            registers[1] = GeneralRegister("R1");
            registers[2] = GeneralRegister("R2");
            registers[3] = GeneralRegister("R3");
            registers[4] = GeneralRegister("R4");
            registers[5] = GeneralRegister("R5");
            registers[6] = GeneralRegister("R6");
            registers[7] = GeneralRegister("R7");
        }        

        // copy: deep copy of another VirtualMachine
        VirtualMachine(const VirtualMachine& other)
                      : memory(other.memory), stack(other.stack), flags(other.flags),
                        PC(other.PC), SI(other.SI), isRunning(other.isRunning)
        {
            for (int i = 0; i < 8; i++) { registers[i] = other.registers[i]; }
        }

        // deep copy assignment
        VirtualMachine& operator=(const VirtualMachine& other)
        {
            if (this != &other)
            {
                memory = other.memory;
                stack  = other.stack;
                flags  = other.flags;
                PC     = other.PC;
                SI     = other.SI;
                isRunning = other.isRunning;
                
                for (int i = 0; i < 8; i++) { registers[i] = other.registers[i]; }
            }
            return *this;
        }

        ~VirtualMachine() {} // clean up (no dynamic memory, but included for completeness)
        
        
        /*  SECTION getter methods (other team members to access components)
            :To provide controlled access to private components
        */

        // get reference to Memory object
        Memory& getMemory() {return memory;}
        const Memory& getMemory() const {return memory;}

        // get reference to VMStack object
        VMStack& getStack() {return stack;}
        const VMStack& getStack() const {return stack;}

        // get reference to FlagRegister object
        FlagRegister& getFlags() {return flags;}
        const FlagRegister& getFlags() const {return flags;}

        /*  get reference to a specific GeneralRegister by index
            index -register number (0-7)
        */
        GeneralRegister& getRegister(int index)
        {
            if (index < 0 || index >= 8)
            {
                throw out_of_range("Register index out of range (0-7)");
            }
            return registers[index];
        }

        const GeneralRegister& getRegister(int index) const
        {
            if (index < 0 || index >= 8)
            {
                throw out_of_range("Register index out of range (0-7)");
            }
            return registers[index];
        }

        // Get/Set Program Counter value
        int getPC() const {return PC;}
        void setPC(int value) {PC = value;}

        // Get/Set Stack Index value
        signed char getSI() const {return SI;}
        void setSI(signed char value) {SI = value;}

        // Get/Set execution state
        bool isRunningState() const {return isRunning;}
        void setRunningState(bool state) {isRunning = state;}

        /*  reset entire VM to initial state
                - clear memory
                - reset all flags
                - PC = 0, SI = 0, isRunning = false
                - all registers set to 0
        */
        void reset()
        {
            memory.clear();
            flags.resetFlags();

            PC = 0;
            SI = 0;
            isRunning = false;

            for (int i = 0; i < 8; i++) { registers[i].setValue(0); }
        }
        
        /*  load a program (opcodes) into memory
            program -pointer to signed char array, size -number of bytes in program
        */
        void loadProgram(const signed char* program, int size)
        {
            if (size > 64)
            {
                throw out_of_range("Program size exceeds memory capacity (64 bytes)");
            }

            for (int i = 0; i < size; i++) { memory.write(i, program[i]); }
        
            PC = 0;
            isRunning = false;
        }

        /*  load a program (opcodes) into memory (for opcode values 0-255)
            program -pointer to unsigned char array, size -number of bytes in program
        */
        void loadProgram(const unsigned char* program, int size)
        {
            if (size > 64)
            {
                throw out_of_range("Program size exceeds memory capacity (64 bytes)");
            }
        
            for (int i = 0; i < size; i++) { memory.write(i, (signed char)program[i]); }

            PC = 0;
            isRunning = false;
        }        

        /*  execute the loaded program
                - FETCH:   read opcode from memory[PC]
                - DECODE:  determine which instruction to execute
                - EXECUTE: call appropriate execution method
                - continue until HLT or error
            implementations by - YUSOF    :ALU operations
                               - AMIRA    :RESET instruction
                               - SYAHIRAH :INPUT, DISPLAY
        */
        void run()
        {
            isRunning = true;

            enum Opcodes
            {
                HLT = 0x00,      // halt execution
                LDI = 0x01,      // load Immediate (MOV immediate in spec)
                ADD = 0x02,      // add 2 registers
                PUSH = 0x03,     // push register to stack
                POP = 0x04,      // pop from stack to register
                MOV = 0x05,      // move (reg-to-reg, immediate, memory)
                SUB = 0x06,      // subtract 2 registers
                MUL = 0x07,      // multiply 2 registers
                DIV = 0x08,      // divide 2 registers
                INC = 0x09,      // increment register
                DEC = 0x0A,      // decrement register
                LOAD = 0x0B,     // load from memory
                STORE = 0x0C,    // store to memory
                ROL = 0x0D,      // rotate left
                ROR = 0x0E,      // rotate right
                SHL = 0x0F,      // shift left
                SHR = 0x10,      // shift right
                INPUT = 0x11,    // input from keyboard
                DISPLAY = 0x12,  // display to screen
                RESET = 0x13     // reset flag
            };

            while (isRunning)
            {
                signed char opcode = memory.read(PC++); // FETCH read the opcode from memory at current PC

                switch (opcode) // DECODE & EXECUTE
                {
                    // halt
                    case HLT:
                        isRunning = false;
                        break;

                    // yusof
                    case LDI:
                        break;

                    case ADD:
                        break;

                    case PUSH:
                        break;

                    case POP:
                        break;

                    case MOV:
                        break;

                    case SUB:
                        break;

                    case MUL:
                        break;

                    case DIV:
                        break;

                    case INC:
                        break;

                    case DEC:
                        break;

                    case LOAD:
                        break;

                    case STORE:
                        break;

                    case ROL:
                        break;

                    case ROR:
                        break;

                    case SHL:
                        break;

                    case SHR:
                        break;

                    // syahirah
                    case INPUT:
                        break;

                    case DISPLAY:
                        break;

                    case RESET:
                        break;

                    default:
                        cout << "Error: Unknown opcode 0x" << hex << (int)opcode
                             << " at PC=" << dec << (PC - 1) << endl;
                        isRunning = false;
                        break;
                }
            }
        }

        //  print current VM state in human-readable format (for debugging)
        void printState() const
        {
            cout << "=== Virtual Machine State ===" << endl;
            cout << "Registers:" << endl;
        
            for (int i = 0; i < 8; i++) { registers[i].printStatus(); }
        
            cout << "Flags: ";
            flags.displayFlags();
            
            cout << "PC: " << PC << ", SI: " << (int)SI << endl;
            cout << "==============================" << endl;
        }

        /*  get VM state in required output format for file export
            buffer -character buffer to store formatted output, bufferSize -size of buffer
        */
        void getFormattedState(char* buffer, int bufferSize) const
        {
            if (!buffer || bufferSize < 2048)
            {
                if (buffer && bufferSize > 0) buffer[0] = '\0'; // buffer too small or invalid

                return;
            }

            int pos = 0;

            pos += snprintf(buffer + pos, bufferSize - pos, "#Begin#\n");
            pos += snprintf(buffer + pos, bufferSize - pos, "#Registers#\n");

            for (int i = 0; i < 8; i++) // print all 8 registers with 4-digit format
            {
                int val = registers[i].getValue();

                // handle negative values
                if (val < 0)
                {
                    pos += snprintf(buffer + pos, bufferSize - pos, "-%03d", -val);
                }
                else
                {
                    pos += snprintf(buffer + pos, bufferSize - pos, "%04d", val);
                }

                // add # separator (except after last register)
                if (i < 7)
                    pos += snprintf(buffer + pos, bufferSize - pos, "#");
            }
            pos += snprintf(buffer + pos, bufferSize - pos, "#\n");
            pos += snprintf(buffer + pos, bufferSize - pos, "#Flags#\n");
            pos += snprintf(buffer + pos, bufferSize - pos,
                            "OF#%d#UF#%d#CF#%d#ZF#%d#\n",
                            flags.getOverflowFlag() ? 1 : 0,
                            flags.getUnderflowFlag() ? 1 : 0,
                            flags.getCarryFlag() ? 1 : 0,
                            flags.getZeroFlag() ? 1 : 0);

            pos += snprintf(buffer + pos, bufferSize - pos, "#PC#\n");
            pos += snprintf(buffer + pos, bufferSize - pos, "%04d#\n", PC);
            pos += snprintf(buffer + pos, bufferSize - pos, "#Memory#\n");

            for (int i = 0; i < memory.getSize(); i++)
            {
                int val = memory.read(i);

                if (val < 0)
                {
                    pos += snprintf(buffer + pos, bufferSize - pos, "-%03d", -val);
                }
                else
                {
                    pos += snprintf(buffer + pos, bufferSize - pos, "%04d", val);
                }   

                if ((i + 1) % 8 == 0)
                {
                    pos += snprintf(buffer + pos, bufferSize - pos, "#\n");
                }
                else
                {
                    pos += snprintf(buffer + pos, bufferSize - pos, "#");
                }
            }

            pos += snprintf(buffer + pos, bufferSize - pos, "#End#\n");
        }

};

int main()
{
   //goodluck gng

    return 0;
}