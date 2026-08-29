#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QTableWidget>
#include <QTableWidgetItem>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    serialPort(new QSerialPort(this))
{
    ui->setupUi(this);
    QFont font("Courier New", 10);
    ui->tableView->setFont(font);
    ui->tableView->horizontalHeader()->setFont(font);
    ui->tableView->verticalHeader()->setFont(font);

    // Автоматически сужаем колонки под размер "FF"
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    hexModel = new HexModel(this);
    ui->tableView->setModel(hexModel);

    // В Qt 6.5 синтаксис сигналов использует указатели на методы
    connect(this, &MainWindow::chipUpdated, this, &MainWindow::resizeBuffers);

    updatePortsConnection = connect(&updatePortsTimer, &QTimer::timeout, this, &MainWindow::reload_ports);
    updatePortsTimer.setInterval(1000);
    updatePortsTimer.start();

    ui->lcdNumber->setDigitCount(8);
    ui->lcdNumber->display("00:00:00");
    connect(&processTimer, &QTimer::timeout, this, &MainWindow::updateProcessTimer);

    reload_ports();

    this->setFixedSize(QSize(371, 431));
}

MainWindow::~MainWindow()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
    delete ui;
}

void MainWindow::updateProcessTimer()
{
    qint64 elapsedMs = eraseTimer.elapsed();
    QString formattedTime = QTime::fromMSecsSinceStartOfDay(elapsedMs).toString("hh:mm:ss");
    ui->lcdNumber->display(formattedTime);
}

void MainWindow::log(const QString &str)
{
    // Гарантируем, что если строка не содержит явного HTML-тега цвета,
    // она всегда будет напечатана обычным черным цветом (#000000)
    if (!str.contains("<font")) {
        ui->textBrowser->append("<font color='#000000'>" + str + "</font>");
    } else {
        ui->textBrowser->append(str);
    }

    ui->textBrowser->moveCursor(QTextCursor::End);
}

void MainWindow::chipSelectSetEnabled(bool state)
{
    ui->KB32Button->setEnabled(state);
    ui->KB512Button->setEnabled(state);
    ui->MB1Button->setEnabled(state);
    ui->MB2Button->setEnabled(state);
    ui->MB3Button->setEnabled(state);
    ui->MB4Button->setEnabled(state);
    ui->MB8Button->setEnabled(state);
    ui->autoRomButton->setEnabled(state);

    ui->KB32Button->setAutoExclusive(state);
    ui->KB512Button->setAutoExclusive(state);
    ui->MB1Button->setAutoExclusive(state);
    ui->MB2Button->setAutoExclusive(state);
    ui->MB3Button->setAutoExclusive(state);
    ui->MB4Button->setAutoExclusive(state);
    ui->MB8Button->setAutoExclusive(state);
    ui->autoRomButton->setAutoExclusive(state);

    if (!state) {
        ui->KB32Button->setChecked(false);
        ui->KB512Button->setChecked(false);
        ui->MB1Button->setChecked(false);
        ui->MB2Button->setChecked(false);
        ui->MB3Button->setChecked(false);
        ui->MB4Button->setChecked(false);
        ui->MB8Button->setChecked(false);
        ui->autoRomButton->setChecked(false);
    }
}

void MainWindow::chipSelectSetEnDes(bool state)
{
    ui->KB32Button->setEnabled(state);
    ui->KB512Button->setEnabled(state);
    ui->MB1Button->setEnabled(state);
    ui->MB2Button->setEnabled(state);
    ui->MB3Button->setEnabled(state);
    ui->MB4Button->setEnabled(state);
    ui->MB8Button->setEnabled(state);
    ui->autoRomButton->setEnabled(state);
}

