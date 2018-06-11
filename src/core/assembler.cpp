#include "tokenizer.h"
#include "peanutmemory.h"

#include "assembler.h"

#include <QDebug>

#define lexNext() \
    m_tokenizer->next(); \
    if(m_tokenizer->hasError()) \
    { \
        m_errorInfo = m_tokenizer->errorInfo(); \
        m_errorPos = m_tokenizer->errorPos(); \
        return false; \
    }

Assembler::Assembler(QObject *parent) : QObject(parent),
    m_tokenizer(new Tokenizer(this)),
    m_memory(nullptr)
{
    m_regIdMap.insert("R0",   0x00);
    m_regIdMap.insert("R1",   0x01);
    m_regIdMap.insert("R2",   0x02);
    m_regIdMap.insert("R3",   0x03);
    m_regIdMap.insert("R4",   0x04);
    m_regIdMap.insert("R5",   0x05);
    m_regIdMap.insert("R6",   0x06);
    m_regIdMap.insert("R7",   0x07);
    m_regIdMap.insert("SP",   0x08);
    m_regIdMap.insert("SR",   0x09);
    m_regIdMap.insert("PC",   0x0A);
    m_regIdMap.insert("ONE",  0x0B);
    m_regIdMap.insert("ZERO", 0x0C);
    m_regIdMap.insert("MONE", 0x0D);

    m_aluMap.insert("add",     0x10000000);
    m_aluMap.insert("sub",     0x20000000);
    m_aluMap.insert("mult",    0x30000000);
    m_aluMap.insert("div",     0x40000000);
    m_aluMap.insert("mod",     0x50000000);
    m_aluMap.insert("and",     0x60000000);
    m_aluMap.insert("or",      0x70000000);
    m_aluMap.insert("xor",     0x80000000);
    m_aluMap.insert("rotate",  0xE0000000);
    m_alu2Map.insert("neg",    0xA0000000);
    m_alu2Map.insert("not",    0xA1000000);
    m_alu2Map.insert("move",   0xA2000000);

    m_jumpMap.insert("jumpz",  0xA4100000);
    m_jumpMap.insert("jumpn",  0xA4200000);
    m_jumpMap.insert("jumpnz", 0xA4300000);

    m_setMap.insert("reset",   0xA5000000);
    m_setMap.insert("set",     0xA5100000);
}

bool Assembler::hasError()
{
    return !m_errorInfo.isEmpty();
}

QString Assembler::error() const
{
    return m_errorInfo;
}

QPoint Assembler::errorPos() const
{
    return m_errorPos;
}

void Assembler::setSourceCode(const QString &sourceCode)
{
    // Set the code to tokenizer.
    m_tokenizer->setSourceCode(sourceCode);
    m_errorInfo = QString();
    m_errorPos = QPoint();
    m_postWorkQueue=QList<PostTask>();
}

void Assembler::setMemory(PeanutMemory *memory)
{
    m_memory = memory;
}

