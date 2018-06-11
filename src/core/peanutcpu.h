#ifndef PEANUTCPU_H
#define PEANUTCPU_H

#include "peanutglobal.h"

#include <QObject>

class PeanutMemory;
class PeanutCpu : public QObject
{
    Q_OBJECT
public:
    explicit PeanutCpu(QObject *parent = nullptr);

    bool isHalt() const;

    void dump();

signals:

public slots:
    void reset();
    void loadInstruction();
    bool executeInstruction();
    bool executeNext();
    bool execute(PEANUT_INS_TYPE instruction);

    void setMemory(PeanutMemory *memory);

private:
    typedef enum AluOperations
    {
        AluAdd,
        AluSub,
        AluMult,
        AluDiv,
        AluMod,
        AluAnd,
        AluOr,
        AluXor,
        AluRotate,
        AluNeg,
        AluNot
    } AluOperations;

    inline bool executeAlu(AluOperations op, PEANUT_INS_TYPE instruction);
    inline bool alu (AluOperations op,
                     PEANUT_REG_TYPE rs1,
                     PEANUT_REG_TYPE rs2,
                     PEANUT_REG_TYPE *res);

    inline bool readReg(quint8 regId, PEANUT_REG_TYPE *target);
    inline bool writeReg(quint8 regId, PEANUT_REG_TYPE source);
    inline bool readMemory(PEANUT_ADDRESS_TYPE address, PEANUT_WORD_TYPE *value);
    inline bool writeMemory(PEANUT_ADDRESS_TYPE address, PEANUT_WORD_TYPE value);
    inline bool readInsRs1(PEANUT_INS_TYPE instruction, PEANUT_REG_TYPE *target);
    inline bool readInsRs2(PEANUT_INS_TYPE instruction, PEANUT_REG_TYPE *target);
    inline bool readInsRs3(PEANUT_INS_TYPE instruction, PEANUT_REG_TYPE *target);
    inline bool readInsRs4(PEANUT_INS_TYPE instruction, PEANUT_REG_TYPE *target);
    inline bool setInsRs2(PEANUT_INS_TYPE instruction, PEANUT_REG_TYPE value);
    inline bool setInsRs3(PEANUT_INS_TYPE instruction, PEANUT_REG_TYPE value);
    inline quint8 getInsRs(PEANUT_INS_TYPE instruction, quint8 pos);
    inline PEANUT_ADDRESS_TYPE getInsAddr(PEANUT_INS_TYPE instruction, quint8 pos);

    PEANUT_REG_TYPE m_regGeneral[PEANUT_GENERAL_REGRS];
    PEANUT_REG_UTYPE m_regIR;
    PEANUT_REG_UTYPE m_regSR;

    PEANUT_STATE_TYPE m_regSP;
    PEANUT_ADDRESS_TYPE m_regPC;
    QList<AluOperations> m_unaryOps;

    PeanutMemory *m_memory;
    bool m_halt;

    PEANUT_ERR_TYPE m_errorIndex;
    QString m_errorMessage;
};

#endif // PEANUTCPU_H