void MainWindow::on_connectButton_clicked()
{
    QListWidgetItem* item = ui->portList->currentItem();
    if (!item) {
        QMessageBox::critical(this, tr("16BitFlash Programmer"), tr("Select serial port!"));
        return;
    }
    if (!(item->flags() & Qt::ItemIsSelectable)) {
        QMessageBox::critical(this, tr("16BitFlash Programmer"), tr("Port is busy!"));
        return;
    }
    log(QString("Connect to %1").arg(item->data(Qt::UserRole).toString()));
    openSerialPort(item->data(Qt::UserRole).toString());
}

void MainWindow::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError || error == QSerialPort::PermissionError) {
        log("Warning: USB device was disconnected!");
        closeSerialPort();
        reload_ports();
    }
}

void MainWindow::openSerialPort(const QString &path)
{
    ui->connectButton->setEnabled(false);
    ui->portList->setEnabled(false);

    serialPort->setPortName(path);
    serialPort->setBaudRate(QSerialPort::Baud115200);
    if (serialPort->open(QIODevice::ReadWrite)) {
        serialPort->setDataTerminalReady(true);
        serialPort->setRequestToSend(true);
        connect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleSerialError, Qt::UniqueConnection);

        writeData("p");
        QByteArray readData;

        QElapsedTimer timeoutTimer;
        timeoutTimer.start();

        while (timeoutTimer.elapsed() < 3000) {
            if (serialPort->waitForReadyRead(100)) {
                readData.append(serialPort->readAll());
                if (readData.contains("Arduino 29 Series programmer")) {
                    log("Connect successful");

                    chipSelectSetEnabled(true);
                    ui->disconnectButton->setEnabled(true);
                    ui->connectButton->setEnabled(false);
                    ui->portList->setEnabled(false);
                    ui->eraseChipButton->setEnabled(true);

                    updateButtons(true, false);
                    return;
                }
            }
        }
        log("Arduino programmer not found.");
        closeSerialPort();
    } else {
        QMessageBox::critical(this, tr("Error"), serialPort->errorString());
        ui->connectButton->setEnabled(true);
        ui->portList->setEnabled(true);
    }
}

void MainWindow::on_disconnectButton_clicked()
{
    closeSerialPort();
}

void MainWindow::closeSerialPort()
{
    chipSelected = false;
    bufferClear = true;

    processTimer.stop();
    ui->lcdNumber->display("00:00:00");

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    ui->portList->setEnabled(true);

    ui->disconnectButton->setEnabled(false);
    ui->connectButton->setEnabled(false);
    updatePortsTimer.start();

    updateButtons(false, false);
    chipSelectSetEnabled(false);
    ui->showButton->setChecked(false);

    allowSleep();

    if (serialPort->isOpen()) {
        disconnect(serialPort, &QSerialPort::errorOccurred, this, &MainWindow::handleSerialError);
        log("Disconnect...");
        disconnect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readData);
        serialPort->close();
    }
}

void MainWindow::on_portList_itemClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    ui->connectButton->setEnabled(true);
}

void MainWindow::reload_ports()
{
    if (serialPort && serialPort->isOpen()) {
        ui->connectButton->setEnabled(false);
        ui->portList->setEnabled(false);
        return;
    }

    QString currentSelectedPort;
    if (ui->portList->currentItem()) {
        currentSelectedPort = ui->portList->currentItem()->data(Qt::UserRole).toString();
    }

    ui->portList->clear();
    const auto infos = QSerialPortInfo::availablePorts();

    QListWidgetItem *itemToSelect = nullptr;
    QListWidgetItem *firstAvailableItem = nullptr;

    for (const QSerialPortInfo &info : infos) {
        QListWidgetItem *item = new QListWidgetItem(info.portName(), ui->portList);
        QString systemPath = info.systemLocation();
        item->setData(Qt::UserRole, systemPath);

        QSerialPort port(info);
        bool isBusy = !port.open(QIODevice::ReadWrite);
        if (isBusy) {
            item->setText(info.portName() + " (Busy)");
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        } else {
            port.close();
            if (!firstAvailableItem) {
                firstAvailableItem = item;
            }
        }

        if (!systemPath.isEmpty() && systemPath == currentSelectedPort) {
            itemToSelect = item;
        }
    }

    ui->portList->setEnabled(true);

    if (itemToSelect) {
        ui->portList->setCurrentItem(itemToSelect);
        ui->connectButton->setEnabled(true);
    } else if (firstAvailableItem) {
        ui->portList->setCurrentItem(firstAvailableItem);
        ui->connectButton->setEnabled(true);
    } else {
        ui->connectButton->setEnabled(false);
    }
}

