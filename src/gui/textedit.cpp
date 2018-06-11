#include <QPaintEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QMimeData>

#include "textedit.h"

#include <QDebug>

TextEdit::TextEdit(QWidget *parent) : QTextEdit(parent),
    m_panel(new TextEditPanel(this))
{
    // Configure the layout.
    setAutoFillBackground(true);
    setWordWrapMode(QTextOption::NoWrap);
    // Set default font.
    QFont font = this->font();
    font.setFamily("Monaco");
    font.setPixelSize(12);
    setFont(font);
    setTabStopWidth(fontMetrics().width(' ') * 4);
    // Set triggers.
    connect(document(), &QTextDocument::blockCountChanged,
            this, &TextEdit::updatePanelWidth);
    connect(this, &TextEdit::cursorPositionChanged,
            this, &TextEdit::highlighCurrentLine);

    // Update the current line.
    highlighCurrentLine();
}

int TextEdit::panelWidth()
{
    return fontMetrics().width('9') *
            QString::number(document()->blockCount()).size() + 13;
}

void TextEdit::paintPanel(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(m_panel);
    painter.fillRect(m_panel->rect(), QColor(0xEF, 0xEB, 0xE7));
    // Get the first visible block.
    QTextBlock block = cursorForPosition(QPoint(0, 0)).block();
    QAbstractTextDocumentLayout *layout = document()->documentLayout();
    int panelWidth = m_panel->width() - 5, startPosition,
            currentLine = textCursor().block().blockNumber();
    QRectF blockRect = layout->blockBoundingRect(block);
    // Configure the painter.
    painter.setPen(QColor(0xAf, 0xAd, 0xAa));
    startPosition = blockRect.y()-verticalScrollBar()->value();
    while(block.isValid() && startPosition < height())
    {
        if(block.blockNumber()==currentLine)
        {
            painter.setPen(QColor(0x30, 0x30, 0x30));
        }
        painter.drawText(0, startPosition, panelWidth, blockRect.height(),
                         Qt::AlignVCenter | Qt::AlignRight,
                         QString::number(block.blockNumber()+1));
        if(block.blockNumber()==currentLine)
        {
            painter.setPen(QColor(0xAf, 0xAd, 0xAa));
        }
        // Move the position.
        startPosition += blockRect.height();
        block = block.next();
        blockRect = layout->blockBoundingRect(block);
    }
}

void TextEdit::paintEvent(QPaintEvent *e)
{
    // Update the panel.
    m_panel->update();
    // Draw the content.
    QTextEdit::paintEvent(e);
}

void TextEdit::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);
    // Update the panel widget.
    updatePanelWidth();
}

bool TextEdit::event(QEvent *e)
{
    // Override shift+enter.
    if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        // Check modifiers.
        if(keyEvent->modifiers() & Qt::ShiftModifier &&
                (keyEvent->key() == Qt::Key_Enter ||
                 keyEvent->key() == Qt::Key_Return))
        {
            keyEvent->setModifiers(keyEvent->modifiers() & ~Qt::ShiftModifier);
        }
        return QTextEdit::event(keyEvent);
    }
    return QTextEdit::event(e);
}

void TextEdit::highlighCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(0xe0, 0xee, 0xf6);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void TextEdit::updatePanelWidth()
{
    setViewportMargins(panelWidth(), 0, 0, 0);
    m_panel->resize(panelWidth(), height());
}

void TextEdit::insertFromMimeData(const QMimeData *source)
{
    QTextEdit::insertPlainText(source->text());
}
