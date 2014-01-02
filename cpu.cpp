#include "cpu.h"

CPU::CPU(QByteArray input) : flags("zcsoi"), regs {0}, offset(0), lll(0), isJMP(false)
{
    opCodes = {"", "mov", "mov64", "mul", "add", "sub", "jz", "ret", "jmp", "jnz", "ie", "id"}; // все возможные команды
    sysRegs = { {"PC", 0},
                {"PCI", 0},
                {"F", 0},
                {"FI", 0},
                {"CI", 0},
                {"CIA", 0},
                {"NI", 0},
                {"NIA", 0}
              };
    this->parse(input);
}

void CPU::parse(QByteArray input)
{
    qDebug() << "opCodes:" << opCodes;
    QString in = input;
    QStringList string;
    if(input.isEmpty()) return;
        //throw Exception("Файл пуст!");
    string = in.split('\n'); // делим построчно файл
    for(QStringList::Iterator i = string.begin(); i < string.end(); i++)
        this->parseCommand(i->split(" ", QString::SkipEmptyParts)); // парсим каждую строку поотдельности
    qDebug() << data;
}

void CPU::parseCommand(QStringList command) // на входе одна команда
{
    // первый операнд всегда регистр
    int opCode = 0;
    qDebug() << "len" << command.length();
    switch(command.length())
    {
        case 3: // коп + 2 операнда
            isJMP = false;
            opCode = opCodes.indexOf(command[0]);
            qDebug() << "case3" << opCode;
            if(opCode == -1) return;
                /** @todo
                * throw Exception("Нет такой команды");
                */
            // первый операнд всегда регистр
            if(command[2][0] == 'r') // второй операнд регистр
            {
                opCode = (opCode << 8); // по 4 бита на каждый регистр
                //qDebug() << "opcode" << QString::number(opCode, 2);
                QStringList r1 = command[1].split("r", QString::SkipEmptyParts); // достаем номер регистра1
                QStringList r2 = command[2].split("r", QString::SkipEmptyParts); // регистра2
                int reg1 = r1.first().toInt();
                reg1 = reg1 << 4;
                int reg2 = r2.first().toInt();
                opCode |= reg1;
                opCode |= reg2;
                //qDebug() << "opcode" << QString::number(opCode, 2);
                data.push_back(qMakePair(QString::number(opCode, 16).toUpper(), 2));
            }
            else // второй операнд - число
            {
                long long number = command[2].toLongLong();
                QStringList r = command[1].split("r", QString::SkipEmptyParts);
                int reg = r.first().toInt(); // достаем номер регистра
                if((number >= 0) && (number < 256)) // 8-разрядная константа
                {
                    opCode = (opCode << 5); // 5 бит для признака длины операнда и номера регистра
                    opCode |= reg;
                    data.push_back(qMakePair(QString::number(opCode, 16).toUpper(), 1));
                    data.push_back(qMakePair(QString::number(number, 10).toUpper(), 0));
                }
                if((number > 255) && (number < 4294967296)) // 32-разрядная константа
                {
                    opCode = (opCode << 5); // 5 для всяких флагов
                    opCode |= 0x10; // признак 32-разрядной константы
                    opCode |= reg;
                    data.push_back(qMakePair(QString::number(opCode, 16).toUpper(), 1));
                    data.push_back(qMakePair(QString::number(number, 10).toUpper(), 0));
                }
                if(number > 4294967295) // 64-разрядная константа
                {
                    opCode = (opCode << 5); // 4 для номера регистра
                    opCode |= reg;
                    int l = (int32_t)(number); // младшая часть
                    int h = (int)(number >> 32); // старшая часть
                    data.push_back(qMakePair(QString::number(opCode, 16).toUpper(), 1));
                    data.push_back(qMakePair(QString::number(h, 10).toUpper(), 0));
                    data.push_back(qMakePair(QString::number(l, 10).toUpper(), 0));
                }
            }
            break;
        case 2: // всего лишь 1 операнд: коп + операнд (команды передачи управления)
            {
                opCode = opCodes.indexOf(command[0]);
                qDebug() << "case2" << opCode;
                QStringList r = command[1].split("r", QString::SkipEmptyParts);
                //if(r.length() == 0) // нет регистра, а число
                int reg = r.first().toInt(); // достаем номер регистра
                opCode = (opCode << 4); // для номера регистра
                opCode |= reg;
                data.push_back(qMakePair(QString::number(opCode, 16).toUpper(), 4));
                break;
                qDebug() << "next" << opCode;
            }
        case 1: // ret, ie, id
            isJMP = false;
            opCode = opCodes.indexOf(command[0]);
            qDebug() << "case1" << opCode;
            data.push_back(qMakePair(QString::number(opCode, 16).toUpper(), 3));
            break;
    }
}