void MainWindow::updateButtons(bool actions, bool buffer)
{
    ui->disconnectButton->setEnabled(actions);

    if (actions) {
        ui->writeChipButton->setEnabled(!bufferClear);
        ui->verifyChipButton->setEnabled(!bufferClear);
        ui->readChipButton->setEnabled(chipSelected);
    } else {
        ui->writeChipButton->setEnabled(false);
        ui->verifyChipButton->setEnabled(false);
        ui->readChipButton->setEnabled(false);
    }
    ui->eraseChipButton->setEnabled(actions);

    if (buffer) {
        ui->saveFileButton->setEnabled(!bufferClear);
        ui->showButton->setEnabled(!bufferClear);
    } else {
        ui->saveFileButton->setEnabled(false);
        ui->showButton->setEnabled(false);
    }
    ui->openFileButton->setEnabled(actions);
}

void MainWindow::on_writeChipButton_clicked()
{
    writeData("w");
    eraseTimer.start();
    writeChip();
}

void MainWindow::writeData(const QByteArray &data)
{
    if (serialPort && serialPort->isOpen()) {
        serialPort->write(data);
    }
}

void MainWindow::writeChip()
{
    updateButtons(false, false);
    chipSelectSetEnDes(false);
    preventSleep();

    log(QString("Writing %1 bytes to chip...").arg(bufSize));
    ui->progressBar->setRange(0, static_cast<int>(bufSize));
    ui->progressBar->setValue(0);

    ui->lcdNumber->display("00:00:00");
    eraseTimer.start();
    processTimer.start(100);

    for (uint32_t i = 0; i < bufSize; i += 2) {
        ui->progressBar->setValue(static_cast<int>(i + 2));

        QByteArray pair;
        pair.append(bufWork[i]);
        pair.append((i + 1 < bufSize) ? bufWork[i + 1] : static_cast<char>(0xFF));

        writeData(pair);

        bool ackReceived = false;
        QElapsedTimer ackTimer;
        ackTimer.start();

        while (!ackReceived && ackTimer.elapsed() < 1500) {
            QByteArray response = serialPort->readAll();
            if (response.contains('k')) {
                ackReceived = true;
            }
            else if (ackTimer.elapsed() > 1500) {
                processTimer.stop();
                log("Error: Microcontroller did not respond!");
                updateButtons(true, true);
                chipSelectSetEnDes(true);
                allowSleep();
                return;
            }
            QCoreApplication::processEvents();
        }

        if (!ackReceived) {
            processTimer.stop();
            log("Error: Target ACK timeout!");
            log("Error: Microcontroller did not respond!");
            updateButtons(true, true);
            chipSelectSetEnDes(true);
            allowSleep();
            return;
        }
    }

    qint64 elapsedMs = eraseTimer.elapsed();
    QString formattedTime = QTime::fromMSecsSinceStartOfDay(elapsedMs).toString("hh:mm:ss");
    processTimer.stop();
    log(QString("Writing successful: %1").arg(formattedTime));
    updateButtons(true, true);
    chipSelectSetEnDes(true);
    allowSleep();
}

void MainWindow::on_readChipButton_clicked()
{
    bufferClear = false;
    bufCheck.fill(0);
    eraseTimer.start();
    readChip();
    checkClearConnection = connect(this, &MainWindow::chipReaded, this, &MainWindow::checkClear, Qt::UniqueConnection);
}

