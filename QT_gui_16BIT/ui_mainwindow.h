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
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGroupBox *chipType;
    QComboBox *romComboBox;
    QTextBrowser *textBrowser;
    QProgressBar *progressBar;
    QGroupBox *serialPort;
    QListWidget *portList;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QTableView *tableView;
    QLCDNumber *lcdNumber;
    QRadioButton *autoRomButton;
    QPushButton *aboutButton;
    QGroupBox *chipGroupBox;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *openFileButton;
    QPushButton *saveFileButton;
    QPushButton *detectButton;
    QPushButton *showButton;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *eraseChipButton;
    QPushButton *readChipButton;
    QPushButton *writeChipButton;
    QPushButton *verifyChipButton;
    QWidget *verticalLayoutWidget_3;
    QVBoxLayout *verticalLayout_4;
    QLabel *chipInfoLabel;
    QHBoxLayout *horizontalLayout;
    QLabel *idLabel;
    QSpacerItem *horizontalSpacer;
    QLabel *sizeLabel;

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
        chipType->setGeometry(QRect(270, 20, 91, 51));
        chipType->setAlignment(Qt::AlignCenter);
        chipType->setFlat(false);
        chipType->setCheckable(false);
        romComboBox = new QComboBox(chipType);
        romComboBox->addItem(QString());
        romComboBox->setObjectName("romComboBox");
        romComboBox->setGeometry(QRect(10, 20, 71, 22));
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
        verticalLayoutWidget = new QWidget(serialPort);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(10, 79, 71, 81));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        connectButton = new QPushButton(verticalLayoutWidget);
        connectButton->setObjectName("connectButton");
        connectButton->setEnabled(false);

        verticalLayout->addWidget(connectButton);

        disconnectButton = new QPushButton(verticalLayoutWidget);
        disconnectButton->setObjectName("disconnectButton");
        disconnectButton->setEnabled(false);

        verticalLayout->addWidget(disconnectButton);

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
        chipGroupBox = new QGroupBox(centralWidget);
        chipGroupBox->setObjectName("chipGroupBox");
        chipGroupBox->setGeometry(QRect(110, 90, 151, 81));
        chipGroupBox->setAlignment(Qt::AlignCenter);
        verticalLayoutWidget_2 = new QWidget(centralWidget);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(10, 180, 351, 51));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        openFileButton = new QPushButton(verticalLayoutWidget_2);
        openFileButton->setObjectName("openFileButton");
        openFileButton->setEnabled(false);

        horizontalLayout_2->addWidget(openFileButton);

        saveFileButton = new QPushButton(verticalLayoutWidget_2);
        saveFileButton->setObjectName("saveFileButton");
        saveFileButton->setEnabled(false);

        horizontalLayout_2->addWidget(saveFileButton);

        detectButton = new QPushButton(verticalLayoutWidget_2);
        detectButton->setObjectName("detectButton");
        detectButton->setEnabled(false);
        detectButton->setCheckable(false);

        horizontalLayout_2->addWidget(detectButton);

        showButton = new QPushButton(verticalLayoutWidget_2);
        showButton->setObjectName("showButton");
        showButton->setEnabled(false);
        showButton->setCheckable(true);

        horizontalLayout_2->addWidget(showButton);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        eraseChipButton = new QPushButton(verticalLayoutWidget_2);
        eraseChipButton->setObjectName("eraseChipButton");
        eraseChipButton->setEnabled(false);
        eraseChipButton->setCheckable(true);

        horizontalLayout_3->addWidget(eraseChipButton);

        readChipButton = new QPushButton(verticalLayoutWidget_2);
        readChipButton->setObjectName("readChipButton");
        readChipButton->setEnabled(false);
        readChipButton->setCheckable(false);

        horizontalLayout_3->addWidget(readChipButton);

        writeChipButton = new QPushButton(verticalLayoutWidget_2);
        writeChipButton->setObjectName("writeChipButton");
        writeChipButton->setEnabled(false);

        horizontalLayout_3->addWidget(writeChipButton);

        verifyChipButton = new QPushButton(verticalLayoutWidget_2);
        verifyChipButton->setObjectName("verifyChipButton");
        verifyChipButton->setEnabled(false);

        horizontalLayout_3->addWidget(verifyChipButton);


        verticalLayout_2->addLayout(horizontalLayout_3);

        verticalLayoutWidget_3 = new QWidget(centralWidget);
        verticalLayoutWidget_3->setObjectName("verticalLayoutWidget_3");
        verticalLayoutWidget_3->setGeometry(QRect(120, 109, 131, 51));
        verticalLayout_4 = new QVBoxLayout(verticalLayoutWidget_3);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        chipInfoLabel = new QLabel(verticalLayoutWidget_3);
        chipInfoLabel->setObjectName("chipInfoLabel");
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Times New Roman")});
        font2.setPointSize(12);
        chipInfoLabel->setFont(font2);
        chipInfoLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_4->addWidget(chipInfoLabel);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        idLabel = new QLabel(verticalLayoutWidget_3);
        idLabel->setObjectName("idLabel");
        QFont font3;
        font3.setPointSize(10);
        idLabel->setFont(font3);
        idLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout->addWidget(idLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        sizeLabel = new QLabel(verticalLayoutWidget_3);
        sizeLabel->setObjectName("sizeLabel");
        QFont font4;
        font4.setPointSize(11);
        sizeLabel->setFont(font4);
        sizeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(sizeLabel);


        verticalLayout_4->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "16BitFlash Programmer", nullptr));
        chipType->setTitle(QCoreApplication::translate("MainWindow", "ROM", nullptr));
        romComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "TEST32KB", nullptr));

        serialPort->setTitle(QCoreApplication::translate("MainWindow", "Serial port", nullptr));
        connectButton->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        disconnectButton->setText(QCoreApplication::translate("MainWindow", "Disconnect", nullptr));
        autoRomButton->setText(QCoreApplication::translate("MainWindow", "Auto ROM Select", nullptr));
        aboutButton->setText(QCoreApplication::translate("MainWindow", "---", nullptr));
        chipGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Chip Information", nullptr));
        openFileButton->setText(QCoreApplication::translate("MainWindow", "Open file", nullptr));
        saveFileButton->setText(QCoreApplication::translate("MainWindow", "Save file", nullptr));
        detectButton->setText(QCoreApplication::translate("MainWindow", "Chip ID", nullptr));
        showButton->setText(QCoreApplication::translate("MainWindow", "Show buffer", nullptr));
        eraseChipButton->setText(QCoreApplication::translate("MainWindow", "Erase", nullptr));
        readChipButton->setText(QCoreApplication::translate("MainWindow", "Read", nullptr));
        writeChipButton->setText(QCoreApplication::translate("MainWindow", "Write", nullptr));
        verifyChipButton->setText(QCoreApplication::translate("MainWindow", "Verify", nullptr));
        chipInfoLabel->setText(QString());
        idLabel->setText(QString());
        sizeLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
