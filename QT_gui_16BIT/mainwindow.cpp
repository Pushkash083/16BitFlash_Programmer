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

    ui->romComboBox->clear();
    ui->romComboBox->addItems({
        "TEST32KB",
        "512KB",
        "1MB",
        "2MB",
        "4MB",
        "8MB",
        "16MB",
        "32MB"
    });
    bufSize = 0x00008000;  // Устанавливаем стартовый размер вручную (32 KB):
    ui->romComboBox->setCurrentIndex(0); // Выставляем 0-й индекс
    // Заблокируем до подключения
    ui->textBrowser->clear();
    ui->romComboBox->setEnabled(false);
    ui->autoRomButton->setEnabled(false);

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
    ui->romComboBox->setEnabled(state);
    ui->autoRomButton->setEnabled(state);
}

void MainWindow::chipSelectSetEnDes(bool state)
{
    chipSelectSetEnabled(state);
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
                    ui->readChipButton->setEnabled(true);
                    ui->detectButton->setEnabled(true);  //on_detectButton_clicked()
                    detectButtonWasEnabled = true;

                    writeData("a"); // Задаем начальный размер 32 KB на микроконтроллер, чтобы устройство точно знало текущий размер:

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
    detectedChipSize = 0; // Сбрасываем размер обнаруженного чипа при отключении!

    processTimer.stop();
    ui->lcdNumber->display("00:00:00");

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    // Сбрасываем информацию о чипе при отключении
    ui->chipInfoLabel->setText("");
    ui->sizeLabel->setText("");
    ui->idLabel->setText("");

    // --- Возвращаем romComboBox в исходное состояние (все 8 пресетов) ---
    ui->romComboBox->blockSignals(true);
    while (ui->romComboBox->count() > 8) {
        ui->romComboBox->removeItem(ui->romComboBox->count() - 1);
    }
    // Если каких-то базовых пунктов вдруг не хватало, можно восстановить полный список:
    if (ui->romComboBox->count() < 8) {
        ui->romComboBox->clear();
        ui->romComboBox->addItems({
            "TEST32KB",
            "512KB",
            "1MB",
            "2MB",
            "4MB",
            "8MB",
            "16MB",
            "32MB"
        });
    }
    ui->romComboBox->setCurrentIndex(0);
    ui->romComboBox->blockSignals(false);
    // -----------------------------------------------------------------

    ui->portList->setEnabled(true);
    ui->detectButton->setEnabled(false);
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
        // Процесс ЗАВЕРШИЛСЯ. Возвращаем кнопки в рабочее состояние.

        // Если чип уже был успешно опознан (detectedChipSize > 0),
        // то кнопка Chip ID должна оставаться заблокированной!
        if (detectedChipSize > 0) {
            ui->detectButton->setEnabled(false);
            detectButtonWasEnabled = false;
        } else {
            ui->detectButton->setEnabled(detectButtonWasEnabled);
        }

        ui->writeChipButton->setEnabled(!bufferClear);
        ui->verifyChipButton->setEnabled(!bufferClear);
        ui->readChipButton->setEnabled(chipSelected);
    } else {
        // Процесс НАЧАЛСЯ. Всё блокируем.
        detectButtonWasEnabled = ui->detectButton->isEnabled();
        ui->detectButton->setEnabled(false);

        ui->writeChipButton->setEnabled(false);
        ui->verifyChipButton->setEnabled(false);
        ui->readChipButton->setEnabled(actions);
    }

    ui->eraseChipButton->setEnabled(actions);
    ui->readChipButton->setEnabled(actions);

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
    uint32_t fileSize = static_cast<uint32_t>(fileData.size());

    // Проверяем превышение размера относительно определенного чипа
    if (detectedChipSize > 0 && fileSize > detectedChipSize) {
        log(QString("<font color='#D32F2F'><b>Warning: File size (%1 MB) exceeds detected chip size (%2 MB)! Buffer truncated to chip size.</b></font>")
                .arg((double)fileSize / (1024.0 * 1024.0), 0, 'f', 2)
                .arg((double)detectedChipSize / (1024.0 * 1024.0), 0, 'f', 2));

        // Принудительно ограничиваем буфер размером чипа, чтобы не выйти за рамки
        bufSize = detectedChipSize;
    } else {
        // Если размер в пределах нормы (или чип еще не определялся кнопкой ID), берем размер файла
        bufSize = fileSize;
    }

    // 1. Очищаем старые буферы
    bufWork.clear();
    bufCheck.clear();

    bufWork.resize(static_cast<qsizetype>(bufSize));
    bufWork.fill(static_cast<char>(0xFF));

    // Копируем в буфер только то количество байт, которое помещается в bufSize (если файл больше — он просто обрежется до размера чипа)
    memcpy(bufWork.data(), fileData.constData(), qMin(static_cast<qint64>(bufSize), (qint64)fileData.size()));

    log(QString("Load from %1 file").arg(fileName));
    log(QString("Read %1 MB. Total buffer size: %2 bytes")
            .arg((double)fileData.size() / (1024.0 * 1024.0), 0, 'f', 2).arg(bufSize));

    ui->romComboBox->blockSignals(true);

    // Восстанавливаем базовый список по размеру чипа
    const QList<quint32> standardSizes = {
        0x00008000, 0x00080000, 0x00100000, 0x00200000,
        0x00400000, 0x00800000, 0x01000000, 0x02000000
    };

    int baseCount = standardSizes.size();
    if (detectedChipSize > 0) {
        baseCount = 0;
        for (int i = 0; i < standardSizes.size(); ++i) {
            if (standardSizes[i] <= detectedChipSize) {
                baseCount = i + 1;
            }
        }
    }

    while (ui->romComboBox->count() > baseCount) {
        ui->romComboBox->removeItem(ui->romComboBox->count() - 1);
    }

    // Если файл больше чипа, принудительно выбираем максимальный индекс чипа в комбобоксе
    if (detectedChipSize > 0 && fileSize > detectedChipSize) {
        ui->romComboBox->setCurrentIndex(baseCount - 1);
    } else {
        // Иначе ищем стандартный индекс или добавляем нестандартный, если он меньше/равен чипу
        switch (bufSize) {
        case 0x00008000: ui->romComboBox->setCurrentIndex(0); break;
        case 0x00080000: ui->romComboBox->setCurrentIndex(1); break;
        case 0x00100000: ui->romComboBox->setCurrentIndex(2); break;
        case 0x00200000: ui->romComboBox->setCurrentIndex(3); break;
        case 0x00400000: ui->romComboBox->setCurrentIndex(4); break;
        case 0x00800000: ui->romComboBox->setCurrentIndex(5); break;
        case 0x01000000: ui->romComboBox->setCurrentIndex(6); break;
        case 0x02000000: ui->romComboBox->setCurrentIndex(7); break;

        default: {
            double sizeInMB = static_cast<double>(bufSize) / (1024.0 * 1024.0);
            QString customSizeStr = QString("%1MB").arg(sizeInMB, 0, 'f', 2);

            ui->romComboBox->addItem(customSizeStr);
            ui->romComboBox->setCurrentIndex(ui->romComboBox->count() - 1);

            ui->autoRomButton->setChecked(true);
            sendAutoRomSize();
            break;
        }
        }
    }

    ui->romComboBox->blockSignals(false);
    sendAutoRomSize();

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