void MainWindow::readChip()
{
    updateButtons(false, false);
    chipSelectSetEnDes(false);
    preventSleep();
    log(QString("Reading %1 bytes from chip...").arg(bufSize));

    serialPort->readAll();
    bufWork.clear();

    ui->progressBar->setRange(0, static_cast<int>(bufSize));
    ui->progressBar->setValue(0);

    ui->lcdNumber->display("00:00:00");
    eraseTimer.start();
    processTimer.start(100);

    serialDataConnection = connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readData, Qt::UniqueConnection);
    writeData("r");
}

void MainWindow::readData()
{
    const QByteArray data = serialPort->readAll();
    bufWork.append(data);

    ui->progressBar->setValue(static_cast<int>(bufWork.size()));

    if (static_cast<uint32_t>(bufWork.size()) >= bufSize) {
        ui->progressBar->setValue(static_cast<int>(bufSize));
        disconnect(serialDataConnection);

        updateButtons(true, true);
        chipSelectSetEnDes(true);

        log(QString("Read %1 bytes.").arg(bufWork.size()));

        qint64 elapsedMs = eraseTimer.elapsed();
        QString formattedTime = QTime::fromMSecsSinceStartOfDay(elapsedMs).toString("hh:mm:ss");
        log(QString("Read successful: %1").arg(formattedTime));

        processTimer.stop();
        ui->progressBar->setValue(static_cast<int>(bufSize));
        disconnect(serialDataConnection);

        allowSleep();

        emit chipReaded();
        emit bufferUpdated();
    }
}

void MainWindow::checkClear()
{
    disconnect(checkClearConnection);
    bool isClear = true;
    for (uint32_t count = 0; count < bufSize; count++) {
        if (static_cast<uint8_t>(bufWork[count]) != 0xFF) {
            log("Chip not clear. Check before write.");
            isClear = false;
            break;
        }
    }
    if (isClear) log("Chip clear.");
}

void MainWindow::on_openFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open binary to buffer"),
                                                    "",
                                                    tr("Binary Files (*.bin *.gen *.md);;BIN (*.bin);;GEN (*.gen);;MD (*.md);;All Files (*)"));

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    QByteArray fileData = file.readAll();

    if (fileData.size() > bufSize) {
        bufSize = static_cast<uint32_t>(fileData.size());
    }

    bufWork.resize(static_cast<qsizetype>(bufSize));
    bufWork.fill(static_cast<char>(0xFF));

    memcpy(bufWork.data(), fileData.constData(), fileData.size());

    log(QString("Load from %1 file").arg(fileName));
    log(QString("Read %1 MB. Total buffer size: %2 bytes")
            .arg((double)fileData.size() / (1024.0 * 1024.0), 0, 'f', 2).arg(bufSize));

    switch (bufSize) {
    case 0x00008000: ui->KB32Button->setChecked(true); writeData("a"); break;     // 32 KB[cite: 1]
    case 0x00080000: ui->KB512Button->setChecked(true); writeData("b"); break;    // 512 KB[cite: 1]
    case 0x00100000: ui->MB1Button->setChecked(true); writeData("c"); break;      // 1 MB[cite: 1]
    case 0x00200000: ui->MB2Button->setChecked(true); writeData("d"); break;      // 2 MB[cite: 1]
    case 0x00300000: ui->MB3Button->setChecked(true); writeData("q"); break;    // 3 MB[cite: 1]
    case 0x00400000: ui->MB4Button->setChecked(true); writeData("e"); break;      // 4 MB[cite: 1]
    case 0x00800000: ui->MB8Button->setChecked(true); writeData("f"); break;      // 8 MB[cite: 1]
    default:
        on_autoRomButton_clicked();
        ui->autoRomButton->setChecked(true);
        log(QString("Auto ROM size mode enabled: %1MB (%2 bytes)")
                .arg((double)bufSize / (1024.0 * 1024.0), 0, 'f', 2)
                .arg(bufSize));
        sendAutoRomSize();
        break;
    }

    chipSelected = true;
    bufCheck.resize(static_cast<qsizetype>(bufSize));
    bufCheck.fill(static_cast<char>(0xFF));

    bufferClear = false;
    updateButtons(true, true);
    emit bufferUpdated();
}

