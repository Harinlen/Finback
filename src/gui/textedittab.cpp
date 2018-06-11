#include <QBoxLayout>

#include "textedit.h"
#include "texthighlighter.h"

#include "textedittab.h"

TextEditTab::TextEditTab(QWidget *parent) : QWidget(parent),
    m_editor(new TextEdit(this))
{
    TextHighlighter *highlighter = new TextHighlighter(this);
    highlighter->setDocument(m_editor->document());

    QBoxLayout *mainLayout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    mainLayout->addWidget(m_editor);
}

QString TextEditTab::sourceCode()
{
    return m_editor->document()->toPlainText();
}

void TextEditTab::focusOn()
{
    // Set the focus.
    m_editor->setFocus();
}
