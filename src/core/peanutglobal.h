#ifndef PEANUTGLOBAL_H
#define PEANUTGLOBAL_H

#define PEANUT_GENERAL_REGRS    8
#define PEANUT_REG_TYPE         qint32
#define PEANUT_REG_UTYPE        quint32
#define PEANUT_REG_EMPTY        0x00000000
#define PEANUT_STATE_TYPE       quint16
#define PEANUT_STATE_DEFAULT    0x7000
#define PEANUT_PC_DEFAULT       0x0100

#define PEANUT_SP_ADDRESS       0x08
#define PEANUT_SR_ADDRESS       0x09
#define PEANUT_PC_ADDRESS       0x0A
#define PEANUT_ONE_ADDRESS      0x0B
#define PEANUT_ZERO_ADDRESS     0x0C
#define PEANUT_MONE_ADDRESS     0x0D

#define PEANUT_SR_INTOVERFLOW   0x00
#define PEANUT_SR_INTERRUPT     0x01
#define PEANUT_SR_ENINTTIMER    0x02

#define PEANUT_INS_TYPE         quint32

#define PEANUT_ADDRESS_BIT      16
#define PEANUT_ADDRESS_TYPE     quint16
#define PEANUT_ADDRESS_PTYPE    quint32
#define PEANUT_ADDRESS_NULL     0x0000
#define PEANUT_WORD_TYPE        quint32
#define PEANUT_WORD_NULL        0x00000000
#define PEANUT_MEM_SIZE         (1<<PEANUT_ADDRESS_BIT)
#define PEANUT_ADDRESS_MAX      (PEANUT_MEM_SIZE - 1)

#define PEANUT_ERR_TYPE         quint8
#define PEANUT_ERR_NULL         0x00
#define PEANUT_ERR_INVMEM       0x01
#define PEANUT_ERR_NOMEM        0x02
#define PEANUT_ERR_NOINS        0x03
#define PEANUT_ERR_NOREG        0x04
#define PEANUT_ERR_DIVBYZERO    0x05
#define PEANUT_ERR_RREG         0x06

#endif // PEANUTGLOBAL_H
