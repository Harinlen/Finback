#include <QTabWidget>
#include <QTabBar>
#include <QMenu>
#include <QMenuBar>

#include "peanut.h"
#include "assembler.h"
#include "tokenizer.h"

#include "textedittab.h"
#include "terminalsimulator.h"

#include "mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    m_peanut(new Peanut(this)),
    m_assembler(new Assembler(this)),
    m_terminal(new TerminalSimulator(this)),
    m_documentHolder(new QTabWidget(this))
{
    // Set properties.
    setCentralWidget(m_documentHolder);
    addDockWidget(Qt::RightDockWidgetArea, m_terminal, Qt::Vertical);

    // Menu.
    QMenu *menuFile = new QMenu("&File", this);
    menuBar()->addMenu(menuFile);
    QAction *actNew = new QAction("&New", this);
    actNew->setShortcut(QKeySequence::New);
    connect(actNew, &QAction::triggered, this, &MainWindow::onActionNew);
    menuFile->addAction(actNew);

    QMenu *menuRun = new QMenu("&Run", this);
    menuBar()->addMenu(menuRun);
    QAction *actRun = new QAction("&Run", this);
    actRun->setShortcut(QKeySequence::Refresh);
    connect(actRun, &QAction::triggered, this, &MainWindow::onActionRun);
    menuRun->addAction(actRun);

    // Set data io to terminal.
    m_peanut->setDataIo(m_terminal->terminalIo());
    // Set the memory to assmebler.
    m_assembler->setMemory(m_peanut->memory());
}

void MainWindow::onActionNew()
{
    createNewTab();
}

void MainWindow::onActionRun()
{
    // Get the current widget.
    if(m_documentHolder->currentWidget()==nullptr)
    {
        return;
    }
    // Get the current tab.
    TextEditTab *tab =
            static_cast<TextEditTab *>(m_documentHolder->currentWidget());
    // Reset the CPU.
    m_peanut->reset();
    // Set the source code.
    m_assembler->setSourceCode(tab->sourceCode());
    // Assmeble.
    if(m_assembler->assemble())
    {
        qDebug()<<"Execute!!";
        // Execute the code.
        m_peanut->execute();
    }
    else
    {
        qDebug()<<m_assembler->error();
        qDebug()<<m_assembler->errorPos();
    }
}

TextEditTab *MainWindow::createNewTab()
{
    TextEditTab *tab = new TextEditTab(this);
    m_documentHolder->addTab(tab, "Untitled");
    m_documentHolder->setCurrentWidget(tab);
    tab->focusOn();
    return tab;
}
