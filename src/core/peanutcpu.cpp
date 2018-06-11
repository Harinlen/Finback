#include "peanutmemory.h"

#include "peanutcpu.h"

#include <QDebug>

#define CPU_GET_RS1(ins)    (getInsRs(ins, 24)) //0000xxxx 00000000 00000000 00000000
#define CPU_GET_RS2(ins)    (getInsRs(ins, 20)) //00000000 xxxx0000 00000000 00000000
#define CPU_GET_RS3(ins)    (getInsRs(ins, 16)) //00000000 0000xxxx 00000000 00000000
#define CPU_GET_RS4(ins)    (getInsRs(ins, 12)) //00000000 00000000 xxxx0000 00000000

PeanutCpu::PeanutCpu(QObject *parent) : QObject(parent),
    m_regIR(PEANUT_REG_EMPTY),
    m_regSR(PEANUT_REG_EMPTY),
    m_regSP(PEANUT_STATE_DEFAULT),
    m_regPC(PEANUT_PC_DEFAULT),
    m_memory(nullptr),
    m_halt(false)
{
    m_unaryOps.append(AluNeg);
    m_unaryOps.append(AluNot);
}

void PeanutCpu::reset()
{
    // Reset the value to default value.
    m_regPC = PEANUT_PC_DEFAULT;
    m_regSR = PEANUT_REG_EMPTY;
    m_regIR = PEANUT_REG_EMPTY;
    m_regSP = PEANUT_STATE_DEFAULT;
    m_halt = false;
    // Reset all the registers.
    for(quint8 i=0; i<PEANUT_GENERAL_REGRS; ++i)
    {
        // Reset the general memory state.
        m_regGeneral[i] = PEANUT_REG_EMPTY;
    }
}

void PeanutCpu::loadInstruction()
{
    // Load the instruction from memory.
    readMemory((PEANUT_ADDRESS_TYPE)m_regPC++, &m_regIR);
    qDebug()<<m_regPC;
}

bool PeanutCpu::executeInstruction()
{
    // Execute the instruction.
    bool result = execute(m_regIR);
    // Check the interrupts.
    if(result)
    {
        //!FIXME: add code here.
        ;
    }
    return result;
}

bool PeanutCpu::executeNext()
{
    loadInstruction();
    return executeInstruction();
}