int CPU::getRegister(QString reg)
{
    return sysRegs.value(reg);
}

int CPU::getRegister(int reg)
{
    return regs[reg];
}

void CPU::setRegister(int id, int value)
{
    regs[id] = value;
}

void CPU::setRegister(QString name, int value)
{
    sysRegs[name] = value;
}

void CPU::setFlag(QChar flag, bool value)
{
    int mask = 1 << (4 - flags.indexOf(flag));
    if(value) // true
        this->setRegister("F", this->getRegister("F") | mask);
    else // false
        this->setRegister("F", this->getRegister("F") & ~mask);
}

bool CPU::getFlag(QChar flag)
{
    int mask = 1 << (4 - flags.indexOf(flag));
    return (this->getRegister("F") & mask) == mask;
}

bool CPU::nextStep()
{
//    qDebug() << "PC" << this->getRegister("PC");
//    qDebug() << "CI" << this->getRegister("CI");
//    qDebug() << "CIA" << this->getRegister("CIA");
    offset = this->getRegister("PC");
    bool ok;
    int command = data[offset].first.toInt(&ok, 16);
    int opCode = 0, source = 0, dest = 0, value_source = 0, value_dest;
    switch(data[offset].second) // формат команды (см. в функции parseCommand)
    {
    case 1: // непосредственная
        offset++;
        dest = (command & 0xF);
        source = 0; // второго операнда нет
        value_source = data[offset++].first.toInt();
        opCode = (command >> 5);
        lll++;
        break;
    case 2: // регистровая
        offset++;
        opCode = (command >> 8);
        dest = ((command >> 4) & 0xF);
        source = (command & 0xF);
        value_source = regs[source];
        value_dest = regs[dest];
        break;
    case 3:
        offset++;
        opCode = command;
        qDebug() << "type3opcode" << opCode;
        break;
    case 4:
        offset++;
        opCode = (command >> 4);
        dest = (command & 0xF);
        break;
    }

    switch (opCode) // что за команда и что с ней делать
    {
    case 0b001: // mov
        if(source == 0) // непосредственная адресация
        {
            this->setRegister(dest, value_source);
            qDebug() << "r"+QString::number(dest)+" =" << getRegister(dest);
        }
        else
        {
            this->setRegister(dest, value_source);
            qDebug() << "dest" << getRegister(dest) << "source" << getRegister(source);
        }
        break;
    case 0b100: // add
        this->setRegister(dest, (value_dest + value_source));
        this->setFlag('z', (value_dest + value_source) == 0);
        this->setFlag('s', (value_dest + value_source) < 0);
        qDebug() << "add" << getRegister(dest);
        qDebug() << "flags:" << QString::number(getRegister("F"), 2);
        break;
    case 0b101: // sub
        this->setRegister(dest, (value_dest - value_source));
        this->setFlag('z', (value_dest - value_source) == 0);
        this->setFlag('s', (value_dest - value_source) < 0);
        qDebug() << "sub" << getRegister(dest);
        qDebug() << "flags:" << QString::number(getRegister("F"), 2);
        break;
    case 0b011: // mul
        this->setRegister(dest, (value_dest * value_source));
        this->setFlag('z', (value_dest * value_source) == 0);
        this->setFlag('s', (value_dest * value_source) < 0);
        qDebug() << "mul" << getRegister(dest);
        qDebug() << "flags:" << QString::number(getRegister("F"), 2);
        break;
    case 0b110: // jz
        qDebug() << "flag z" << this->getFlag('z');
        if(!this->getFlag('z'))
            break;
        else
        {
            isJMP = true;
            go = regs[dest];
            this->setRegister("PC", regs[dest]);
            qDebug() << "PC" << this->getRegister("PC");
            return false;
        }
    case 0b1000: // jmp
        this->setRegister("PC", regs[dest]);
        qDebug() << "PC" << this->getRegister("PC");
        isJMP = true;
        go = regs[dest];
        return false;
    case 0b1001: // jnz
        if(this->getFlag('z'))
            break;
        isJMP = true;
        go = regs[dest];
        this->setRegister("PC", regs[dest]);
        qDebug() << "PC" << this->getRegister("PC");
        return false;
        case 0xA: // ie
            this->setFlag('i', true);
            break;
        case 0xB: // id
            this->setFlag('i', false);
            break;
    case 0b111: // ret
        return true;
    default:
        qDebug() << "ERRORR!!!";
        break;
    }
    this->setRegister("PC", offset);
    this->setRegister("CIA", offset);
    this->setRegister("CI", this->data[offset].first.toInt());
    return false;
}
