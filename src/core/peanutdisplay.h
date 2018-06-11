#ifndef PEANUTDISPLAY_H
#define PEANUTDISPLAY_H

#include <QObject>

class PeanutDisplay : public QObject
{
    Q_OBJECT
public:
    PeanutDisplay(QObject *parent = nullptr)  : QObject(parent){}

signals:

public slots:
    virtual void updatePixels(int position, quint32 pixels) = 0;
};

#endif // PEANUTDISPLAY_H