void MainWindow::on_saveFileButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save buffer"),
                                                    "",
                                                    tr("Binary Files (*.bin *.gen *.md);;BIN (*.bin);;GEN (*.gen);;MD (*.md);;All Files (*)"));

    if (fileName.isEmpty()) return;

    if (!fileName.endsWith(".bin", Qt::CaseInsensitive) &&
        !fileName.endsWith(".gen", Qt::CaseInsensitive) &&
        !fileName.endsWith(".md", Qt::CaseInsensitive)) {
        fileName.append(".bin");
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    file.write(bufWork);
    file.close();
    log(QString("Buffer saved to %1 file").arg(fileName));
}

void MainWindow::verifyData()
{
    uint32_t errors_count = 0;
    uint32_t warnings_count = 0;

    QByteArray errorMap;
    errorMap.resize(static_cast<qsizetype>(bufSize));

    for (uint32_t i = 0; i < bufSize; i++) {
        uint8_t orig = static_cast<uint8_t>(bufWork[i]);
        uint8_t check = (i < static_cast<uint32_t>(bufCheck.size())) ? static_cast<uint8_t>(bufCheck[i]) : 0xFF;

        if ((orig ^ check) & check) {
            errors_count++;
            errorMap[i] = NOT_WRITABLE; // Красный в HexView
        } else if (orig != check) {
            warnings_count++;
            errorMap[i] = WRITABLE;     // Горчичный в HexView
        } else {
            errorMap[i] = NO_ERR;       // Обычный цвет (нет ошибок)
        }
    }

    bufCheck = errorMap;

    updateButtons(true, true);
    chipSelectSetEnDes(true);

    if (errors_count || warnings_count) {
        log("<font color='#D32F2F'><b>Verification failed!</b></font>");

        if (errors_count > 0) {
            log(QString("<font color='#D32F2F'><b>Errors (Requires Erase): %1</b></font>").arg(errors_count));
        }

        if (warnings_count > 0) {
            log(QString("<font color='#9E7B05'><b>Warnings (Underwrite): %1</b></font>").arg(warnings_count));
        }
    } else {
        log("<font color='#2E7D32'><b>Verification successful! All bytes match.</b></font>");
    }

    emit bufferUpdated();
}

void MainWindow::on_showButton_toggled(bool checked)
{
    if (checked) {
        this->setFixedSize(QSize(791, 431));
        updateBufConnection = connect(this, &MainWindow::bufferUpdated, this, &MainWindow::showBuf, Qt::UniqueConnection);
        emit bufferUpdated();
    } else {
        this->setFixedSize(QSize(371, 431));
        disconnect(updateBufConnection);
    }
}

void MainWindow::showBuf()
{
    hexModel->updateData(&bufWork, &bufCheck);
}

void MainWindow::resizeBuffers()
{
    chipSelected = true;

    ui->openFileButton->setEnabled(true);
    ui->readChipButton->setEnabled(true);

    int oldSize = bufWork.size();
    bufWork.resize(static_cast<qsizetype>(bufSize));

    if (bufWork.size() > oldSize) {
        std::fill(bufWork.begin() + oldSize, bufWork.end(), static_cast<char>(0xFF));
    }

    bufCheck.resize(static_cast<qsizetype>(bufSize));
    bufCheck.fill(static_cast<char>(0xFF));

    emit bufferUpdated();
}