bool Assembler::assemble()
{
    PEANUT_ADDRESS_TYPE address = 0x0100;
    PEANUT_INS_TYPE insCache;
    // Load the next token from tokenizer.
    while(m_tokenizer->hasToken())
    {
        // Load the current token.
        QVariant token = m_tokenizer->current();
        if(token.type() == QVariant::UInt)
        {
            //Change the memory address counter, should followed by a colon.
            address = token.toUInt();
            if(!parseChar(':',
                          "An address redirection should followed by ':'."))
            {
                return false;
            }
            // Move to next token.
            lexNext();
            continue;
        }
        // Should be instructions.
        if(token.type() != QVariant::String)
        {
            // Error.
            return false;
        }
        // Check the instruction.
        QString instruction = token.toString();
        if(instruction=="halt")
        {
            insCache = 0x00000000;
        }
        else if(m_aluMap.contains(instruction))
        {
            // Followed by three reg id.
            quint8 rs1, rs2, rd;
            if(!parseRegId(rs1,
                           QString("%1 should followed by a register name").arg(
                               instruction)))
            {
                return false;
            }
            if(!parseRegId(rs2,
                           QString("%1 should followed by a register name").arg(
                               instruction)))
            {
                return false;
            }
            if(!parseRegId(rd,
                           QString("%1 should followed by a register name").arg(
                               instruction)))
            {
                return false;
            }
            insCache = m_aluMap.value(instruction) |
                        ((PEANUT_INS_TYPE)rs1 << 24) |
                        ((PEANUT_INS_TYPE)rs2 << 20) |
                        ((PEANUT_INS_TYPE)rd << 16);
        }
        else if(m_alu2Map.contains(instruction))
        {
            // Followed by two reg id.
            quint8 rs, rd;
            if(!parseRegId(rs,
                           QString("%1 should followed by a register name").arg(
                               instruction)))
            {
                return false;
            }
            if(!parseRegId(rd,
                           QString("%1 should followed by a register name").arg(
                               instruction)))
            {
                return false;
            }
            insCache = m_alu2Map.value(instruction) |
                        ((PEANUT_INS_TYPE)rs << 20) |
                        ((PEANUT_INS_TYPE)rd << 16);
        }
        else if(instruction=="call")
        {
            // Should followed by address.
            PEANUT_ADDRESS_TYPE callAddr = 0x0000;
            if(!parseAddress(callAddr, address, "call immediate should followed"
                             " by an address or an address label"))
            {
                return false;
            }
            insCache = 0xA3000000 | callAddr;
        }
        else if(instruction=="return")
        {
            insCache = 0xA3010000;
        }
        else if(instruction=="trap")
        {
            insCache = 0xA3020000;
        }
        else if(instruction=="jump")
        {
            // Should followed by address.
            PEANUT_ADDRESS_TYPE jumpAddr = 0x0000;
            if(!parseAddress(jumpAddr, address, "jump should followed by an "
                             "address or an address label"))
            {
                return false;
            }
            insCache = 0xA4000000 | jumpAddr;
        }
        else if(m_jumpMap.contains(instruction))
        {
            // Should followed by the reg id.
            quint8 regId;
            if(!parseRegId(regId, QString("%1 should followed by a register "
                                          "name").arg(instruction)))
            {
                return false;
            }
            // Parse the address.
            PEANUT_ADDRESS_TYPE toAddr;
            if(!parseAddress(toAddr, address,
                             QString("%1 should followed by an address or an "
                                     "address label").arg(instruction)))
            {
                return false;
            }
            insCache = m_jumpMap.value(instruction) |
                        ((PEANUT_INS_TYPE)regId << 16) | toAddr;
        }
        else if(m_setMap.contains(instruction))
        {
            // Should followed by a number.
            lexNext();
            token = m_tokenizer->current();
            if(token.type() != QVariant::UInt)
            {
                setError(QString("%1 should followed by the bit number"
                                 ).arg(instruction));
                return false;
            }
            insCache = m_setMap.value(instruction) |
                        ((token.toUInt() & 0x0F)<<16);
        }
        else if(instruction=="push")
        {
            // 0xA60 push onto stack
            quint8 regId;
            if(!parseRegId(regId, "push should followed by a register name"))
            {
                return false;
            }
            insCache = 0xA6000000 | ((PEANUT_INS_TYPE)regId << 16);
        }
        else if(instruction=="pop")
        {
            // 0xA61 pop from stack
            quint8 regId;
            if(!parseRegId(regId, "pop should followed by a register name"))
            {
                return false;
            }
            insCache = 0xA6100000 | ((PEANUT_INS_TYPE)regId << 16);
        }
        else if(instruction=="load")
        {
            // Check the next type.
            lexNext();
            token = m_tokenizer->current();
            if(token.type() == QVariant::Char)
            {
                // 0xC00 immediate load from memory
                // Should be '#'
                if(token.toChar() != '#')
                {
                    setError(QString("load instruction should followed by '#' "
                                     "but not '%1' for immediate load").arg(
                                 token.toChar()));
                    return false;
                }
                // Then followed by an immedate value.
                quint16 immedateValue;
                if(!parseImmedate(immedateValue,
                                  address,
                                  "a char or a label should followed by the '#'"
                                  " in immediate load"))
                {
                    return false;
                }
                // Then followed by the register.
                quint8 regId;
                if(!parseRegId(regId, "immediate load should followed by a "
                                      "register name"))
                {
                    return false;
                }
                insCache = 0xC0000000 | ((PEANUT_INS_TYPE)regId << 16) |
                            immedateValue;
            }
            else if(token.type() == QVariant::UInt)
            {
                // 0xC10 absolute load from memory
                PEANUT_ADDRESS_TYPE address = token.toUInt();
                // Then followed by register.
                quint8 regId;
                if(!parseRegId(regId, "absolute load should followed by a "
                                      "register name"))
                {
                    return false;
                }
                insCache = 0xC1000000 | ((PEANUT_INS_TYPE)regId << 16) |
                           address;
            }
            else
            {
                quint8 regId;
                // Should be reg id.
                if(!toRegId(token.toString(), regId))
                {
                    return false;
                }
                // Check the next token type.
                lexNext();
                token = m_tokenizer->current();
                if(token.type() == QVariant::String)
                {
                    // 0xC2 indirect load from memory
                    // Should follow the RD reg id.
                    quint8 rdRegId;
                    if(!toRegId(token.toString(), rdRegId))
                    {
                        return false;
                    }
                    // Compose the instruction.
                    insCache = 0xC2000000 | ((PEANUT_INS_TYPE)regId << 20) |
                            ((PEANUT_INS_TYPE)rdRegId << 16);
                }
                else if(token.type() == QVariant::Char)
                {
                    // 0xC3 base + displacement load from memory
                    // Should be '#'
                    if(token.toChar() != '#')
                    {
                        setError(QString("base + displacement load from memory "
                                         "instruction should followed by '#' "
                                         "but not '%1'").arg(
                                     token.toChar()));
                        return false;
                    }
                    // Then followed by an immedate value.
                    quint16 immedateValue;
                    if(!parseImmedate(immedateValue,
                                      address,
                                      "a char or a label should followed by "
                                      "the '#' in base + displacement load "
                                      "from memory"))
                    {
                        return false;
                    }
                    // Should follow the RD reg id.
                    quint8 rdRegId;
                    if(!parseRegId(rdRegId,
                                   "base + displacement load should followed by"
                                   " a register name"))
                    {
                        return false;
                    }
                    // Compose the instruction.
                    insCache = 0xC3000000 | ((PEANUT_INS_TYPE)regId << 20) |
                            ((PEANUT_INS_TYPE)rdRegId << 16) | immedateValue;
                }
                else
                {
                    setError("unknown load instruction");
                    return false;
                }
            }
        }
        else if(instruction=="store")
        {
            // Must followed by one reg Id.
            quint8 regId;
            if(!parseRegId(regId, "store should followed by a register name"))
            {
                return false;
            }
            // Check next type.
            lexNext();
            token = m_tokenizer->current();
            if(token.type() == QVariant::UInt)
            {
                // 0xD1 absolute store to memory.
                insCache = 0xD1000000 | ((PEANUT_INS_TYPE)regId << 20) |
                        (PEANUT_ADDRESS_TYPE)token.toUInt();
            }
            else if(token.type() == QVariant::String)
            {
                // 0xD2 indirect store to memory
                quint8 rda;
                if(!parseRegId(rda, "indirect store should followed by a "
                                      "register name"))
                {
                    return false;
                }
                insCache = 0xD2000000 | ((PEANUT_INS_TYPE)regId << 20) |
                        ((PEANUT_INS_TYPE)rda << 16);
            }
            else if(token.type() == QVariant::Char)
            {
                // 0xD3 base + displacement store to memory
                // Should be '#'
                if(token.toChar() != '#')
                {
                    setError(QString("base + displacement store to memory "
                                     "instruction should followed by '#' "
                                     "but not '%1'").arg(
                                 token.toChar()));
                    return false;
                }
                // Then followed by an immedate value.
                quint16 immedateValue;
                if(!parseImmedate(immedateValue,
                                  address,
                                  "a char or a label should followed by the '#'"
                                  " in base + displacement store to memory"))
                {
                    return false;
                }
                // Should follow the RD reg id.
                quint8 rdRegId;
                if(!parseRegId(rdRegId,
                               "base + displacement load should followed by"
                               " a register name"))
                {
                    return false;
                }
                // Compose the instruction.
                insCache = 0xD3000000 | ((PEANUT_INS_TYPE)regId << 20) |
                        ((PEANUT_INS_TYPE)rdRegId << 16) | immedateValue;
            }
        }
        else if(instruction=="block")
        {
            // Must followed by '#'
            if(!parseChar('#', "block should followed by '#' and a string for "
                          "the block data"))
            {
                return false;
            }
            // Fetch the next data.
            lexNext();
            token = m_tokenizer->current();
            if(token.type()!=QVariant::String)
            {
                setError("in block, '#' should followed by a string for its "
                         "data");
                return false;
            }
            QString blockData = token.toString();
            if(blockData.size() < 2 || blockData.at(0) != '\"' ||
                    blockData.at(blockData.size()-1) != '\"')
            {
                setError("in block, '#' should followed by a string within a "
                         "quote '\"'");
                return false;
            }
            blockData = blockData.mid(1, blockData.size()-2);
            // Translate the escape.
            QHash<QString, QChar> escapeMap = m_tokenizer->escapeMap();
            QHash<QString, QChar>::const_iterator mapItem =
                    escapeMap.constBegin();
            while (mapItem != escapeMap.constEnd())
            {
                blockData.replace(mapItem.key(), mapItem.value());
                ++mapItem;
            }
            for(auto blockChar : blockData)
            {
                // Write the data to memory.
                m_memory->write(address++,
                                (PEANUT_INS_TYPE)blockChar.toLatin1());
            }
            // Add one more 0x00 at the end.
            insCache = 0x00;
        }
        else // Custom label.
        {
            QString labelName = instruction;
            // Should followed by a colon.
            if(!parseChar(':',
                          QString("Unknown instruction '%1'").arg(labelName)))
            {
                return false;
            }
            // Recording the label index.
            m_labelMap.insert(labelName, address);
            // Move to next token.
            lexNext();
            continue;
        }
        // Write the instruction to memory.
        m_memory->write(address++, insCache);
        // Move to next token.
        lexNext();
    }
    // Run the post working.
    while(!m_postWorkQueue.isEmpty())
    {
        //Pick out the task.
        PostTask task = m_postWorkQueue.takeFirst();
        // Check the task label name.
        if(!m_labelMap.contains(task.labelName))
        {
            // Invalid label detected.
            setError(QString("Label '%1' cannot found").arg(task.labelName),
                     task.pos);
            return false;
        }
        // Replace the label info.
        m_memory->read(task.address, &insCache);
        insCache &= 0xFFFF0000;
        insCache |= m_labelMap.value(task.labelName);
        m_memory->write(task.address, insCache);
    }
    return true;
}