void MainWindow::on_romComboBox_currentIndexChanged(int index)
{
    // При выборе стандартных объемов (0..6) удаляем кастомный пункт
    if (index >= 0 && index <= 7) {
        ui->autoRomButton->setChecked(false);

        ui->romComboBox->blockSignals(true);
        while (ui->romComboBox->count() > 8) {
            ui->romComboBox->removeItem(ui->romComboBox->count() - 1);
        }
        ui->romComboBox->blockSignals(false);
    }

    switch (index) {
    case 0: // TEST32KB
        log("Selected: 32KB Test ROM.");
        bufSize = 0x00008000;
        break;
    case 1: // 512KB
        log("Selected: 512KB x08 ROM (MX29LV400BTC)");
        bufSize = 0x00080000;
        break;
    case 2: // 1MB
        log("Selected: 1MB x08 ROM (MX29LV800C, ES29LV800D, HY29F800TT)");
        bufSize = 0x00100000;
        break;
    case 3: // 2MB
        log("Selected: 2MB x08 ROM (MX29LV160DT, M29W160ET, EN29LV160A, S29AL016M)");
        bufSize = 0x00200000;
        break;
    case 4: // 4MB
        log("Selected: 4MB x08 ROM (MX29LV320E)");
        bufSize = 0x00400000;
        break;
    case 5: // 8MB
        log("Selected: 8MB x08 ROM (MX29LV640E)");
        bufSize = 0x00800000;
        break;
    case 6: // 16MB
        log("Selected: 16MB ROM");
        bufSize = 0x01000000;
        break;
    case 7: // 32MB
        log("Selected: 32MB ROM");
        bufSize = 0x02000000;
        break;

    default: {
        // Кастомный выбранный элемент (индекс 7)
        ui->autoRomButton->setChecked(true);
        if (!bufWork.isEmpty()) {
            bufSize = static_cast<uint32_t>(bufWork.size());
        }
        log(QString("Selected Custom ROM: %1 MB (%2 bytes)")
                .arg((double)bufSize / (1024.0 * 1024.0), 0, 'f', 2)
                .arg(bufSize));
        break;
    }
    }

    sendAutoRomSize();
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

    if (!bufWork.isEmpty()) {
        bufSize = static_cast<uint32_t>(bufWork.size());
    }

    sendAutoRomSize();
    emit chipUpdated();
}

