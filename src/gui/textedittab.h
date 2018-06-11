#ifndef TEXTEDITTAB_H
#define TEXTEDITTAB_H

#include <QWidget>

class TextEdit;
class TextEditTab : public QWidget
{
    Q_OBJECT
public:
    explicit TextEditTab(QWidget *parent = nullptr);

    QString sourceCode();

signals:

public slots:
    void focusOn();

private:
    TextEdit *m_editor;
};

#endif // TEXTEDITTAB_H
