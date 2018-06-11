#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QPoint>
#include <QHash>

#include <QObject>

class Tokenizer;
class PeanutMemory;
class Assembler : public QObject
{
    Q_OBJECT
public:
    explicit Assembler(QObject *parent = nullptr);

    bool hasError();
    QString error() const;
    QPoint errorPos() const;

signals:
    void warning(QString warningInfo, QPoint warningPos);

public slots:
    void setSourceCode(const QString &sourceCode);
    void setMemory(PeanutMemory *memory);
    bool assemble();

private:
    inline void setError(const QString &errorInfo,
                         const QPoint &errorPos = QPoint());
    inline bool parseImmedate(quint16 &immedateValue,
                              quint16 address,
                              const QString &errorInfo);
    inline bool parseAddress(quint16 &addressValue,
                             quint16 address,
                             const QString &errorInfo);
    inline bool parseRegId(quint8 &regId, const QString &errorInfo);
    inline bool parseChar(char targetChar, const QString &errorInfo);
    inline bool parseChar(char *value, const QString &errorInfo);
    inline bool toRegId(const QString &name, quint8 &regId);
    inline void appendPostWork(const quint32 &address,
                               const QString &labelName);
    struct PostTask
    {
        quint16 address;
        QPoint pos;
        QString labelName;
    };

    QHash<QString, quint32> m_aluMap;
    QHash<QString, quint32> m_alu2Map;
    QHash<QString, quint32> m_jumpMap;
    QHash<QString, quint32> m_setMap;
    QList<PostTask> m_postWorkQueue;
    QHash<QString, quint8> m_regIdMap;
    QHash<QString, quint16> m_labelMap;
    QString m_errorInfo;
    QPoint m_errorPos;
    Tokenizer *m_tokenizer;
    PeanutMemory *m_memory;
};

#endif // ASSEMBLER_H
