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
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

/*custom dynamic array instead of using vector
    by:Nursyahirah Aqilah
*/
template <class T>
class DynamicArray
{
    private:
        T*  data;
        int capacity;
        int length;

        void resize()
        {
            int newCap   = capacity * 2;
            T*  newData  = new T[newCap];
            for (int i = 0; i < length; i++) newData[i] = data[i];
            delete[] data;
            data     = newData;
            capacity = newCap;
        }

    public:
        DynamicArray() : capacity(8), length(0)
        {
            data = new T[capacity];
        }

        ~DynamicArray() { delete[] data; }

        void push_back(const T &val)
        {
            if (length >= capacity) resize();
            data[length++] = val;
        }

        T& operator[](int idx)       { return data[idx]; }
        const T& operator[](int idx) const { return data[idx]; }
        int size() const { return length; }
        bool empty() const { return length == 0; }

        void clear()
        {
            delete[] data;
            capacity = 8;
            length   = 0;
            data     = new T[capacity];
        }
};



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
            carryOut -the bit that was shifted out (for CF)
        */
        void updateShiftFlags(signed char result, bool carryOut = false)
        {
            zeroFlag = (result == 0);
            carryFlag = carryOut;
            // OF and UF are not affected by shift/rotate
        }

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

        void updateDecrementFlags(signed char result, signed char original)
        {
            zeroFlag = (result == 0);
            overflowFlag = (result < -128);
            underflowFlag = (result < -125);
            carryFlag = ((original == -128 && result == 127));
        }

        /*  flags specifically for ROL operation */
        void updateROLFlags(signed char result, int shiftCount)
        {
            zeroFlag = (result == 0);
            if (shiftCount == 1) {
                carryFlag = (result & 0x80) != 0;
            }
            // OF and UF not affected
        }

        /*  flags specifically for ROR operation */
        void updateRORFlags(signed char result, int shiftCount)
        {
            zeroFlag = (result == 0);
            if (shiftCount == 1) {
                carryFlag = (result & 0x01) != 0;
            }
            // OF and UF not affected
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
            if (!buffer || bufferSize < 1) return;

            char temp[64];
            int pos = 0;

            // OF
            temp[pos++] = 'O';
            temp[pos++] = 'F';
            temp[pos++] = '#';
            temp[pos++] = overflowFlag ? '1' : '0';
            temp[pos++] = '#';

            // UF
            temp[pos++] = 'U';
            temp[pos++] = 'F';
            temp[pos++] = '#';
            temp[pos++] = underflowFlag ? '1' : '0';
            temp[pos++] = '#';

            // CF
            temp[pos++] = 'C';
            temp[pos++] = 'F';
            temp[pos++] = '#';
            temp[pos++] = carryFlag ? '1' : '0';
            temp[pos++] = '#';

            // ZF
            temp[pos++] = 'Z';
            temp[pos++] = 'F';
            temp[pos++] = '#';
            temp[pos++] = zeroFlag ? '1' : '0';
            temp[pos++] = '#';

            temp[pos] = '\0';

            int copyLen = (pos < bufferSize) ? pos : (bufferSize - 1);
            for (int i = 0; i < copyLen; i++) {
                buffer[i] = temp[i];
            }
            buffer[copyLen] = '\0';
        }

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

        void executeINC()
        {
            signed char destReg = memory.read(PC++);
            signed char original = registers[destReg].getValue();
            signed char result = original + 1;
            registers[destReg].setValue(result);
            flags.updateIncrementFlags(result, original);
        }

        void executeDEC()
        {
            signed char destReg = memory.read(PC++);
            signed char original = registers[destReg].getValue();
            signed char result = original - 1;
            registers[destReg].setValue(result);
            flags.updateDecrementFlags(result, original);
        }

        void executeLOAD()
        {
            signed char destReg = memory.read(PC++);
            signed char mode    = memory.read(PC++);
            signed char operand = memory.read(PC++);

            int address;
            if (mode == 1) address = (int)registers[(int)operand].getValue();
            else address = (int)operand;

            signed char value = memory.read(address);
            registers[destReg].setValue(value);
            flags.updateInputFlags(value);
        }

        void executeSTORE()
        {
            signed char mode    = memory.read(PC++);
            signed char operand = memory.read(PC++);
            signed char srcReg  = memory.read(PC++);

            int address;
            if (mode == 1) address = (int)registers[(int)operand].getValue();
            else address = (int)operand;

            memory.write(address, registers[srcReg].getValue());
        }

        void executeROL()
        {
            signed char destReg = memory.read(PC++);
            signed char count = memory.read(PC++);

            unsigned char bits = (unsigned char)registers[destReg].getValue();
            int shiftBy = ((int)count) % 8;
            if (shiftBy < 0) shiftBy += 8;

            bool carryOut = false;
            unsigned char result = bits;

            if (shiftBy != 0) {
                carryOut = (bits & (0x80 >> (shiftBy - 1))) != 0;
                result = (unsigned char)((bits << shiftBy) | (bits >> (8 - shiftBy)));
            }

            signed char finalValue = (signed char)result;
            registers[destReg].setValue(finalValue);
            flags.updateShiftFlags(finalValue, carryOut);
        }

        void executeROR()
        {
            signed char destReg = memory.read(PC++);
            signed char count = memory.read(PC++);

            unsigned char bits = (unsigned char)registers[destReg].getValue();
            int shiftBy = ((int)count) % 8;
            if (shiftBy < 0) shiftBy += 8;

            bool carryOut = false;
            unsigned char result = bits;

            if (shiftBy != 0) {
                carryOut = (bits & (0x01 << (shiftBy - 1))) != 0;
                result = (unsigned char)((bits >> shiftBy) | (bits << (8 - shiftBy)));
            }

            signed char finalValue = (signed char)result;
            registers[destReg].setValue(finalValue);
            flags.updateShiftFlags(finalValue, carryOut);
        }

        void executeSHL()
        {
            signed char destReg = memory.read(PC++);
            signed char count = memory.read(PC++);

            unsigned char bits = (unsigned char)registers[destReg].getValue();
            int shiftBy = (int)count;
            bool carryOut = false;

            unsigned char result;
            if (shiftBy >= 8 || shiftBy < 0) {
                result = 0;
                carryOut = false;
            } else if (shiftBy == 0) {
                result = bits;
                carryOut = false;
            } else {
                carryOut = (bits & (0x80 >> (shiftBy - 1))) != 0;
                result = (unsigned char)(bits << shiftBy);
            }

            signed char finalValue = (signed char)result;
            registers[destReg].setValue(finalValue);
            flags.updateShiftFlags(finalValue, carryOut);
        }

        void executeSHR()
        {
            signed char destReg = memory.read(PC++);
            signed char count = memory.read(PC++);

            unsigned char bits = (unsigned char)registers[destReg].getValue();
            int shiftBy = (int)count;
            bool carryOut = false;

            unsigned char result;
            if (shiftBy >= 8 || shiftBy < 0) {
                result = 0;
                carryOut = false;
            } else if (shiftBy == 0) {
                result = bits;
                carryOut = false;
            } else {
                carryOut = (bits & (0x01 << (shiftBy - 1))) != 0;
                result = (unsigned char)(bits >> shiftBy);
            }

            signed char finalValue = (signed char)result;
            registers[destReg].setValue(finalValue);
            flags.updateShiftFlags(finalValue, carryOut);
        }

        void executeADDI()
        {
            signed char destReg = memory.read(PC++);
            signed char imm = memory.read(PC++);
            signed char val1 = registers[destReg].getValue();
            signed char result = val1 + imm;
            registers[destReg].setValue(result);
            flags.updateArithmeticFlags(result, val1, imm);
        }
        void executeSUBI()
        {
            signed char destReg = memory.read(PC++);
            signed char imm = memory.read(PC++);
            signed char val1 = registers[destReg].getValue();
            signed char result = val1 - imm;
            registers[destReg].setValue(result);
            flags.updateArithmeticFlags(result, val1, -imm);
        }
        void executeMULI()
        {
            signed char destReg = memory.read(PC++);
            signed char imm = memory.read(PC++);
            signed char val1 = registers[destReg].getValue();
            signed char result = val1 * imm;
            registers[destReg].setValue(result);
            flags.updateArithmeticFlags(result, val1, imm);
        }
        void executeDIVI()
        {
            signed char destReg = memory.read(PC++);
            signed char imm = memory.read(PC++);
            signed char val1 = registers[destReg].getValue();
            if (imm == 0) throw runtime_error("ALU Error: Division by zero detected!");
            signed char result = val1 / imm;
            registers[destReg].setValue(result);
            flags.updateArithmeticFlags(result, val1, imm);
        }

        /* INPUT <DestReg> : reads & validates keyboard input.
        DISPLAY <SrcReg> : prints register's value to terminal
        By: NURSYAHIRAH */
        void executeINPUT()
        {
            signed char destReg = memory.read(PC++);

            cout << endl << "?";
            int rawInput;
            cin >> rawInput;

            while (cin.fail() || rawInput < -128 || rawInput > 127)
            {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Error: Please enter a value between -128 and 127." << endl;
                cout << "?";
                cin >> rawInput;
            }

            signed char value = (signed char)rawInput;
            registers[destReg].setValue(value);
            flags.updateInputFlags(value);
        }

        void executeDISPLAY()
        {
            signed char srcReg = memory.read(PC++);
            cout << (int)registers[srcReg].getValue() << endl;
        }

        void executeRESET()
        {
            signed char flagCode = memory.read(PC++);

            switch (flagCode)
            {
                case 0: flags.resetFlag('O'); break;   // Reset Overflow Flag
                case 1: flags.resetFlag('U'); break;   // Reset Underflow Flag
                case 2: flags.resetFlag('C'); break;   // Reset Carry Flag
                case 3: flags.resetFlag('Z'); break;   // Reset Zero Flag
                default:
                    throw "Unknown flag code for RESET instruction.";
            }
        }
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
                RESET = 0x13,     // reset flag
                ADDI = 0x14,     // add register with immediate number
                SUBI = 0x15,     // subtract register with immediate number
                MULI = 0x16,     // multiply register with immediate number
                DIVI = 0x17,      // divide register with immediate number
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
                    case LDI: executeLDI();
                        break;

                    case ADD: executeADD();
                        break;

                    case PUSH: executePUSH();
                        break;

                    case POP: executePOP();
                        break;

                    case MOV: executeMOV();
                        break;

                    case SUB: executeSUB();
                        break;

                    case MUL: executeMUL();
                        break;

                    case DIV: executeDIV();
                        break;

                    case INC: executeINC();
                        break;

                    case DEC: executeDEC();
                        break;

                    case LOAD:executeLOAD();
                        break;

                    case STORE: executeSTORE();
                        break;

                    case ROL: executeROL();
                        break;

                    case ROR: executeROR();
                        break;

                    case SHL: executeSHL();
                        break;

                    case SHR: executeSHR();
                        break;

                    // syahirah
                    case INPUT: executeINPUT();
                        break;

                    case DISPLAY: executeDISPLAY();
                        break;

                    case RESET: executeRESET();
                        break;

                    case ADDI: executeADDI();
                        break;

                    case SUBI: executeSUBI();
                        break;

                    case MULI: executeMULI();
                        break;

                    case DIVI: executeDIVI();
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
                if (buffer && bufferSize > 0) buffer[0] = '\0';
                return;
            }

            int pos = 0;

            // Helper lambda to append to buffer
            auto append = [&](const char* str) {
                int len = 0;
                while (str[len] != '\0') len++;
                for (int i = 0; i < len && pos < bufferSize - 1; i++) {
                    buffer[pos++] = str[i];
                }
            };

            // Helper to append integer with formatting
            auto appendInt = [&](int value, int width) {
                if (value < 0) {
                    append("-");
                    value = -value;
                    width--;
                }
                // Count digits
                int digits = 0;
                int temp = value;
                if (temp == 0) digits = 1;
                else { while (temp > 0) { digits++; temp /= 10; } }

                int padding = width - digits;
                for (int i = 0; i < padding && pos < bufferSize - 1; i++) {
                    buffer[pos++] = '0';
                }

                if (value == 0) {
                    buffer[pos++] = '0';
                } else {
                    char tempBuf[16];
                    int idx = 0;
                    temp = value;
                    while (temp > 0) {
                        tempBuf[idx++] = '0' + (temp % 10);
                        temp /= 10;
                    }
                    for (int i = idx - 1; i >= 0 && pos < bufferSize - 1; i--) {
                        buffer[pos++] = tempBuf[i];
                    }
                }
            };

            append("#Begin#\n");
            append("#Registers#\n");

            for (int i = 0; i < 8; i++) {
                int val = registers[i].getValue();
                appendInt(val, 4);
                if (i < 7) append("#");
            }
            append("#\n");

            append("#Flags#\n");
            // Use getFlagsString for flags
            char flagBuf[64];
            flags.getFlagsString(flagBuf, 64);
            append(flagBuf);
            append("\n");

            append("#PC#\n");
            appendInt(PC, 4);
            append("#\n");

            append("#Memory#\n");
            for (int i = 0; i < memory.getSize(); i++) {
                int val = memory.read(i);
                appendInt(val, 4);
                if ((i + 1) % 8 == 0) {
                    append("#\n");
                } else {
                    append("#");
                }
            }

            append("#End#\n");
            buffer[pos] = '\0';
        }

};

