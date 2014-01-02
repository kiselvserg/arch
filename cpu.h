#ifndef CPU_H
#define CPU_H
#include <QByteArray>
#include <QStringList>
#include <QRegExp>
#include <QDebug>

class CPU
{
public:
    CPU(QByteArray input);
    bool nextStep();
    int getRegister(QString reg);
    void setRegister(QString name, int value);
    QVector<QPair<QString, int> > data; // сами данные и формат команды
    QVector<QString> opCodes;
    QMap<QString, int> sysRegs;
    QString flags;
    int regs[16];
    int offset;
    int lll; // лишние ячейки
    bool isJMP;
    int go;
private:
    void parse(QByteArray input);
    void parseCommand(QStringList command);
    int getRegister(int reg);
    void setRegister(int id, int value);
    void setFlag(QChar flag, bool value);
    bool getFlag(QChar flag);
//    QVector<QPair<QString, int> > data; // сами данные и формат команды
//    QVector<QString> opCodes;
//    QMap<QString, int> sysRegs;
//    QString flags;
//    int regs[16];
//    int offset;
};

#endif // CPU_H
