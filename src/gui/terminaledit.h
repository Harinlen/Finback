#ifndef TERMINALEDIT_H
#define TERMINALEDIT_H

#include <QPlainTextEdit>

class TerminalEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit TerminalEdit(QWidget *parent = nullptr);

signals:
    void dataIn(char data);

public slots:

protected:
    bool event(QEvent *e) override;
};

#endif // TERMINALEDIT_H