void MainWindow::on_eraseChipButton_toggled(bool checked)
{
    Q_UNUSED(checked);

    if (!serialPort->isOpen()) {
        log("Error: Serial port not open!");
        return;
    }

    log("Erasing chip... Please wait.");

    ui->lcdNumber->display("00:00:00");
    eraseTimer.start();
    processTimer.start(100);

    serialPort->clear(QSerialPort::AllDirections);

    updateButtons(false, false);
    chipSelectSetEnDes(false);
    preventSleep();
    ui->progressBar->setRange(0, 0);

    // Создаем страховочный таймер в виде объекта, чтобы его можно было остановить
    QTimer *safetyTimer = new QTimer(this);
    safetyTimer->setSingleShot(true);

    // 1. Настраиваем перехват ответа от Arduino
    QMetaObject::Connection *conn = new QMetaObject::Connection();
    *conn = connect(serialPort, &QSerialPort::readyRead, this, [this, safetyTimer, conn]() {
        QByteArray eraseData = serialPort->readAll();

        if (!eraseData.isEmpty()) {
            // Отменяем страховочный таймер, раз ответ пришел вовремя!
            safetyTimer->stop();
            safetyTimer->deleteLater();

            // Отключаем сигнал готовности
            disconnect(*conn);
            delete conn;

            quint8 statusByte = static_cast<quint8>(eraseData[0]);

            if (statusByte != 0xFF)
            {

             if (statusByte & 0x20)
                {
                log(QString("<font color='red'><b>Erase ERROR! Timeout / Flash worn out. Status: 0x%1</b></font>")
                        .arg(statusByte, 2, 16, QChar('0')));
                }else finishErase(); //вызывает finishErase и запускает верификацию

            } else finishErase(); // Завершаем процесс (вызывает finishErase и запускает верификацию)

        }
    });

    // 2. Настраиваем срабатывание страховочного таймера на 90 секунд
    connect(safetyTimer, &QTimer::timeout, this, [this, conn]() {
        disconnect(*conn);
        delete conn;
        log("<font color='red'><b>Erase ERROR: Timeout waiting for Arduino response!</b></font>");
        finishErase();
    });

    safetyTimer->start(90000); // Запускаем 90 секунд

    // 3. Отправляем команду на запуск стирания в Arduino
    writeData("g");
}