void MainWindow::on_verifyChipButton_clicked()
{
    log("Starting verification...");

    updateButtons(false, false);
    chipSelectSetEnDes(false);
    preventSleep();

    bufCheck.clear();
    ui->progressBar->setRange(0, static_cast<int>(bufSize));
    ui->progressBar->setValue(0);

    ui->lcdNumber->display("00:00:00");
    eraseTimer.start();
    processTimer.start(100);

    verifyDataConnection = connect(serialPort, &QSerialPort::readyRead, this, [this]() {
        const QByteArray data = serialPort->readAll();
        bufCheck.append(data);
        ui->progressBar->setValue(static_cast<int>(bufCheck.size()));

        if (static_cast<uint32_t>(bufCheck.size()) >= bufSize) {
            disconnect(verifyDataConnection);
            processTimer.stop();
            allowSleep();

            verifyData();
        }
    });

    writeData("r");
}

void MainWindow::on_KB32Button_clicked()
{
    ui->autoRomButton->setChecked(false);
    log("Selected: 32KB Test ROM.");
    bufSize = 0x00008000;
    writeData("a");
    emit chipUpdated();
}

void MainWindow::on_KB512Button_clicked()
{
    ui->autoRomButton->setChecked(false);
    log("Selected: 512KB x08 ROM");
    log("(MX29LV400BTC)");
    bufSize = 0x00080000;
    writeData("b");
    emit chipUpdated();
}

void MainWindow::on_MB1Button_clicked()
{
    ui->autoRomButton->setChecked(false);
    log("Selected: 1MB x08 ROM");
    log("(MX29LV800C,ES29LV800D,HY29F800TT(5V))");
    bufSize = 0x00100000;
    writeData("c");
    emit chipUpdated();
}

void MainWindow::on_MB2Button_clicked()
{
    ui->autoRomButton->setChecked(false);
    log("Selected: 2MB x08 ROM");
    log("(MX29LV160DT,M29W160ET,EN29LV160A,S29AL016M)");
    bufSize = 0x00200000;
    writeData("d");
    emit chipUpdated();
}

void MainWindow::on_MB3Button_clicked()
{
    ui->autoRomButton->setChecked(false);
    log("Selected: 3MB x08 ROM");
    bufSize = 0x00300000;
    writeData("q");
    emit chipUpdated();
}

void MainWindow::on_MB4Button_clicked()
{
    ui->autoRomButton->setChecked(false);
    log("Selected: 4MB x08 ROM");
    log("(MX29LV320E)");
    bufSize = 0x00400000;
    writeData("e");
    emit chipUpdated();
}

void MainWindow::on_MB8Button_clicked()
{
    ui->autoRomButton->setChecked(false);
    log("Selected: 8MB x08 ROM");
    log("(MX29LV640E)");
    bufSize = 0x00800000;
    writeData("f");
    emit chipUpdated();
}

void MainWindow::sendAutoRomSize()
{
    uint32_t wordsCount = bufSize / 2;

    QByteArray packet;
    packet.append('z');
    packet.append(static_cast<char>((wordsCount >> 24) & 0xFF));
    packet.append(static_cast<char>((wordsCount >> 16) & 0xFF));
    packet.append(static_cast<char>((wordsCount >> 8) & 0xFF));
    packet.append(static_cast<char>(wordsCount & 0xFF));

    writeData(packet);
}

void MainWindow::on_autoRomButton_clicked()
{
    log("Switched to Auto ROM size mode");

    ui->KB32Button->setAutoExclusive(false);
    ui->KB512Button->setAutoExclusive(false);
    ui->MB1Button->setAutoExclusive(false);
    ui->MB2Button->setAutoExclusive(false);
    ui->MB3Button->setAutoExclusive(false);
    ui->MB4Button->setAutoExclusive(false);
    ui->MB8Button->setAutoExclusive(false);

    ui->KB32Button->setChecked(false);
    ui->KB512Button->setChecked(false);
    ui->MB1Button->setChecked(false);
    ui->MB2Button->setChecked(false);
    ui->MB3Button->setChecked(false);
    ui->MB4Button->setChecked(false);
    ui->MB8Button->setChecked(false);

    ui->KB32Button->setAutoExclusive(true);
    ui->KB512Button->setAutoExclusive(true);
    ui->MB1Button->setAutoExclusive(true);
    ui->MB2Button->setAutoExclusive(true);
    ui->MB3Button->setAutoExclusive(true);
    ui->MB4Button->setAutoExclusive(true);
    ui->MB8Button->setAutoExclusive(true);

    if (!bufWork.isEmpty()) {
        bufSize = static_cast<uint32_t>(bufWork.size());
    }

    sendAutoRomSize();
    emit chipUpdated();
}

