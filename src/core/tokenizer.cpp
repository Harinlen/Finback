#include "tokenizer.h"

#include <QDebug>

#define checkEmpty() \
    if(currentLine.isEmpty()) \
    { \
        continue; \
    } \
    firstChar = currentLine.at(0);

#define skipEmpty() \
    trimmedCache = currentLine.trimmed(); \
    m_column += currentLine.size() - trimmedCache.size(); \
    currentLine = trimmedCache; \

#define skipAndCheckEmpty() \
    skipEmpty(); \
    checkEmpty();

QString Tokenizer::hexChars = "0123456789ABCDEFabcdef";
QString Tokenizer::decChars = "0123456789";
QString Tokenizer::binChars = "01";
QString Tokenizer::specChars = ":#&";

Tokenizer::Tokenizer(QObject *parent) : QObject(parent),
    m_tokenList(QList<Token>()),
    m_row(0),
    m_column(0)
{
    m_escapeMap.insert("\\n", '\n');
    m_escapeMap.insert("\\t", '\t');
    m_escapeMap.insert("\\0", '\0');
    m_escapeMap.insert("\\r", '\r');
    m_escapeMap.insert("\\v", '\v');
}

void Tokenizer::setSourceCode(const QString &sourceCode)
{
    //Clear the original source code.
    reset();
    // Split the source code line by line.
    m_sourceLines = sourceCode.split('\n');
    // Update the current.
    next();
}

void Tokenizer::reset()
{
    m_sourceLines = QStringList();
    // Reset the row and column.
    m_errorInfo = QString();
    m_row = 0; m_column = 0;
}

bool Tokenizer::hasToken() const
{
    return !m_current.isNull();
}

bool Tokenizer::parse(const QVariant &expected)
{
    bool result = (m_current == expected);
    // Move to next.
    next();
    return result;
}

QVariant Tokenizer::current() const
{
    return m_current;
}

void Tokenizer::next()
{
    // Check the token list.
    if(!m_tokenList.isEmpty())
    {
        Token token = m_tokenList.takeFirst();
        m_current = token.token;
        m_pos = token.pos;
        return;
    }
    // Parse one line.
    QChar firstChar;
    while(m_tokenList.isEmpty() && !m_sourceLines.isEmpty())
    {
        // Get the first line.
        QString currentLine = m_sourceLines.takeFirst(),
                trimmedCache;
        ++m_row; m_column = 0;
        // Skip the space.
        while(!currentLine.isEmpty())
        {
            skipAndCheckEmpty();
            // Check comments.
            if(firstChar == ';')
            {
                // Skip to the next line.
                break;
            }
            // Check numbers.
            if(firstChar.isDigit() || firstChar == '-')
            {
                if(firstChar=='0')
                {
                    // For other base number.
                    if(currentLine.size()>1)
                    {
                        if(currentLine.at(1) == 'x')
                        {
                            //0x matches, hex number found.
                            currentLine = currentLine.mid(
                                        parseNumber(hexChars, currentLine, 16));
                            skipAndCheckEmpty();
                        }
                        else if(currentLine.at(1) == 'b')
                        {
                            //0b matches, bin number found.
                            currentLine = currentLine.mid(
                                        parseNumber(binChars, currentLine, 2));
                            skipAndCheckEmpty();
                        }
                    }
                }
                else
                {
                    // Normal 10-base number found.
                    currentLine = currentLine.mid(
                                parseNumber(decChars, currentLine, 10));
                    skipAndCheckEmpty();
                }
                continue;
            }
            // Check the first char.
            if(specChars.contains(firstChar))
            {
                // Special chars.
                appendToken(firstChar);
                ++m_column;
                // Remove the previous chars.
                currentLine = currentLine.remove(0, 1);
                skipAndCheckEmpty();
                continue;
            }
            // A single character.
            if(firstChar=='\'')
            {
                //Three cases:
                // '\c' for escaping
                // 'c' for direct chars.
                QString tokenCache;
                int pos = 1;
                while(pos < currentLine.size() && currentLine.at(pos) != '\'')
                {
                    tokenCache.append(currentLine.at(pos++));
                }
                // Translate the data from the escape.
                QChar tokenChar = fromEscape(tokenCache);
                if(hasError())
                {
                    // Error occurs.
                    return;
                }
                // Append the char token.
                appendToken(tokenChar);
                m_column+=pos+1;
                currentLine = currentLine.mid(pos+1);
                skipAndCheckEmpty();
                continue;
            }
            // Treat as a simple string.
            QString tokenCache;
            int pos = 0;
            while(pos < currentLine.size() && !currentLine.at(pos).isSpace() &&
                  !specChars.contains(currentLine.at(pos)))
            {
                tokenCache.append(currentLine.at(pos++));
            }
            appendToken(tokenCache);
            // Move to the new position.
            m_column+=pos;
            currentLine = currentLine.mid(pos);
            skipAndCheckEmpty();
        }
    }
    if(m_tokenList.isEmpty())
    {
        m_current = QVariant();
        m_pos = QPoint();
    }
    else
    {
        Token token = m_tokenList.takeFirst();
        m_current = token.token;
        m_pos = token.pos;
    }
}

inline int Tokenizer::parseNumber(const QString &validChars,
                                  const QString &currentLine,
                                  int base)
{
    // Prepare the number cache.
    QString numCache;
    // Loop from the correct position.
    int pos = (base == 10) ? 0 : 2;
    if(base==10 && currentLine.at(0)=='-')
    {
        numCache = "-";
        pos = 1;
    }
    while(pos < currentLine.size() &&
          validChars.contains(currentLine.at(pos)))
    {
        numCache.append(currentLine.at(pos++));
    }
    // Append the token.
    appendToken((quint32)numCache.toInt(nullptr, base));
    // Update the column
    m_column += pos;
    return pos;
}

inline void Tokenizer::appendToken(QVariant token)
{
    Token tokenItem;
    tokenItem.token = token;
    tokenItem.pos = QPoint(m_row, m_column);
    m_tokenList.append(tokenItem);
}

void Tokenizer::dumpTokens()
{
    for(auto i : m_tokenList)
    {
        qDebug()<<i.token;
    }
}

QChar Tokenizer::fromEscape(const QString &escape)
{
    // Single char.
    if(escape.size() == 1)
    {
        if(escape.at(0)=='\\')
        {
            m_errorInfo = "Empty char escape '\\'. "
                          "If you mean by char '\\', please use '\\\\'.";
            m_errorPos = QPoint(m_row, m_column);
            return QChar();
        }
        return escape.at(0);
    }
    if(m_escapeMap.contains(escape))
    {
        return m_escapeMap.value(escape);
    }
    // Error happened.
    m_errorInfo = QString("Unknown char escape '%1'.").arg(escape);
    m_errorPos = QPoint(m_row, m_column);
    return QChar();
}

QHash<QString, QChar> Tokenizer::escapeMap() const
{
    return m_escapeMap;
}

int Tokenizer::column() const
{
    return m_column;
}

int Tokenizer::row() const
{
    return m_row;
}

QString Tokenizer::errorInfo() const
{
    return m_errorInfo;
}

QPoint Tokenizer::errorPos() const
{
    return m_errorPos;
}

QPoint Tokenizer::pos() const
{
    return m_pos;
}

QVariant Tokenizer::takeCurrent()
{
    QVariant current = m_current;
    next();
    return current;
}

bool Tokenizer::hasError() const
{
    return !m_errorInfo.isEmpty();
}