/*
===========================================================================================
PART:             Runner (Assembler / Interpreter front-end)
Written by:       NURSYAHIRAH AQILAH BINTI AINUL HISHAM
Responsibility:   - Reads the .asm text file line by line into a DynamicArray<string>
                     (custom dynamic array, no STL vector)
                   - Detects empty lines (ignored) and multiple instructions on one
                     line (error + exit)
                   - Converts each assembly line into opcode + operand bytes
                     ("assembling") and loads the resulting byte stream into the
                     VirtualMachine's Memory via loadProgram()
                   - Runs the VirtualMachine and dumps the final state to the
                     terminal in the exact required "#Begin# ... #End#" format
===========================================================================================
*/

class Runner
{
    private:
        VirtualMachine vm;
        DynamicArray<string> sourceLines;  //raw lines to read from .asm file
        DynamicArray<signed char> program; // assembled opcode/operand byte stream

        static string trim(const string &s)
        {
            int start = 0;
            int end = (int)s.size() -1;
            while(start <= end && isspace((unsigned char)s[start]))
                start ++;
            while(end >= start && isspace((unsigned char)s[end]))
                end --;
            if(end<start)
                return "";
            return s.substr(start, end - start +1);
        }

        static string toUpper(const string &s)
        {
            string result = s;
            for(size_t i=0; i < result.size(); i++)
                result[i] = (char)toupper((unsigned char)result[i]);
            return result;
        }