void MainWindow::finishErase()
{
    log("Erase cycle finished. Reading first 32 KB for blank check...");
    serialPort->clear(QSerialPort::AllDirections);
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

// Функция чтения текстовой базы данных чипов из файла chips.txt
QList<FlashInfo> MainWindow::loadFlashDatabase() {
    QList<FlashInfo> db;

    QString filePath = QCoreApplication::applicationDirPath() + "/chips.txt";
    QFile file(filePath);

    // Если файла нет, создаем его автоматически и записываем шаблон
    if (!file.exists()) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "# Format: Name,  Manufacturer_ID, Device_ID, SizeBytes(xMB*1024*1024)\n";
            out << "S29GL128S,            0x01,         0x7E21,      16777216, #16MB\n";
            out << "MX29LV320xT,          0xC2,         0xA7,         4194304, #4MB\n";
            file.close();
            log("<font color='#2E7D32'><b>chips.txt not found. Created default template automatically.</b></font>");
        }
    }

    // Теперь штатно открываем файл для чтения
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        log("<font color='#9E7B05'><b>Warning: Could not open chips.txt! Auto-ID disabled.</b></font>");
        return db;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue; // Пропускаем комментарии и пустые строки

        QStringList parts = line.split(',');
        if (parts.size() >= 4) {
            FlashInfo info;
            info.name = parts[0].trimmed();

            bool ok1, ok2;
            info.mfgId = parts[1].trimmed().toUInt(&ok1, 16);
            info.devId = parts[2].trimmed().toUInt(&ok2, 16);
            info.size = parts[3].trimmed().toLongLong();

            if (ok1 && ok2) {
                db.append(info);
            }
        }
    }
    file.close();
    return db;
}

void MainWindow::on_detectButton_clicked()
{
    if (!serialPort->isOpen()) {
        log("Error: Serial port not open!");
        return;
    }

    log("Requesting Chip ID...");
    serialPort->clear(QSerialPort::AllDirections);

    // Запрос байтов ID у МК (теперь ждем 3 байта: 1 байт MID + 2 байта CID)
    writeData("i");
    QByteArray idData;
    QElapsedTimer idTimer;
    idTimer.start();

    while (idTimer.elapsed() < 1000 && idData.size() < 3) {
        if (serialPort->waitForReadyRead(100)) {
            idData.append(serialPort->readAll());
        }
    }

    if (idData.size() >= 3) {
        quint8 mfgId = static_cast<quint8>(idData[0]);
        // Собираем 16-битный CID из двух принятых байт (старший + младший)
        quint16 devId = (static_cast<quint8>(idData[1]) << 8) | static_cast<quint8>(idData[2]);

        // Формируем строки для лога и интерфейса
        QString displayMidStr = QString("0x%1h").arg(QString::number(mfgId, 16).rightJustified(2, '0').toUpper());
        QString logCidStr;
        QString uiIdStr;

        if (devId > 0x00FF) {
            logCidStr = QString("0x%1h").arg(QString::number(devId, 16).rightJustified(4, '0').toUpper());
            uiIdStr = QString("ID: 0x%1%2")
                          .arg(QString::number(mfgId, 16).rightJustified(2, '0').toUpper())
                          .arg(QString::number(devId, 16).rightJustified(4, '0').toUpper());
        } else {
            logCidStr = QString("0x%1h").arg(QString::number(devId, 16).rightJustified(2, '0').toUpper());
            uiIdStr = QString("ID: 0x%1%2")
                          .arg(QString::number(mfgId, 16).rightJustified(2, '0').toUpper())
                          .arg(QString::number(devId, 16).rightJustified(2, '0').toUpper());
        }

        log(QString("MID: %1 | CID: %2").arg(displayMidStr, logCidStr));

        // Загружаем базу из файла chips.txt (парсер devId уже настроен на quint16)
        QList<FlashInfo> db = loadFlashDatabase();
        bool found = false;
        for (const auto &chip : db) {
            // Прямое сопоставление (теперь поддерживает как 0x5B, так и 0x7E1A)
            if (chip.mfgId == mfgId && chip.devId == devId) {
                log(QString("<font color='#2E7D32'><b>Chip found: %1: %2Mb (%3MB)</b></font>")
                        .arg(chip.name)
                        .arg(((double)chip.size / (1024.0 * 1024.0))*8, 0, 'f', 0)
                        .arg((double)chip.size / (1024.0 * 1024.0), 0, 'g', 4));

                ui->chipInfoLabel->setText(QString("%1").arg(chip.name));

                QString sizeMb = QString::number((double)chip.size / (1024.0 * 1024.0), 'g', 4) + "MB";
                ui->sizeLabel->setText(sizeMb);

                // В UI (метке) выводим красивый ID
                ui->idLabel->setText(uiIdStr);

                bufSize = chip.size;
                detectedChipSize = chip.size; // Сохраняем размер определенного чипа
                found = true;

                // --- Динамическое ограничение ComboBox по размеру чипа (оставлено без изменений) ---
                ui->romComboBox->blockSignals(true);

                const QList<quint32> standardSizes = {
                    0x00008000, 0x00080000, 0x00100000, 0x00200000,
                    0x00400000, 0x00800000, 0x01000000, 0x02000000
                };

                int allowedCount = 0;
                for (int i = 0; i < standardSizes.size(); ++i) {
                    if (standardSizes[i] <= bufSize) {
                        allowedCount = i + 1;
                    }
                }

                while (ui->romComboBox->count() > allowedCount) {
                    ui->romComboBox->removeItem(ui->romComboBox->count() - 1);
                }

                ui->romComboBox->setCurrentIndex(allowedCount - 1);
                ui->romComboBox->blockSignals(false);
                // ---------------------------------------------------------------------------------

                break;
            }
        }

        if (!found) {
            log(QString("<font color='#9E7B05'><b>Unknown Chip ID. Try reading again or it is not in chips.txt.</b></font>"));
            ui->chipInfoLabel->setText("Chip: Unknown");
            ui->sizeLabel->setText("N/A");
            ui->idLabel->setText(uiIdStr);

            bufSize = 0x00008000; // по умолчанию 32 KB
            ui->romComboBox->setCurrentIndex(0);
        } else {
            ui->detectButton->setEnabled(false);
        }

        // Отправляем реальный размер на микроконтроллер
        sendAutoRomSize();
        log("ROM size synchronized with MCU.");

    } else {
        log("Failed to receive Chip ID bytes.");
    }
}

