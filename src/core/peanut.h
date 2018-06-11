#ifndef PEANUT_H
#define PEANUT_H

#include <QObject>

class PeanutCpu;
class PeanutMemory;
class PeanutDataIo;
class Peanut : public QObject
{
    Q_OBJECT
public:
    explicit Peanut(QObject *parent = nullptr);
    PeanutMemory *memory() const;

signals:
    void requireNext();
    void halt();

public slots:
    void setDataIo(PeanutDataIo *dataIo);
    void reset();
    void execute();
    void step();

private:
    bool m_stop;
    PeanutCpu *m_cpu;
    PeanutMemory *m_memory;
};

#endif // PEANUT_H