void MainWindow::on_eraseChipButton_toggled(bool checked)
{
    Q_UNUSED(checked);
    log("Erasing chip... Please wait.");

    ui->lcdNumber->display("00:00:00");
    eraseTimer.start();
    processTimer.start(100);

    writeData("g");

    updateButtons(false, false);
    chipSelectSetEnDes(false);
    preventSleep();

    ui->progressBar->setRange(0, 0);

    QTimer::singleShot(90000, this, &MainWindow::finishErase);
}

void MainWindow::finishErase()
{
    log("Erase cycle finished. Reading first 32 KB for blank check...");

    if (serialPort->isOpen()) {
        serialPort->readAll();
    }

    uint32_t eraseCheckSize = 0x00008000;
    bufWork.clear();
    bufSize = eraseCheckSize;

    writeData("a");
    writeData("r");

    connect(this, &MainWindow::chipReaded, this, [this]() {
            bool isClear = true;
            for (int i = 0; i < bufWork.size(); ++i) {
                if (static_cast<uint8_t>(bufWork[i]) != 0xFF) {
                    isClear = false;
                    break;
                }
            }

            if (isClear) {
                log("<font color='#2E7D32'><b>SUCCESS: Chip is fully erased! (32 KB blank check: All 0xFF)</b></font>");
            } else {
                log("<font color='#D32F2F'><b>ERROR: Chip NOT clear! Non-0xFF bytes detected in first 32 KB.</b></font>");
            }

            ui->progressBar->setRange(0, static_cast<int>(bufSize));
            ui->progressBar->setValue(static_cast<int>(bufSize));

            updateButtons(true, true);
            chipSelectSetEnDes(true);
            allowSleep();

            emit chipUpdated();
        }, Qt::SingleShotConnection);

    serialDataConnection = connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readData, Qt::UniqueConnection);
}

void MainWindow::on_progressBar_valueChanged(int value)
{
    Q_UNUSED(value);
}

void MainWindow::on_eraseChipButton_2_clicked()
{
    log("Unprotect chip...");
    writeData("h");
    emit chipUpdated();
}

void MainWindow::preventSleep()
{
#ifdef Q_OS_WIN
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
#endif
}

void MainWindow::allowSleep()
{
#ifdef Q_OS_WIN
    SetThreadExecutionState(ES_CONTINUOUS);
#endif
}

void MainWindow::on_aboutButton_clicked()
{
    QMessageBox::about(this, tr("О программе"),
                       tr("<h3>16BitFlash Programmer</h3>"
                          "<p><b>Версия:</b> 3.0.0</p>"
                          "<p><b>Разработчик:</b> Pushkash<br>"
                          "<b>AI-ассистент:</b> Gemini</p>"
                          "<hr>"
                          "<p><b>Описание:</b><br>"
                          "Программа предназначена для чтения, записи, проверки и стирания "
                          "16-битных микросхем Flash-памяти (серии MX29LV.., AM29F.., ES29LV.., HY29F.., S29AL.. и др.) "
                          "с помощью кастомного программатора на базе Arduino NANO.</p>"
                          "<p><b>Назначение:</b><br>"
                          "Разработано специально для работы с картриджами и ROM-файлами "
                          "для игровой консоли SEGA Mega Drive / Genesis.</p>"
                          "<hr>"
                          "<p style='font-size: 11px; color: #555;'>"
                          "<b>Примечание:</b> Для правильной работы устройства и связи с ПК "
                          "в микроконтроллер Arduino NANO должен быть загружен специальный скетч (Firmware).</p>"));
}
