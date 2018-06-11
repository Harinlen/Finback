#ifndef PEANUTDATAIO_H
#define PEANUTDATAIO_H

#include <QObject>
#include <QWidget>

class PeanutDataIo : public QObject
{
    Q_OBJECT
public:
    PeanutDataIo(QObject *parent = nullptr) : QObject(parent){}
    virtual QChar read() = 0;
    virtual bool isCacheEmpty() = 0;

signals:

public slots:
    virtual void resetCache() = 0;
    virtual void write(QChar data) = 0;
};

#endif // PEANUTDATAIO_H