bool PeanutCpu::execute(PEANUT_INS_TYPE instruction)
{
    if(m_halt)
    {
        // Halt, no more execute.
        return false;
    }
    // Parse the instruction.
    // 00000000 00000000 00000000 00000000
    // 11111111 11111111 11110000 00000000
    // |I1||I2| |I3||I4| |I5|
    quint8 I1 = (instruction & 0xF0000000) >> 28,
           I2 = (instruction & 0x0F000000) >> 24,
           I3 = (instruction & 0x00F00000) >> 20,
           I4 = (instruction & 0x000F0000) >> 16;
    switch(I1)
    {
    case 0x0:
        if(instruction==0x00)
        {
            m_halt = true;
            return true;
        }
        // Invalid instruction found.
        return false;
    case 0x1:
        // 0x1 add addition
        return executeAlu(AluAdd, instruction);
    case 0x2:
        // 0x2 sub subtraction
        return executeAlu(AluSub, instruction);
    case 0x3:
        // 0x3 mult multiply
        return executeAlu(AluMult, instruction);
    case 0x4:
        // 0x4 div divide
        return executeAlu(AluDiv, instruction);
    case 0x5:
        // 0x5 mod modulo
        return executeAlu(AluMod, instruction);
    case 0x6:
        // 0x6 and bit and
        return executeAlu(AluAnd, instruction);
    case 0x7:
        // 0x7 or bit or
        return executeAlu(AluOr, instruction);
    case 0x8:
        // 0x8 xor bit xor
        return executeAlu(AluXor, instruction);
    case 0xA:
    {
        switch(I2)
        {
        case 0:
            // 0xA0 neg negate
            return executeAlu(AluNeg, instruction);
        case 1:
            // 0xA1 not bit not
            return executeAlu(AluNot, instruction);
        case 2:{
            // 0xA2 move copy register
            PEANUT_REG_TYPE res = PEANUT_REG_EMPTY;
            if(readInsRs2(instruction, &res))
            {
                return setInsRs3(instruction, res);
            }
            //Failed to read instruction RS1.
            return false;
        }
        case 3:
        {
            // 0xA3x
            switch(I3)
            {
            case 0:
            {
                // 0xA30x
                switch(I4)
                {
                case 0:
                    // 0xA300 call call immediate.
                    if(writeMemory(++m_regSP, m_regPC))
                    {
                        // Update PC.
                        m_regPC = getInsAddr(instruction, 0);
                        // Complete.
                        return true;
                    }
                    return false;
                case 1:
                    if(instruction == 0xA3010000)
                    {
                        // 0xA3010000 return return from call
                        PEANUT_WORD_TYPE previousPc;
                        if(readMemory(m_regSP, &previousPc))
                        {
                            // Update PC.
                            m_regPC = previousPc;
                            --m_regSP;
                            // Complete.
                            return true;
                        }
                        return false;
                    }
                    break;
                case 2:
                    if(instruction==0xA3020000)
                    {
                        // 0xA3020000 trap trap
                        if(writeMemory(++m_regSP, m_regPC))
                        {
                            // Update PC & SR.
                            m_regPC = 0x0002;
                            m_regSR |= (1 << PEANUT_SR_INTERRUPT);
                            // Complete.
                            return true;
                        }
                        return false;
                    }
                    break;
                }
                break;
            }
            }
            break;
        }
        case 4:
        {
            // 0xA4x
            switch(I3)
            {
            case 0:
            {
                //0xA40x
                if(I4==0)
                {
                    //0xA400 jump jump.
                    // Update PC.
                    m_regPC = getInsAddr(instruction, 0);
                    return true;
                }
                break;
            }
            case 1:
            {
                //0xA41 jumpz jump if zero
                PEANUT_REG_TYPE condition;
                if(readInsRs3(instruction, &condition))
                {
                    if(condition == 0x00000000)
                    {
                        // Update PC.
                        m_regPC = getInsAddr(instruction, 0);
                        return true;
                    }
                }
                return false;
            }
            case 2:
            {
                //0xA42 jumpn jump if negative
                PEANUT_REG_TYPE condition;
                if(readInsRs3(instruction, &condition))
                {
                    if(((PEANUT_REG_UTYPE)condition & 0x80000000) != 0x00000000)
                    {
                        // Update PC.
                        m_regPC = getInsAddr(instruction, 0);
                    }
                    return true;
                }
                return false;
            }
            case 3:
            {
                //0xA43 jumpnz jump if not zero
                PEANUT_REG_TYPE condition;
                if(readInsRs3(instruction, &condition))
                {
                    if(condition != 0x00000000)
                    {
                        // Update PC.
                        m_regPC = getInsAddr(instruction, 0);
                    }
                    return true;
                }
                return false;
            }
            }
            break;
        }
        case 5:
        {
            //0xA5x
            switch(I3)
            {
            case 0:
            {
                // 0xA50 reset reset status bit.
                quint8 bit = getInsRs(instruction, 16);
                m_regSR &= ~(1<<bit);
                return true;
            }
            case 1:
            {
                // 0xA51 set set status bit.
                quint8 bit = getInsRs(instruction, 16);
                m_regSR |= (1<<bit);
                return true;
            }
            }
            break;
        }
        case 6:
        {
            //0xA6x
            switch(I3)
            {
            case 0:
            {
                //0xA60 push push onto stack
                PEANUT_REG_TYPE rsValue;
                if(readInsRs3(instruction, &rsValue))
                {
                    // Write the data to memory.
                    return writeMemory(++m_regSP, rsValue);
                }
                return false;
            }
            case 1:
            {
                //0xA61 pop pop from stack
                PEANUT_REG_TYPE rdValue;
                if(!readMemory(m_regSP, (PEANUT_WORD_TYPE *)&rdValue))
                {
                    // Get the register value.
                    setInsRs3(instruction, rdValue);
                    // Reduce the SP register.
                    --m_regSP;
                    return true;
                }
                return false;
            }
            }
            break;
        }
        }
        break;
    }
    case 0xC:
    {
        switch(I2)
        {
        case 0x0:
        {
            //0xC0x
            if(I3==0x0)
            {
                //0xC00 load immediate load from memory.
                // Set the value to register.
                return setInsRs3(instruction,
                                 (qint16)(instruction & 0x0000FFFF));
            }
            break;
        }
        case 0x1:
        {
            // 0xC1x
            if(I3==0x0)
            {
                //0xC10 load absolute load from memory
                PEANUT_ADDRESS_TYPE address = instruction & 0x0000FFFF;
                PEANUT_REG_TYPE value;
                // Read the data from the address.
                if(readMemory(address, (PEANUT_WORD_TYPE *)&value))
                {
                    // Set the data to register.
                    return setInsRs3(instruction, value);
                }
                break;
            }
            break;
        }
        case 0x2:
        {
            if((instruction & 0x0000FFFF)== 0x00000000)
            {
                // 0xC2 load indirectly load from memory.
                PEANUT_ADDRESS_TYPE address;
                if(readInsRs2(instruction, (PEANUT_REG_TYPE *)&address))
                {
                    // Load the data.
                    PEANUT_REG_TYPE value;
                    if(readMemory(address, (PEANUT_WORD_TYPE *)&value))
                    {
                        // Load data to RS3.
                        return setInsRs3(instruction, value);
                    }
                    return false;
                }
                return false;
            }
            break;
        }
        case 0x3:
        {
            //Read from both RS2 and RS3
            PEANUT_ADDRESS_TYPE addressBase;
            // Read the data from instruction.
            if(readInsRs2(instruction, (PEANUT_REG_TYPE *)&addressBase))
            {
                // Load the data.
                PEANUT_REG_TYPE value;
                if(readMemory(addressBase + (instruction & 0x0000FFFF),
                              (PEANUT_WORD_TYPE *)&value))
                {
                    // Load data to RS3.
                    return setInsRs3(instruction, value);
                }
                return false;
            }
            return false;
        }
        }
        break;
    }
    case 0xD:
    {
        switch(I2)
        {
        case 0x1:
        {
            //0xD1 absolute store to memory
            // Save the value.
            PEANUT_REG_TYPE regData;
            if(readInsRs2(instruction, &regData))
            {
                PEANUT_ADDRESS_TYPE address = instruction & 0x0000FFFF;
                return writeMemory(address, (PEANUT_WORD_TYPE)regData);
            }
            return false;
        }
        case 0x2:
        {
            //0xD2 indirect store to memory
            PEANUT_REG_TYPE regData;
            if(readInsRs2(instruction, &regData))
            {
                // Read the address data.
                PEANUT_ADDRESS_TYPE address;
                if(readInsRs3(instruction, (PEANUT_REG_TYPE *)&address))
                {
                    // Write the data.
                    return writeMemory(address, regData);
                }
            }
            return false;
        }
        case 0x3:
        {
            // 0xD3 base + displacement store to memory
            PEANUT_REG_TYPE regData;
            if(readInsRs2(instruction, &regData))
            {
                // Read the address base data.
                PEANUT_ADDRESS_TYPE addressBase;
                if(readInsRs3(instruction, (PEANUT_REG_TYPE *)&addressBase))
                {
                    // Read the extension.
                    PEANUT_ADDRESS_TYPE ext = instruction & 0x0000FFFF;
                    return writeMemory(addressBase + ext, regData);
                }
            }
            return false;
        }
        }
        break;
    }
    case 0xE:
        // 0xE rotate bit left rotate
        return executeAlu(AluRotate, instruction);
    }
    // Failed to parse the instruction.
    m_errorMessage=QString("Failed to parse the instruction: %1").arg(
                QString::number(instruction, 16));
    m_errorIndex=PEANUT_ERR_NOINS;
    return false;
}