        static string stripBrackets(const string &s, bool &hadBrackets)
            {
                string t = trim(s);
                hadBrackets = false;
                if (t.size() >= 2 && t.front() == '[' && t.back() == ']')
                {
                    hadBrackets = true;
                    return trim(t.substr(1, t.size() - 2));
                }
                return t;
            }

        static bool isRegisterToken(const string &s)
        {
            string t = toUpper(trim(s));
            if (t.size() != 2) return false;
            if (t[0] != 'R') return false;
            if (t[1] < '0' || t[1] > '7') return false;
            return true;
        }

        static int registerIndex(const string &s)
        {
            string t = toUpper(trim(s));
            return t[1] - '0';
        }

        static bool isNumberToken(const string &s)
        {
            string t = trim(s);
            if (t.empty()) return false;
            size_t i = 0;
            if (t[0] == '+' || t[0] == '-') i = 1;
            if (i >= t.size()) return false;
            for (; i < t.size(); i++)
            {
                if (!isdigit((unsigned char)t[i])) return false;
            }
            return true;
        }

        static int splitOperands(const string &operandText, string &op1, string &op2)
        {
            string t = trim(operandText);
            if (t.empty()) return 0;

            int commaPos = -1;
            for (size_t i = 0; i < t.size(); i++)
            {
                if (t[i] == ',') { commaPos = (int)i; break; }
            }

            if (commaPos == -1)
            {
                op1 = trim(t);
                return 1;
            }

            op1 = trim(t.substr(0, commaPos));
            op2 = trim(t.substr(commaPos + 1));
            return 2;
        }