void MainWindow::on_aboutButton_clicked() {
    // Вёрстка блока заголовка с помощью HTML-таблицы:
    // Аватар слева, название и версия справа
    QString aboutText = QString(
        "<table>"
        "<tr>"
        "  <!-- Левая колонка: аватар -->"
        "  <td style='padding-right: 15px; vertical-align: middle;'>"
        "    <img src=':/icons/avatar.png' width='80' height='80' "
        "         style='border-radius: 40px; border: 2px solid #333;'>"
        "  </td>"

        "  <!-- Правая колонка: Название, версия и авторы -->"
        "  <td style='vertical-align: middle;'>"
        "    <h3 style='margin: 0; padding: 0;'>16BitFlash Programmer</h3>"
        "    <p style='margin: 4px 0;'><b>Версия:</b> 4.3.0</p>"
        "    <p style='margin: 0; font-size: 12px; color: #444;'>"
        "      <b>Разработчик:</b> Pushkash<br>"
        "      <b>AI-ассистент:</b> Gemini"
        "    </p>"
        "  </td>"
        "</tr>"
        "</table>"

        "<hr>"

        // Описание и назначение
        "<p><b>Описание:</b><br>"
        "Программа предназначена для чтения, записи, проверки и стирания "
        "16-битных микросхем Flash-памяти (серии MX29LV..., AM29LV..., EN29LV..., S29GL... и др.)<br>"
        "с помощью кастомного программатора на базе Raspberry Pi Pico или Arduino.</p>"

        "<p><b>Назначение:</b><br>"
        "Разработано специально для работы с картриджами и ROM-файлами "
        "для игровой консоли SEGA Mega Drive / Genesis.</p>"

        "<hr>"

        // Примечание
        "<p style='font-size: 11px; color: #555;'>"
        "<b>Примечание:</b> Для правильной работы устройства и связи с ПК "
        "в микроконтроллер должен быть загружен специальный скетч (Firmware).</p>"
        );

    QMessageBox aboutDlg(this);
    aboutDlg.setWindowTitle(tr("О программе"));
    aboutDlg.setIconPixmap(QPixmap(":/icons/icon.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    aboutDlg.setText(aboutText);
    aboutDlg.exec();
}