void PeanutCpu::setMemory(PeanutMemory *memory)
{
    m_memory = memory;
}

inline bool PeanutCpu::executeAlu(PeanutCpu::AluOperations op,
                                  PEANUT_INS_TYPE instruction)
{
    // Get two register type for calculation.
    PEANUT_REG_TYPE op1 = PEANUT_REG_EMPTY,
                    op2 = PEANUT_REG_EMPTY,
                    res = PEANUT_REG_EMPTY;
    // Get the RS2.
    if(readInsRs2(instruction, &op2))
    {
        // For Unary operations, we can calculate now.
        if(m_unaryOps.contains(op))
        {
            // Execute ALU.
            if(!alu(op, op2, PEANUT_REG_EMPTY, &res))
            {
                return false;
            }
        }
        else
        {
            // For binary values, read the RS1.
            if(readInsRs1(instruction, &op1))
            {
                // Execute ALU.
                if(!alu(op, op1, op2, &res))
                {
                    return false;
                }
            }
            else
            {
                // Failed to read instruction rs1.
                return false;
            }
        }
        // Write the data to rs3 position.
        return setInsRs3(instruction, res);
    }
    // Failed to read instruction rs2.
    return false;
}

inline bool PeanutCpu::alu(AluOperations op,
                           PEANUT_REG_TYPE rs1,
                           PEANUT_REG_TYPE rs2,
                           PEANUT_REG_TYPE *res)
{
    // Check the operation.
    switch(op)
    {
    case AluAdd:
        (*res) = rs1 + rs2;
        return true;
    case AluSub:
        (*res) = rs1 - rs2;
        return true;
    case AluMult:
        (*res) = rs1 * rs2;
        return true;
    case AluDiv:
        if (rs2 == 0)
        {
            m_errorMessage = QString("Divided by zero.");
            m_errorIndex = PEANUT_ERR_DIVBYZERO;
            return false;
        }
        (*res) = rs1 / rs2;
        return true;
    case AluMod:
        if (rs2 == 0)
        {
            m_errorMessage = QString("Modular by zero.");
            m_errorIndex = PEANUT_ERR_DIVBYZERO;
            return false;
        }
        (*res) = rs1 % rs2;
        return true;
    case AluAnd:
        (*res) = rs1 & rs2;
        return true;
    case AluOr:
        (*res) = rs1 | rs2;
        return true;
    case AluXor:
        (*res) = rs1 ^ rs2;
        return true;
    case AluRotate:
        if(rs2 > 0)
        {
            (*res) = (rs1 << rs2) | ((PEANUT_REG_UTYPE)rs1 >> (32 - rs2));
        }
        else
        {
            rs2 = -rs2;
            (*res) = ((PEANUT_REG_UTYPE)rs1 >> rs2) | (rs1 << (32 - rs2));
        }
        return true;
    case AluNeg:
        qDebug()<<"rs1"<<rs1<<"rs2"<<rs2;
        (*res) = -rs1;
        return true;
    case AluNot:
        (*res) = ~rs1;
        return true;
    }
    // Nothing return.
    return false;
}

