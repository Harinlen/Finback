#ifndef TERMINALSIMULATOR_H
#define TERMINALSIMULATOR_H

#include <QDockWidget>

class TerminalEdit;
class TerminalDataIo;
class PeanutDataIo;
class TerminalSimulator : public QDockWidget
{
    Q_OBJECT
public:
    explicit TerminalSimulator(QWidget *parent = nullptr);

    PeanutDataIo *terminalIo() const;

signals:

public slots:

private:
    TerminalEdit *m_textEdit;
    TerminalDataIo *m_dataIo;
};

#endif // TERMINALSIMULATOR_H
