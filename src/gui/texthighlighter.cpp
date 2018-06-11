
#include "texthighlighter.h"

TextHighlighter::TextHighlighter(QObject *parent) : QSyntaxHighlighter(parent)
{
    QTextCharFormat textFormat;
    HighlightingRule rule;

    // Add keyword highlighter
    QStringList keywordPatterns;
    keywordPatterns << "add" << "sub" << "mult" << "div" << "mod" << "and"
                    << "or" << "xor" << "rotate" << "neg" << "not" << "move"
                    << "call" << "return" << "trap" << "jump" << "jumpz"
                    << "jumpn" << "jumpnz" << "reset" << "set" << "push"
                    << "pop" << "load" << "store" << "halt" << "block";
    textFormat.setForeground(QColor(0x80, 0x80, 0x00));
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression("\\b"+pattern+"\\b");
        rule.format = textFormat;
        m_rules.append(rule);
    }

    QStringList regPatterns;
    regPatterns << "R0" << "R1" << "R2" << "R3" << "R4" << "R5" << "R6" << "R7"
                << "SP" << "SR" << "PC" << "ONE" << "ZERO" << "MONE";
    textFormat.setForeground(QColor(0x80, 0x00, 0x80));
    foreach (const QString &pattern, regPatterns) {
        rule.pattern = QRegularExpression("\\b"+pattern+"\\b");
        rule.format = textFormat;
        m_rules.append(rule);
    }

    textFormat.setForeground(QColor(0x00, 0x00, 0x80));
    rule.pattern = QRegularExpression("0x(?:[A-Fa-f0-9]*)");
    rule.format = textFormat;
    m_rules.append(rule);
    rule.pattern = QRegularExpression("#([-+])?[0-9]*");
    rule.format = textFormat;
    m_rules.append(rule);

    // String.
    textFormat.setForeground(QColor(0x00, 0x80, 0x00));
    rule.pattern = QRegularExpression("#\\\".*?\\\"");
    rule.format = textFormat;
    m_rules.append(rule);

    // Comments must be the last one.
    textFormat.setForeground(QColor(0x00, 0x80, 0x00));
    rule.pattern = QRegularExpression(";[^\n]*");
    rule.format = textFormat;
    m_rules.append(rule);
}

void TextHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, m_rules)
    {
        QRegularExpressionMatchIterator matchIterator =
                rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(),
                      match.capturedLength(),
                      rule.format);
        }
    }
}