inline bool PeanutCpu::readReg(quint8 regId, PEANUT_REG_TYPE *target)
{
    switch (regId) {
    case PEANUT_SP_ADDRESS:
        *target = m_regSP;
        return true;
    case PEANUT_SR_ADDRESS:
        *target = m_regSR;
        return true;
    case PEANUT_PC_ADDRESS:
        *target = m_regPC;
        return true;
    case PEANUT_ONE_ADDRESS:
        *target = 1;
        return true;
    case PEANUT_ZERO_ADDRESS:
        *target = 0;
        return true;
    case PEANUT_MONE_ADDRESS:
        *target = -1;
        return true;
    default:
        if (regId < PEANUT_GENERAL_REGRS)
        {
            *target = m_regGeneral[regId];
            return true;
        }
        m_errorMessage = QString("Read from unknown register: %1").arg(QString::number(regId, 16));
        m_errorIndex = PEANUT_ERR_NOREG;
        return false;
    }
}

inline bool PeanutCpu::writeReg(quint8 regId, PEANUT_REG_TYPE source)
{
    // Check instruction.
    switch(regId)
    {
    case PEANUT_SP_ADDRESS:
        m_regSP = (PEANUT_STATE_TYPE)source;
        return true;
    case PEANUT_SR_ADDRESS:
        m_regSR = (PEANUT_REG_UTYPE)source;
        return true;
    case PEANUT_PC_ADDRESS:
        m_regPC = (PEANUT_ADDRESS_TYPE)source;
        return true;
    case PEANUT_ONE_ADDRESS:
        m_errorMessage = "Write to read only register ONE";
        m_errorIndex = PEANUT_ERR_RREG;
        return false;
    case PEANUT_ZERO_ADDRESS:
        m_errorMessage = "Write to read only register ZERO";
        m_errorIndex = PEANUT_ERR_RREG;
        return false;
    case PEANUT_MONE_ADDRESS:
        m_errorMessage = "Write to read only register MONE";
        m_errorIndex = PEANUT_ERR_RREG;
        return false;
    default:
        if (regId < PEANUT_GENERAL_REGRS)
        {
            m_regGeneral[regId] = (PEANUT_REG_TYPE)source;
            return true;
        }
        m_errorMessage = QString("Write to unknown register: %1").arg(QString::number(regId, 16));
        m_errorIndex = PEANUT_ERR_NOREG;
        return false;
    }
}