void Assembler::setError(const QString &errorInfo, const QPoint &errorPos)
{
    m_errorInfo = errorInfo;
    m_errorPos = errorPos.isNull() ? m_tokenizer->pos() : errorPos;
}

bool Assembler::parseImmedate(quint16 &immedateValue, quint16 address,
                              const QString &errorInfo)
{
    // Get the next token.
    lexNext();
    QVariant token = m_tokenizer->current();
    // Check token type
    if(token.type() == QVariant::Char)
    {
        // Save the value
        immedateValue = token.toChar().toLatin1();
    }
    else if(token.type() == QVariant::String)
    {
        // It would be the label.
        if(m_labelMap.contains(token.toString()))
        {
            immedateValue = m_labelMap.value(token.toString());
        }
        else
        {
            // throw this in post-updating queue.
            appendPostWork(address, token.toString());
        }
    }
    else if(token.type() == QVariant::UInt)
    {
        // Save the value.
        immedateValue = token.toUInt();
    }
    else
    {
        setError(errorInfo);
        return false;
    }
    return true;
}

bool Assembler::parseAddress(quint16 &addressValue,
                             quint16 address,
                             const QString &errorInfo)
{
    lexNext();
    QVariant token = m_tokenizer->current();
    if(token.type() == QVariant::UInt)
    {
        // Address.
        addressValue = token.toUInt();
    }
    else if(token.type() == QVariant::String)
    {
        // It would be the label.
        if(m_labelMap.contains(token.toString()))
        {
            addressValue = m_labelMap.value(token.toString());
        }
        else
        {
            // throw this in post-updating queue.
            appendPostWork(address, token.toString());
        }
    }
    else
    {
        setError(errorInfo);
        return false;
    }
    return true;
}

