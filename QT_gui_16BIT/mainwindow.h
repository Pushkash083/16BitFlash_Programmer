#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QListWidgetItem>
#include <QByteArray>
#include <QElapsedTimer> // <-- ОБЯЗАТЕЛЬНО ДЛЯ ТАЙМЕРА
#include <QAbstractTableModel>
#include <QColor>

#include <QAbstractTableModel>
#include <QColor>




QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


enum CheckStatus {
    NO_ERR = 0,
    WRITABLE = 1,
    NOT_WRITABLE = 2
};

class HexModel : public QAbstractTableModel {
    Q_OBJECT
    const QByteArray *buffer = nullptr;
    const QByteArray *checkData = nullptr;

public:
    explicit HexModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}

    void updateData(const QByteArray *buf, const QByteArray *check) {
        beginResetModel();
        buffer = buf;
        checkData = check;
        endResetModel();
    }

    int rowCount(const QModelIndex &) const override {
        return buffer ? (buffer->size() + 15) / 16 : 0; // 16 байт в строке
    }

    int columnCount(const QModelIndex &) const override {
        return 16; // Ровно 16 колонок байт (00 .. 0F)
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!buffer || !index.isValid()) return QVariant();

        int idx = index.row() * 16 + index.column();
        if (idx >= buffer->size()) return QVariant();

        // Отображение байта в HEX
        if (role == Qt::DisplayRole) {
            return QString("%1").arg(static_cast<uint8_t>(buffer->at(idx)), 2, 16, QChar('0')).toUpper();
        }

        // Подсветка ошибок
        if (role == Qt::ForegroundRole && checkData && idx < checkData->size()) {
            uint8_t val = static_cast<uint8_t>(checkData->at(idx));
            if (val == CheckStatus::NOT_WRITABLE) return QColor(255, 0, 0);   // Красный
            if (val == CheckStatus::WRITABLE)     return QColor(158, 123, 5); // тёмно-жёлтый
        }

        return QVariant();
    }

    // Заголовки таблицы: адреса (00000000) и колонки (00 .. 0F)
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole) return QVariant();

        if (orientation == Qt::Vertical) {
            return QString("%1").arg(section * 16, 8, 16, QChar('0')).toUpper();
        } else {
            return QString("%1").arg(section, 1, 16, QChar('0')).toUpper();
        }
    }
};




class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void chipUpdated();
    void chipReaded();
    void bufferUpdated();

private slots:
    void updateProcessTimer();
    void handleSerialError(QSerialPort::SerialPortError error); // <--- Добавить этот слот
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_portList_itemClicked(QListWidgetItem *item);

    void on_writeChipButton_clicked();
    void on_readChipButton_clicked();
    void on_verifyChipButton_clicked();

    void on_openFileButton_clicked();
    void on_saveFileButton_clicked();
    void on_showButton_toggled(bool checked);

    void on_KB32Button_clicked();
    void on_KB512Button_clicked();
    void on_MB1Button_clicked();
    void on_MB2Button_clicked();
    void on_MB3Button_clicked();
    void on_MB4Button_clicked();
    void on_MB8Button_clicked();
    void on_autoRomButton_clicked();
    void sendAutoRomSize();

    void on_eraseChipButton_toggled(bool checked);
    void on_eraseChipButton_2_clicked();
    void on_progressBar_valueChanged(int value);

    void reload_ports();
    void readData();
    void checkClear();
    void verifyData();
    void showBuf();
    void resizeBuffers();
    void finishErase(); // <--- Слот завершения 30-секундного стирания

    void on_aboutButton_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serialPort;
    QTimer updatePortsTimer;
    QTimer processTimer;
    HexModel *hexModel;

    QElapsedTimer eraseTimer;
    QMetaObject::Connection updatePortsConnection;
    QMetaObject::Connection serialDataConnection;
    QMetaObject::Connection checkClearConnection;
    QMetaObject::Connection verifyDataConnection;
    QMetaObject::Connection updateBufConnection;

    QByteArray bufWork;
    QByteArray bufCheck;
    uint32_t bufSize = 0;

    bool chipSelected = false;
    bool bufferClear = true;

    void preventSleep();
    void allowSleep();
    void log(const QString &str);
    void chipSelectSetEnabled(bool state);
    void chipSelectSetEnDes(bool state);
    void openSerialPort(const QString &path);
    void closeSerialPort();
    void updateButtons(bool actions, bool buffer);
    void writeData(const QByteArray &data);
    void writeChip();
    void readChip();
};

#endif // MAINWINDOW_H