        static void resolveAddressOperand(const string &operand, signed char &mode, signed char &value)
        {
            bool hadBrackets = false;
            string inner = stripBrackets(operand, hadBrackets);

            if (isRegisterToken(inner))
            {
                mode = 1;
                value = (signed char)registerIndex(inner);
            }
            else if (isNumberToken(inner))
            {
                mode = 0;
                value = (signed char)atoi(inner.c_str());
            }
            else
            {
                throw invalid_argument("Invalid address/register operand: '" + operand + "'");
            }
        }

        static bool isMnemonic(const string &token)
        {
            string t = toUpper(trim(token));
            const char* known[] = {
                "HLT","MOV","ADD","SUB","MUL","DIV","INC","DEC","ROL","ROR","SHL","SHR","LOAD","STORE","PUSH","POP","INPUT","DISPLAY","RESET", "LDI", "SUBI", "ADDI", "MULI", "DIVI"
            };
            for (int i = 0; i < sizeof(known)/sizeof(known[0]); i++)
            {
                if (t == known[i]) return true;
            }
            return false;
        }

        void assembleLine(const string &rawLine, int lineNumber)
        {
            string line = trim(rawLine);

            size_t spacePos = line.find_first_of(" \t");
            string mnemonic = (spacePos == string::npos) ? line : line.substr(0, spacePos);
            string rest = (spacePos == string::npos) ? "" : line.substr(spacePos + 1);
            mnemonic = toUpper(trim(mnemonic));

            {
                int mnemonicCount = isMnemonic(mnemonic) ? 1 : 0;
                string tmp = rest;
                for (size_t i = 0; i < tmp.size(); i++) { if (tmp[i] == ',') tmp[i] = ' '; }
                stringstream ss(tmp);
                string tok;
                while (ss >> tok)
                {
                    if (isMnemonic(tok)) mnemonicCount++;
                }
                if (mnemonicCount > 1)
                {
                    cout << "Assembler Error (line " << lineNumber
                         << "): more than one instruction found on a single line." << endl;
                    exit(1);
                }
            }

            string op1, op2;
            int opCount = splitOperands(rest, op1, op2);

            if (mnemonic == "HLT")
            {
                program.push_back((signed char)0x00);
            }
            else if (mnemonic == "LDI")
            {
                if (opCount != 2 || !isRegisterToken(op1) || !isNumberToken(op2))
                    throw invalid_argument("LDI requires: LDI Rn, immediate");

                program.push_back(0x01);
                program.push_back((signed char)registerIndex(op1));
                program.push_back((signed char)atoi(op2.c_str()));
            }
            else if (mnemonic == "MOV")
            {
                if (opCount != 2 || !isRegisterToken(op1))
                {
                    throw invalid_argument("MOV requires a destination register and a source operand.");
                }
                bool hadBrackets = false;
                string inner = stripBrackets(op2, hadBrackets);

                if (hadBrackets)
                {
                    signed char mode, value;
                    resolveAddressOperand(op2, mode, value);
                    program.push_back((signed char)0x0B); // LOAD
                    program.push_back((signed char)registerIndex(op1));
                    program.push_back(mode);
                    program.push_back(value);
                }
                else if (isRegisterToken(op2))
                {
                    program.push_back((signed char)0x05); // MOV
                    program.push_back((signed char)registerIndex(op1));
                    program.push_back((signed char)registerIndex(op2));
                }
                else if (isNumberToken(op2))
                {
                    program.push_back((signed char)0x01); // LDI
                    program.push_back((signed char)registerIndex(op1));
                    program.push_back((signed char)atoi(op2.c_str()));
                }
                else
                {
                    throw invalid_argument("Invalid source operand for MOV: '" + op2 + "'");
                }
            }
            else if (mnemonic == "ADD" || mnemonic == "SUB" || mnemonic == "MUL" || mnemonic == "DIV")
            {
                if (opCount != 2 || !isRegisterToken(op1))
                {
                    throw invalid_argument(mnemonic + " requires a destination register and a register/immediate operand.");
                }

                if (isRegisterToken(op2))
                {
                    signed char opcode = (mnemonic == "ADD") ? 0x02 :
                                          (mnemonic == "SUB") ? 0x06 :
                                          (mnemonic == "MUL") ? 0x07 : 0x08;
                    program.push_back(opcode);
                    program.push_back((signed char)registerIndex(op1));
                    program.push_back((signed char)registerIndex(op2));
                }
                else if (isNumberToken(op2))
                {
                    signed char opcode = (mnemonic == "ADD") ? 0x14 :
                                          (mnemonic == "SUB") ? 0x15 :
                                          (mnemonic == "MUL") ? 0x16 : 0x17;
                    program.push_back(opcode);
                    program.push_back((signed char)registerIndex(op1));
                    program.push_back((signed char)atoi(op2.c_str()));
                }
                else
                {
                    throw invalid_argument("Invalid second operand for " + mnemonic + ": '" + op2 + "'");
                }
            }
            else if (mnemonic == "ADDI" || mnemonic == "SUBI" || mnemonic == "MULI" || mnemonic == "DIVI")
            {
                if (opCount != 2 || !isRegisterToken(op1) || !isNumberToken(op2))
                {
                    throw invalid_argument(mnemonic + " requires: Rn, immediate");
                }

                signed char opcode =
                    (mnemonic == "ADDI") ? 0x14 :
                    (mnemonic == "SUBI") ? 0x15 :
                    (mnemonic == "MULI") ? 0x16 :
                                            0x17;

                program.push_back(opcode);
                program.push_back((signed char)registerIndex(op1));
                program.push_back((signed char)atoi(op2.c_str()));
            }
            else if (mnemonic == "INC" || mnemonic == "DEC")
            {
                if (opCount != 1 || !isRegisterToken(op1))
                {
                    throw invalid_argument(mnemonic + " requires one register.");
                }
                program.push_back((signed char)(mnemonic == "INC" ? 0x09 : 0x0A));
                program.push_back((signed char)registerIndex(op1));
            }
            else if (mnemonic == "ROL" || mnemonic == "ROR" || mnemonic == "SHL" || mnemonic == "SHR")
            {
                if (opCount != 2 || !isRegisterToken(op1) || !isNumberToken(op2))
                {
                    throw invalid_argument(mnemonic + " requires a register and a count.");
                }
                signed char opcode = (mnemonic == "ROL") ? 0x0D :
                                      (mnemonic == "ROR") ? 0x0E :
                                      (mnemonic == "SHL") ? 0x0F : 0x10;
                program.push_back(opcode);
                program.push_back((signed char)registerIndex(op1));
                program.push_back((signed char)atoi(op2.c_str()));
            }
            else if (mnemonic == "LOAD")
            {
                if (opCount != 2 || !isRegisterToken(op1))
                {
                    throw invalid_argument("LOAD requires a destination register and an address/register.");
                }
                signed char mode, value;
                resolveAddressOperand(op2, mode, value);
                program.push_back((signed char)0x0B);
                program.push_back((signed char)registerIndex(op1));
                program.push_back(mode);
                program.push_back(value);
            }
            else if (mnemonic == "STORE")
            {
                if (opCount != 2)
                {
                    throw invalid_argument("STORE requires two operands.");
                }
                bool op1Bracketed = false, op2Bracketed = false;
                string op1Inner = stripBrackets(op1, op1Bracketed);
                string op2Inner = stripBrackets(op2, op2Bracketed);

                string addrOperand, regOperand;
                if (op1Bracketed || isNumberToken(op1Inner))
                {
                    addrOperand = op1; regOperand = op2;
                }
                else
                {
                    addrOperand = op2; regOperand = op1;
                }

                if (!isRegisterToken(regOperand))
                {
                    throw invalid_argument("STORE could not identify the source register.");
                }

                signed char mode, value;
                resolveAddressOperand(addrOperand, mode, value);
                program.push_back((signed char)0x0C);
                program.push_back(mode);
                program.push_back(value);
                program.push_back((signed char)registerIndex(regOperand));
            }
            else if (mnemonic == "PUSH" || mnemonic == "POP")
            {
                if (opCount != 1 || !isRegisterToken(op1))
                {
                    throw invalid_argument(mnemonic + " requires one register.");
                }
                program.push_back((signed char)(mnemonic == "PUSH" ? 0x03 : 0x04));
                program.push_back((signed char)registerIndex(op1));
            }
            else if (mnemonic == "INPUT" || mnemonic == "DISPLAY")
            {
                if (opCount != 1 || !isRegisterToken(op1))
                {
                    throw invalid_argument(mnemonic + " requires one register.");
                }
                program.push_back((signed char)(mnemonic == "INPUT" ? 0x11 : 0x12));
                program.push_back((signed char)registerIndex(op1));
            }
            else if (mnemonic == "RESET")
            {
                if (opCount != 1)
                {
                    throw invalid_argument("RESET requires one flag name (OF, UF, CF, or ZF).");
                }
                string flagName = toUpper(trim(op1));
                signed char flagCode;
                if (flagName == "OF") flagCode = 0;
                else if (flagName == "UF") flagCode = 1;
                else if (flagName == "CF") flagCode = 2;
                else if (flagName == "ZF") flagCode = 3;
                else throw invalid_argument("Unknown flag for RESET: '" + op1 + "'");

                program.push_back((signed char)0x13);
                program.push_back(flagCode);
            }
            else
            {
                throw invalid_argument("Unknown instruction mnemonic: '" + mnemonic + "'");
            }
        }