bool PeanutCpu::readMemory(PEANUT_ADDRESS_TYPE address, PEANUT_WORD_TYPE *value)
{
    if(m_memory)
    {
        // Found the memory.
        if(m_memory->read(address, value))
        {
            // Complete.
            return true;
        }
        // Invalid address found.
        m_errorMessage=QString("Invalid memory read address: %1").arg(QString::number(address, 16));
        m_errorIndex=PEANUT_ERR_INVMEM;
        return false;
    }
    m_errorMessage="Cannot find memory";
    m_errorIndex=PEANUT_ERR_NOMEM;
    return false;
}

inline bool PeanutCpu::writeMemory(PEANUT_ADDRESS_TYPE address,
                                   PEANUT_WORD_TYPE value)
{
    if(m_memory)
    {
        // Found the memory.
        if(m_memory->write(address, value))
        {
            // Complete.
            return true;
        }
        // Invalid address found.
        m_errorMessage=QString("Invalid memory write address: %1").arg(QString::number(address, 16));
        m_errorIndex=PEANUT_ERR_INVMEM;
        return false;
    }
    m_errorMessage="Cannot find memory";
    m_errorIndex=PEANUT_ERR_NOMEM;
    return false;
}

inline bool PeanutCpu::readInsRs1(PEANUT_INS_TYPE instruction,
                                  PEANUT_REG_TYPE *target)
{
    return readReg(CPU_GET_RS1(instruction), target);
}

inline bool PeanutCpu::readInsRs2(PEANUT_INS_TYPE instruction,
                                  PEANUT_REG_TYPE *target)
{
    return readReg(CPU_GET_RS2(instruction), target);
}

inline bool PeanutCpu::readInsRs3(PEANUT_INS_TYPE instruction,
                                  PEANUT_REG_TYPE *target)
{
    return readReg(CPU_GET_RS3(instruction), target);
}

inline bool PeanutCpu::readInsRs4(PEANUT_INS_TYPE instruction,
                                  PEANUT_REG_TYPE *target)
{
    return readReg(CPU_GET_RS4(instruction), target);
}

inline bool PeanutCpu::setInsRs2(PEANUT_INS_TYPE instruction,
                                 PEANUT_REG_TYPE value)
{
    return writeReg(CPU_GET_RS2(instruction), value);
}

inline bool PeanutCpu::setInsRs3(PEANUT_INS_TYPE instruction,
                                 PEANUT_REG_TYPE value)
{
    return writeReg(CPU_GET_RS3(instruction), value);
}

inline quint8 PeanutCpu::getInsRs(PEANUT_INS_TYPE instruction, quint8 pos)
{
    //Get the instruction RS.
    return (quint8)((instruction & ((PEANUT_INS_TYPE)0x0F << pos)) >> pos);
}

inline PEANUT_ADDRESS_TYPE PeanutCpu::getInsAddr(PEANUT_INS_TYPE instruction, quint8 pos)
{
    // Get the instruction memory address.
    return (PEANUT_ADDRESS_TYPE)(instruction & ((PEANUT_INS_TYPE)0xFFFF << pos)) >> pos;
}

bool PeanutCpu::isHalt() const
{
    return m_halt;
}

void PeanutCpu::dump()
{
    qDebug()<<QString::number((quint32)m_regGeneral[0], 16)
            <<QString::number((quint32)m_regGeneral[1], 16)
            <<QString::number((quint32)m_regGeneral[2], 16)
            <<QString::number((quint32)m_regGeneral[3], 16)
            <<QString::number((quint32)m_regGeneral[4], 16)
            <<QString::number((quint32)m_regGeneral[5], 16)
            <<QString::number((quint32)m_regGeneral[6], 16)
            <<QString::number((quint32)m_regGeneral[7], 16)
            <<QString::number((quint32)m_regSP, 16)
            <<QString::number((quint32)m_regIR, 16)
            <<QString::number((quint32)m_regSR, 16)
            <<QString::number((quint32)m_regPC, 16);
}
