/*
=================================================================================
PART:             Custom Data Structures
Written by:       ELLY MAZLIN
Lectures Covered: 1, 2, 3, 4, 7, 8
Responsibility:   Simulating core VM hardware components (Memory, Stack, Registers)
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

// BITWISE FLAGS REGISTER (Amira's part interface integration)
class FlagRegister
{
    private:
        unsigned char FLAGS; // individual flags packed neatly into a single raw byte

    public:
        FlagRegister() : FLAGS(0) {}

        // bitwise configurations conforming to Amira's logic specifications
        static const unsigned char BIT_ZF = 1 << 0; // Zero Flag
        static const unsigned char BIT_CF = 1 << 1; // Carry Flag
        static const unsigned char BIT_OF = 1 << 2; // Overflow Flag
        static const unsigned char BIT_UF = 1 << 3; // Underflow Flag

        void setFlag(unsigned char flagBit, bool value)
        {
            if (value) FLAGS |= flagBit;       // bitwise OR assignment
            else FLAGS &= ~flagBit;            // bitwise AND + NOT bit-clear
        }

        bool getFlag(unsigned char flagBit) const
        {
            return (FLAGS & flagBit) != 0;     // bitwise checking returning boolean
        }

        void resetFlags() { FLAGS = 0; }
        unsigned char getRawFlags() const { return FLAGS; }
};