bool Assembler::parseRegId(quint8 &regId, const QString &errorInfo)
{
    // Get the next token.
    lexNext();
    QVariant token = m_tokenizer->current();
    // Check token type.
    if(token.type() != QVariant::String)
    {
        setError(errorInfo);
        return false;
    }
    // Get the register id.
    return toRegId(token.toString(), regId);
}

bool Assembler::parseChar(char targetChar, const QString &errorInfo)
{
    // Get the next token.
    lexNext();
    QVariant token = m_tokenizer->current();
    // Check token type and content.
    if(token.type() != QVariant::Char ||
            token.toChar().toLatin1() != targetChar)
    {
        setError(errorInfo);
        return false;
    }
    return true;
}

bool Assembler::parseChar(char *value, const QString &errorInfo)
{
    // Get the next token.
    lexNext();
    QVariant token = m_tokenizer->current();
    // Check token type
    if(token.type() != QVariant::Char)
    {
        setError(errorInfo);
        return false;
    }
    // Save the value
    *value = token.toChar().toLatin1();
    return true;
}

bool Assembler::toRegId(const QString &name, quint8 &regId)
{
    if(m_regIdMap.contains(name))
    {
        regId = m_regIdMap.value(name);
        return true;
    }
    setError(QString("invalid register name '%1'").arg(name));
    return false;
}

inline void Assembler::appendPostWork(const quint32 &address,
                                      const QString &labelName)
{
    PostTask item;
    item.address = address;
    item.pos = m_tokenizer->pos();
    item.labelName = labelName;
    m_postWorkQueue.append(item);
}
