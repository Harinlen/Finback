#ifndef TERMINALDATAIO_H
#define TERMINALDATAIO_H

#include "peanutdataio.h"

class TerminalEdit;
class TerminalDataIo : public PeanutDataIo
{
    Q_OBJECT
public:
    explicit TerminalDataIo(QObject *parent = nullptr);
    QChar read() override;
    bool isCacheEmpty() override;

signals:

public slots:
    void setEditor(TerminalEdit *editor);
    void resetCache() override;
    void write(QChar data) override;

private:
    TerminalEdit *m_editor;
    QString m_readCache;
    bool m_clearTerminal;
};

#endif // TERMINALDATAIO_H