    public:
        Runner() {}
        bool loadFile(const string &filename)
        {
            ifstream file(filename.c_str());
            if (!file.is_open())
            {
                cout << "Error: Could not open assembly file '" << filename << "'." << endl;
                return false;
            }

            string line;
            while (getline(file, line))
            {
                size_t commentPos = line.find(';');
                if (commentPos != string::npos) line = line.substr(0, commentPos);

                line = trim(line);
                if (line.empty()) continue; // empty lines are ignored

                sourceLines.push_back(line);
            }

            file.close();
            return true;
        }

        void assemble()
        {
            for (int i = 0; i < sourceLines.size(); i++)
            {
                assembleLine(sourceLines[i], i + 1);
            }
        }

        void execute()
        {
            int size = program.size();
            signed char* bytes = new signed char[size > 0 ? size : 1];
            for (int i = 0; i < size; i++) bytes[i] = program[i];

            vm.loadProgram(bytes, size);
            delete[] bytes;

            cout << "=== Running VirtualMachine.asm ===" << endl;
            vm.run();
            cout << endl;

            vm.printState();
            cout << endl;

            const int bufSize = 4096;
            char buffer[bufSize];
            vm.getFormattedState(buffer, bufSize);
            cout << buffer;
        }
};

int main()
    {
        Runner runner;

        if (!runner.loadFile("VirtualMachine.asm"))
        {
            return 1;
        }

        try
        {
            runner.assemble();
            runner.execute();
        }
        catch (const exception &e)
        {
            cout << "Runtime/Assembler Error: " << e.what() << endl;
            return 1;
        }

        return 0;
    }
