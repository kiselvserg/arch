#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cpu = NULL;
    lineNumber = 0;
    ui->program->horizontalHeader()->resizeSections(QHeaderView::Fixed);
    //on_openFileAction_triggered();
    ui->registers->horizontalHeader()->resizeSection(0,40);
    ui->registers->horizontalHeader()->resizeSection(1,35);
    for(int i = 0; i < 24; i++)
        ui->registers->verticalHeader()->resizeSection(i,20);
    for(int i = 0; i < 16; i++)
    {
        ui->program->horizontalHeader()->resizeSection(i,35);
    }
    for(int i = 0; i < 2048; i++)
        ui->program->verticalHeader()->resizeSection(i,20);
    ui->nextStepButton->setEnabled(false);
    ui->pushButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(cpu) delete cpu;
}

void MainWindow::on_openFileAction_triggered()
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open Program"), "", tr("ASM Files (*.asm)"));
    //fileName = "E:\\test1.asm";
    if(!fileName.isEmpty())
    {
        file.setFileName(fileName);
        file.open(QIODevice::ReadOnly);
        program = file.readAll();
        file.close();
        ui->textEdit->setText(program);
        cpu = new CPU(program);
        ui->nextStepButton->setEnabled(true);
        ui->pushButton->setEnabled(true);
        this->updateState(0);
    }
}

void MainWindow::on_nextStepButton_clicked()
{
    if(cpu->nextStep())
    {
        QMessageBox::information(this, "Программа выполнена", "Программа успешно выполнена", QMessageBox::Ok);
        qDebug() << "All done";
        ui->nextStepButton->setEnabled(false);
    }
    updateState();
}

void MainWindow::updateState(int mode)
{
    for(int i = 0; i < 8; i++) // регистры
        ui->registers->setItem(i,1, new QTableWidgetItem(QString::number(cpu->sysRegs[ui->registers->item(i,0)->text()])));
    for(int i = 0; i < 16; i++) // регистры
        ui->registers->setItem(i+8, 1, new QTableWidgetItem(QString::number(cpu->regs[i])));

    for(int i = 0; i < cpu->data.size(); i++)
    {
        int row = i / 16;
        int col = i % 16;
        ui->program->setItem(row, col, new QTableWidgetItem(cpu->data[i].first));
    }
    ui->program->setCurrentCell(cpu->getRegister("PC") / 16, cpu->getRegister("PC") % 16);
    int f = cpu->getRegister("F");
    ui->label_6->setText(QString::number((f & 0x10) >> 4));
    ui->label_7->setText(QString::number((f & 0x8) >> 3));
    ui->label_8->setText(QString::number((f & 0x4) >> 2));
    ui->label_9->setText(QString::number((f & 0x2) >> 1));
    ui->label_10->setText(QString::number(f & 0x1));


    QTextEdit::ExtraSelection highlight;
    highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
    highlight.format.setBackground(/*QColor(0x74,0xd7,0x93)*//*QColor(0xFF, 0xDE, 0xAD)*/QColor(0x9F, 0xB6, 0xCD));

    if(mode) // next_Button pressed
    {
        int go;
        QTextCursor curs = ui->textEdit->textCursor();
        if(cpu->isJMP)
        {
            qDebug() << "isJMP";
            if(cpu->go - cpu->lll < 0) go = cpu->go;
            else go = cpu->go - cpu->lll;
            curs.setPosition(ui->textEdit->document()->findBlockByLineNumber(go).position());
            lineNumber = go - 1;
            qDebug() << "linenumber" << lineNumber;
            cpu->isJMP = false;
            //curs.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor,
                              //ui->textEdit->document()->findBlockByLineNumber(cpu->go - cpu->lll).length());
        }
        else
        {
            //curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor, 2);
            curs.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, ui->textEdit->document()->findBlockByLineNumber(lineNumber).length());
        }
        highlight.cursor = curs;
        qDebug() << "cursor position1" << ui->textEdit->textCursor().position();
        ui->textEdit->setTextCursor(curs);
        lineNumber++;
    }
    else
    {

        QTextCursor curs = ui->textEdit->textCursor();
        curs.movePosition(QTextCursor::Start);
        //curs.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, )
        ui->textEdit->setTextCursor(curs);
        qDebug() << "cursor position2" << ui->textEdit->textCursor().position();
        highlight.cursor = curs;
        lineNumber = 0;
    }

    QList<QTextEdit::ExtraSelection> extras;
    extras << highlight;
    ui->textEdit->setExtraSelections(extras);
}

void MainWindow::on_pushButton_clicked()
{
    cpu->setRegister("PC", 0);
    cpu->setRegister("PCI", 0);
    cpu->setRegister("F", 0);
    cpu->setRegister("FI", 0);
    cpu->setRegister("CI", 0);
    cpu->setRegister("CIA", 0);
    cpu->setRegister("NI", 0);
    cpu->setRegister("NIA", 0);
    for(int i = 0; i < 16; i++)
        cpu->regs[i] = 0;
    cpu->lll = 0;
    updateState(0);
    ui->nextStepButton->setEnabled(true);
}

void MainWindow::on_aboutAction_triggered()
{
    QMessageBox::about(this, "О программе",
                       tr("<h2>Программная модель процессора</h2>")+
                       tr("Программа представляет собой программную модель процессора, <br/>")+
                       tr("архитектура которого была разработана в рамках курсового проектирования <br/>")+
                       tr("по дисциплине Архитектура ВС <br/><br/>")+
                       tr("<b>Выполнил студент группы 0305 Киселев Сергей</b><br/><br/>")+
                       tr("<center>Санкт-Петербург, 2013 г.</center>")+
                       tr("<center> СПбГЭТУ \"ЛЭТИ\" "));
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, "О библиотеке Qt");
}
