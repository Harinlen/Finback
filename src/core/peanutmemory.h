#ifndef PEANUTMEMORY_H
#define PEANUTMEMORY_H

#include <QHash>

#include "peanutglobal.h"

#include <QObject>

class PeanutDisplay;
class PeanutDataIo;
class PeanutMemory : public QObject
{
    Q_OBJECT
public:
    explicit PeanutMemory(QObject *parent = nullptr);
    bool isInputEmpty();

signals:

public slots:
    void setDisplay(PeanutDisplay *display);
    void setDataIo(PeanutDataIo *dataIo);
    bool read(PEANUT_ADDRESS_TYPE address, PEANUT_WORD_TYPE *target);
    bool write(PEANUT_ADDRESS_TYPE address, PEANUT_WORD_TYPE value);
    void reset();

private:
    QHash<PEANUT_ADDRESS_TYPE, PEANUT_WORD_TYPE> m_memoryBank;
    PeanutDisplay *m_display;
    PeanutDataIo *m_dataIo;
};

#endif // PEANUTMEMORY_H
