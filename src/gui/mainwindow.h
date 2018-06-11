#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Peanut;
class TextEditTab;
class Assembler;
class TerminalSimulator;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:

public slots:

private slots:
    void onActionNew();
    void onActionRun();

private:
    TextEditTab *createNewTab();
    Peanut *m_peanut;
    Assembler *m_assembler;
    TerminalSimulator *m_terminal;

    QTabWidget *m_documentHolder;
};

#endif // MAINWINDOW_H
