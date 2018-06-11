#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>

class TextEditPanel;
class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = nullptr);

signals:

public slots:
    int panelWidth();
    void paintPanel(QPaintEvent *event);

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    bool event(QEvent *e) override;

private slots:
    void highlighCurrentLine();
    void updatePanelWidth();

protected:
    void insertFromMimeData(const QMimeData *source) override;

private:
    TextEditPanel *m_panel;
};

class TextEditPanel : public QWidget
{
    Q_OBJECT
public:
    TextEditPanel(TextEdit *editor) : QWidget(editor),
        m_codeEditor(editor)
    {
    }

    QSize sizeHint() const override
    {
        return QSize(m_codeEditor->panelWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_codeEditor->paintPanel(event);
    }

private:
    TextEdit *m_codeEditor;
};

#endif // TEXTEDIT_H
