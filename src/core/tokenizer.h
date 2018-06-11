#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <QVariant>
#include <QPoint>
#include <QHash>

#include <QObject>

class Tokenizer : public QObject
{
    Q_OBJECT
public:
    struct Token
    {
        QVariant token;
        QPoint pos;
    };

    explicit Tokenizer(QObject *parent = nullptr);

    void setSourceCode(const QString &sourceCode);
    void reset();

    bool hasToken() const;
    bool parse(const QVariant &expected);
    QVariant current() const;
    QPoint pos() const;
    QVariant takeCurrent();

    bool hasError() const;
    QString errorInfo() const;
    QPoint errorPos() const;

    int row() const;
    int column() const;

    QHash<QString, QChar> escapeMap() const;

signals:

public slots:
    void next();

private:
    inline int parseNumber(const QString &validChars,
                           const QString &currentLine,
                           int base);
    inline void appendToken(QVariant token);
    inline void dumpTokens();
    inline QChar fromEscape(const QString &escape);
    static QString binChars, hexChars, decChars, specChars;
    QList<Token> m_tokenList;
    QHash<QString, QChar> m_escapeMap;
    QStringList m_sourceLines;
    QVariant m_current;
    QString m_errorInfo;
    QPoint m_pos, m_errorPos;
    int m_row, m_column;
};

#endif // TOKENIZER_H
