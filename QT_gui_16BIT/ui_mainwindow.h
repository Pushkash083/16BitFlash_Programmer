/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGroupBox *chipType;
    QRadioButton *KB32Button;
    QRadioButton *MB1Button;
    QRadioButton *MB2Button;
    QRadioButton *MB4Button;
    QRadioButton *MB8Button;
    QRadioButton *KB512Button;
    QRadioButton *MB3Button;
    QPushButton *openFileButton;
    QPushButton *saveFileButton;
    QPushButton *readChipButton;
    QPushButton *writeChipButton;
    QPushButton *verifyChipButton;
    QTextBrowser *textBrowser;
    QProgressBar *progressBar;
    QGroupBox *serialPort;
    QListWidget *portList;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *eraseChipButton;
    QTableView *tableView;
    QPushButton *showButton;
    QLCDNumber *lcdNumber;
    QRadioButton *autoRomButton;
    QPushButton *aboutButton;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(798, 431);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMaximumSize(QSize(16777215, 16777215));
        MainWindow->setContextMenuPolicy(Qt::NoContextMenu);
        MainWindow->setAnimated(false);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        chipType = new QGroupBox(centralWidget);
        chipType->setObjectName("chipType");
        chipType->setGeometry(QRect(270, 10, 91, 161));
        chipType->setFlat(false);
        chipType->setCheckable(false);
        KB32Button = new QRadioButton(chipType);
        KB32Button->setObjectName("KB32Button");
        KB32Button->setEnabled(false);
        KB32Button->setGeometry(QRect(10, 10, 81, 22));
        KB32Button->setCheckable(true);
        KB32Button->setChecked(false);
        KB32Button->setAutoRepeat(false);
        MB1Button = new QRadioButton(chipType);
        MB1Button->setObjectName("MB1Button");
        MB1Button->setEnabled(false);
        MB1Button->setGeometry(QRect(10, 50, 61, 22));
        MB2Button = new QRadioButton(chipType);
        MB2Button->setObjectName("MB2Button");
        MB2Button->setEnabled(false);
        MB2Button->setGeometry(QRect(10, 70, 61, 22));
        MB4Button = new QRadioButton(chipType);
        MB4Button->setObjectName("MB4Button");
        MB4Button->setEnabled(false);
        MB4Button->setGeometry(QRect(10, 110, 61, 22));
        MB8Button = new QRadioButton(chipType);
        MB8Button->setObjectName("MB8Button");
        MB8Button->setEnabled(false);
        MB8Button->setGeometry(QRect(10, 130, 61, 22));
        KB512Button = new QRadioButton(chipType);
        KB512Button->setObjectName("KB512Button");
        KB512Button->setEnabled(false);
        KB512Button->setGeometry(QRect(10, 30, 61, 22));
        MB3Button = new QRadioButton(chipType);
        MB3Button->setObjectName("MB3Button");
        MB3Button->setEnabled(false);
        MB3Button->setGeometry(QRect(10, 90, 61, 22));
        openFileButton = new QPushButton(centralWidget);
        openFileButton->setObjectName("openFileButton");
        openFileButton->setEnabled(false);
        openFileButton->setGeometry(QRect(10, 180, 111, 27));
        saveFileButton = new QPushButton(centralWidget);
        saveFileButton->setObjectName("saveFileButton");
        saveFileButton->setEnabled(false);
        saveFileButton->setGeometry(QRect(130, 180, 111, 27));
        readChipButton = new QPushButton(centralWidget);
        readChipButton->setObjectName("readChipButton");
        readChipButton->setEnabled(false);
        readChipButton->setGeometry(QRect(100, 210, 81, 27));
        readChipButton->setCheckable(false);
        writeChipButton = new QPushButton(centralWidget);
        writeChipButton->setObjectName("writeChipButton");
        writeChipButton->setEnabled(false);
        writeChipButton->setGeometry(QRect(190, 210, 81, 27));
        verifyChipButton = new QPushButton(centralWidget);
        verifyChipButton->setObjectName("verifyChipButton");
        verifyChipButton->setEnabled(false);
        verifyChipButton->setGeometry(QRect(280, 210, 81, 27));
        textBrowser = new QTextBrowser(centralWidget);
        textBrowser->setObjectName("textBrowser");
        textBrowser->setGeometry(QRect(10, 270, 351, 151));
        QFont font;
        font.setFamilies({QString::fromUtf8("Monospace")});
        textBrowser->setFont(font);
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName("progressBar");
        progressBar->setEnabled(true);
        progressBar->setGeometry(QRect(10, 240, 351, 23));
        progressBar->setValue(0);
        progressBar->setTextVisible(true);
        serialPort = new QGroupBox(centralWidget);
        serialPort->setObjectName("serialPort");
        serialPort->setGeometry(QRect(10, 10, 91, 161));
        portList = new QListWidget(serialPort);
        portList->setObjectName("portList");
        portList->setGeometry(QRect(10, 20, 71, 51));
        portList->setFrameShape(QFrame::NoFrame);
        portList->setFrameShadow(QFrame::Plain);
        connectButton = new QPushButton(serialPort);
        connectButton->setObjectName("connectButton");
        connectButton->setEnabled(false);
        connectButton->setGeometry(QRect(10, 80, 71, 27));
        disconnectButton = new QPushButton(serialPort);
        disconnectButton->setObjectName("disconnectButton");
        disconnectButton->setEnabled(false);
        disconnectButton->setGeometry(QRect(10, 120, 71, 27));
        eraseChipButton = new QPushButton(centralWidget);
        eraseChipButton->setObjectName("eraseChipButton");
        eraseChipButton->setEnabled(false);
        eraseChipButton->setGeometry(QRect(10, 210, 81, 27));
        eraseChipButton->setCheckable(true);
        tableView = new QTableView(centralWidget);
        tableView->setObjectName("tableView");
        tableView->setGeometry(QRect(380, 10, 401, 411));
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Courier New")});
        font1.setPointSize(14);
        font1.setBold(false);
        tableView->setFont(font1);
        tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableView->setTabKeyNavigation(false);
        tableView->setProperty("showDropIndicator", QVariant(false));
        tableView->setDragDropOverwriteMode(false);
        tableView->setAlternatingRowColors(false);
        tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tableView->setWordWrap(false);
        tableView->verticalHeader()->setVisible(false);
        showButton = new QPushButton(centralWidget);
        showButton->setObjectName("showButton");
        showButton->setEnabled(false);
        showButton->setGeometry(QRect(250, 180, 111, 27));
        showButton->setCheckable(true);
        lcdNumber = new QLCDNumber(centralWidget);
        lcdNumber->setObjectName("lcdNumber");
        lcdNumber->setGeometry(QRect(110, 20, 151, 51));
        lcdNumber->setToolTipDuration(-1);
        lcdNumber->setLineWidth(1);
        lcdNumber->setDigitCount(8);
        autoRomButton = new QRadioButton(centralWidget);
        autoRomButton->setObjectName("autoRomButton");
        autoRomButton->setEnabled(false);
        autoRomButton->setGeometry(QRect(130, 70, 111, 22));
        aboutButton = new QPushButton(centralWidget);
        aboutButton->setObjectName("aboutButton");
        aboutButton->setGeometry(QRect(340, 0, 21, 16));
        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "16BitFlash Programmer", nullptr));
        chipType->setTitle(QCoreApplication::translate("MainWindow", "         ROM", nullptr));
        KB32Button->setText(QCoreApplication::translate("MainWindow", "TEST32KB", nullptr));
        MB1Button->setText(QCoreApplication::translate("MainWindow", "    1MB", nullptr));
        MB2Button->setText(QCoreApplication::translate("MainWindow", "    2MB", nullptr));
        MB4Button->setText(QCoreApplication::translate("MainWindow", "    4MB", nullptr));
        MB8Button->setText(QCoreApplication::translate("MainWindow", "    8MB", nullptr));
        KB512Button->setText(QCoreApplication::translate("MainWindow", " 512KB", nullptr));
        MB3Button->setText(QCoreApplication::translate("MainWindow", "    3MB", nullptr));
        openFileButton->setText(QCoreApplication::translate("MainWindow", "Open file", nullptr));
        saveFileButton->setText(QCoreApplication::translate("MainWindow", "Save file", nullptr));
        readChipButton->setText(QCoreApplication::translate("MainWindow", "Read", nullptr));
        writeChipButton->setText(QCoreApplication::translate("MainWindow", "Write", nullptr));
        verifyChipButton->setText(QCoreApplication::translate("MainWindow", "Verify", nullptr));
        serialPort->setTitle(QCoreApplication::translate("MainWindow", "Serial port", nullptr));
        connectButton->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        disconnectButton->setText(QCoreApplication::translate("MainWindow", "Disconnect", nullptr));
        eraseChipButton->setText(QCoreApplication::translate("MainWindow", "Erase", nullptr));
        showButton->setText(QCoreApplication::translate("MainWindow", "Show buffer", nullptr));
        autoRomButton->setText(QCoreApplication::translate("MainWindow", "Auto ROM Select", nullptr));
        aboutButton->setText(QCoreApplication::translate("MainWindow", "---", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
