#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QThread>
#include <QUdpSocket>
#include <QPushButton>
#include <QTimer>

#include "preferences.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef Q_OS_LINUX
    setWindowTitle(tr("EQA_test1-linux Ver: %1.%2").arg(EQA_test1_GENERAL_VERSION).arg(EQA_test1_VERSION));
#endif
#ifdef Q_OS_WIN
    setWindowTitle(tr("EQA_test1-windows Ver: %1.%2").arg(EQA_test1_GENERAL_VERSION).arg(EQA_test1_VERSION));
#endif
    QPoint  pos(WND_X,WND_Y);
    this->move(pos);
    this->resize(QSize(WND_WIDTH,WND_HIGH));
    //
    QTreeWidgetItem *hitm1 = ui->treeWidget->headerItem();
    hitm1->setText(col_group, "Group/Function/Info");
    hitm1->setText(col_action, "Action/Info");
    hitm1->setText(col_info, "Info");
    ui->treeWidget->setColumnWidth(col_group,150);
    ui->treeWidget->setColumnWidth(col_action,150);
    //
    treeUDP_pkt_cnt = new QTreeWidgetItem();
    treeUDP_pkt_cnt->setText(col_group,tr("Packet count:"));
    ui->treeWidget->addTopLevelItem(treeUDP_pkt_cnt);
    treeUDP_switch = new QTreeWidgetItem();
    treeUDP_switch->setText(col_group,tr("Switch:"));
    ui->treeWidget->addTopLevelItem(treeUDP_switch);
    treeUDP_dev_pkt_cnt = new QTreeWidgetItem();
    treeUDP_dev_pkt_cnt->setText(col_group,tr("Dev pkt cnt:"));
    ui->treeWidget->addTopLevelItem(treeUDP_dev_pkt_cnt);
    treeHDD_pass_cnt = new QTreeWidgetItem();
    treeHDD_pass_cnt->setText(col_group,tr("Pass count:"));
    ui->treeWidget->addTopLevelItem(treeHDD_pass_cnt);
    //
    treeUDP_error = new QTreeWidgetItem();
    treeUDP_error->setText(col_group,tr("Error:"));
    ui->treeWidget->addTopLevelItem(treeUDP_error);
    //
    QTreeWidgetItem *treeSense = new QTreeWidgetItem();
    treeSense->setText(col_group,tr("Device sense"));
    ui->treeWidget->addTopLevelItem(treeSense);
    QPushButton *btnSense = new QPushButton(tr("Ask sense"));
    connect(btnSense, SIGNAL(clicked()), this, SLOT(slotAskSense()));
    ui->treeWidget->setItemWidget(treeSense,col_action,btnSense);
    //
    QTreeWidgetItem *treePower = new QTreeWidgetItem();
    treePower->setText(col_group,tr("Power"));
    ui->treeWidget->addTopLevelItem(treePower);
    QPushButton *btnPowerOn = new QPushButton(tr("On"));
    connect(btnPowerOn, &QPushButton::clicked, this, &MainWindow::slotPowerOn);
    ui->treeWidget->setItemWidget(treePower, col_action, btnPowerOn);
    QPushButton *btnPowerOff = new QPushButton(tr("Off"));
    connect(btnPowerOff, &QPushButton::clicked, this, &MainWindow::slotPowerOff);
    ui->treeWidget->setItemWidget(treePower, col_info, btnPowerOff);
    //
    QTreeWidgetItem *treeHDDID = new QTreeWidgetItem();
    treeHDDID->setText(col_group,tr("ATA cmd"));
    ui->treeWidget->addTopLevelItem(treeHDDID);
    QPushButton *btnHDDID = new QPushButton(tr("Send HDD ID cmd"));
    connect(btnHDDID, SIGNAL(clicked()), this, SLOT(slotHDDID()));
    ui->treeWidget->setItemWidget(treeHDDID,col_action,btnHDDID);
    //
    QTreeWidgetItem *treeSectBufRd = new QTreeWidgetItem();
    treeSectBufRd->setText(col_group,tr("ATA cmd"));
    ui->treeWidget->addTopLevelItem(treeSectBufRd);
    QPushButton *btnSectBufRd = new QPushButton(tr("Read Buf Sect"));
    connect(btnSectBufRd, &QPushButton::clicked, this, &MainWindow::slotSectBufRd);
    ui->treeWidget->setItemWidget(treeSectBufRd, col_action, btnSectBufRd);
    //
    QTreeWidgetItem *treeSectBufWr = new QTreeWidgetItem();
    treeSectBufWr->setText(col_group,tr("ATA cmd"));
    ui->treeWidget->addTopLevelItem(treeSectBufWr);
    QPushButton *btnSectBufWr = new QPushButton(tr("Write Buf Sect"));
    connect(btnSectBufWr, &QPushButton::clicked, this, &MainWindow::slotSectBufWr);
    ui->treeWidget->setItemWidget(treeSectBufWr, col_action, btnSectBufWr);
    //
    QTreeWidgetItem *treeSect2WrBuf = new QTreeWidgetItem();
    treeSect2WrBuf->setText(col_group,tr("Sect to Buf"));
    ui->treeWidget->addTopLevelItem(treeSect2WrBuf);
    QPushButton *btnSect2WrBuf = new QPushButton(tr("Write Buffer 1"));
    connect(btnSect2WrBuf, &QPushButton::clicked, this, &MainWindow::slotSect2WrBuf);
    ui->treeWidget->setItemWidget(treeSect2WrBuf, col_action, btnSect2WrBuf);
    //
    QTreeWidgetItem *treeSect2WrBufTest = new QTreeWidgetItem();
    treeSect2WrBufTest->setText(col_group,tr("Sect to Buf"));
    ui->treeWidget->addTopLevelItem(treeSect2WrBufTest);
    QPushButton *btnSect2WrBufTest = new QPushButton(tr("Write Buffer 1 Test"));
    connect(btnSect2WrBufTest, &QPushButton::clicked, this, &MainWindow::slotSect2WrBufTest);
    ui->treeWidget->setItemWidget(treeSect2WrBufTest, col_action, btnSect2WrBufTest);
    //
    QTreeWidgetItem *treeSectBufRWtest = new QTreeWidgetItem();
    treeSectBufRWtest->setText(col_group,tr("RW HDD cycle"));
    ui->treeWidget->addTopLevelItem(treeSectBufRWtest);
    QPushButton *btnSectRdWrBufTest = new QPushButton(tr("Read-Write HDD Buffer"));
    connect(btnSectRdWrBufTest, &QPushButton::clicked, this, &MainWindow::slotSectRdWrBufTest);
    ui->treeWidget->setItemWidget(treeSectBufRWtest, col_action, btnSectRdWrBufTest);
    QPushButton *btnSectStop = new QPushButton(tr("Stop"));
    connect(btnSectStop, &QPushButton::clicked, this, &MainWindow::slotSectStop);
    ui->treeWidget->setItemWidget(treeSectBufRWtest, col_info, btnSectStop);
    //
    treeCMDCNT = new QTreeWidgetItem();
    treeCMDCNT->setText(col_group,tr("CMDs:"));
    ui->treeWidget->addTopLevelItem(treeCMDCNT);
    tree_struct_hdd_id = new QTreeWidgetItem();
    tree_struct_hdd_id->setText(col_group,tr("HDD ID"));
    ui->treeWidget->addTopLevelItem(tree_struct_hdd_id);
    tree_struct_sense = new QTreeWidgetItem();
    tree_struct_sense->setText(col_group,tr("Sense"));
    ui->treeWidget->addTopLevelItem(tree_struct_sense);
    //
//    QByteArray ba;
//    ba.fill(0,sector_s);
//    ui->hexEdit->setData(ba);
//    fill_ATA_ID(ba);
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged, this, &MainWindow::slotItemChanged);
    //
    worker = new QThread(this);
    eqa = new QAtaViaEthernet();
    eqa->moveToThread(worker);
    connect(this, &MainWindow::askSense, eqa, &QAtaViaEthernet::sendDevSense);
    connect(this, &MainWindow::askPower, eqa, &QAtaViaEthernet::sendDevPower);
    connect(this, &MainWindow::sendHddId, eqa, &QAtaViaEthernet::sendIDcmd);
    connect(this, &MainWindow::sendReadSectorBuffer, eqa, &QAtaViaEthernet::sendReadSectorBuffer);
    connect(this, &MainWindow::sendWriteSectorBuffer, eqa, &QAtaViaEthernet::sendWriteSectorBuffer);
    connect(this, &MainWindow::sendSectorToBuffer1, eqa, &QAtaViaEthernet::sendSectorToBuffer1);
    connect(this, &MainWindow::sendSectorToBuffer1Test, eqa, &QAtaViaEthernet::sendSectorToBuffer1test);
    connect(this, &MainWindow::startHDDsectorBufferTest, eqa, &QAtaViaEthernet::startHDDsectorBufferTest);
    connect(this, &MainWindow::stopHDDsectorBufferTest, eqa, &QAtaViaEthernet::stopHDDsectorBufferTest);
    connect(eqa, &QAtaViaEthernet::error, this, &MainWindow::slotError);
    connect(eqa, &QAtaViaEthernet::cmdData, this, &MainWindow::slotCmdData);
    connect(eqa, &QAtaViaEthernet::passCount, this, &MainWindow::slotPassCount);
    worker->start();
}

MainWindow::~MainWindow()
{
    worker->quit();
    if(!worker->wait(10000)) qDebug()<<"Mainwindow worker thread problem.";
    delete ui;
}

void MainWindow::slotAskSense()
{
    clear_all();
    emit askSense();
}

void MainWindow::slotPowerOn()
{
    emit askPower(true);
}

void MainWindow::slotPowerOff()
{
    emit askPower(false);
}

void MainWindow::slotHDDID()
{
    clear_all();
    emit sendHddId();
}

void MainWindow::slotSectBufRd()
{
    clear_all();
    emit sendReadSectorBuffer();
}

void MainWindow::slotSectBufWr()
{
    clear_all();
    emit sendWriteSectorBuffer();
}

void MainWindow::slotSect2WrBuf()
{
    QByteArray ba;
    ba.fill(1,sector_s);
    emit sendSectorToBuffer1(ba);
}

void MainWindow::slotSect2WrBufTest()
{
    clear_all();
    QByteArray ba;
    ba.fill(0,sector_s);
    for (int i = 0; i < sector_s; ++i) {
        ba[i] = static_cast<quint8>(i);
    }
    emit sendSectorToBuffer1Test(ba);
}

void MainWindow::slotSectRdWrBufTest()
{
    clear_all();
    // block other, activate stop
    // emit cycle start
    emit startHDDsectorBufferTest();
}

void MainWindow::slotPassCount(quint32 passCnt)
{
    treeHDD_pass_cnt->setText(col_action, QString::number(passCnt));
}

void MainWindow::slotSectStop()
{
    emit stopHDDsectorBufferTest();
}

void MainWindow::slotError(const QString &e)
{
    treeUDP_error->setText(col_action, e);
}

void MainWindow::slotCmdData(int cmd_type, const QByteArray &ba)
{
//    qDebug()<<"ba.size:"<<ba.size();
    switch (cmd_type) {
    case atacmd_sense:
        fill_Sense(ba);
        ui->hexEdit->setData(ba);
        break;
    case atacmd_id:
        fill_ATA_ID(ba);
        ui->hexEdit->setData(ba);
        break;
    case atacmd_rd_buff:
        ui->hexEdit->setData(ba);
        break;
    case atacmd_wr_buff:
        break;
    case atacmd_wr_buff_test:
        ui->hexEdit->setData(ba);
        break;
    default:
        break;
    }
}

void MainWindow::slotItemChanged(QTreeWidgetItem *c, QTreeWidgetItem *p)
{
    Q_UNUSED(p);
    //qDebug()<<"item data:"<<c->data(0,Qt::ToolTipRole).toString();
    bool ok;
    quint32 adr = c->data(0,Qt::ToolTipRole).toString().left(8).toInt(&ok,16);
    quint32 sz = c->data(0,Qt::ToolTipRole).toString().right(8).toInt(&ok,16);
    ui->hexEdit->markBytes(adr, sz);
}

void MainWindow::clear_all()
{
    clear_Sense();
    clear_ATA_ID();
    treeUDP_error->setText(col_action, "");
}

void MainWindow::clear_ATA_ID()
{
    foreach(auto i, tree_struct_hdd_id->takeChildren()) delete i;
}

void MainWindow::fill_ATA_ID(const QByteArray &hddID)
{
    QTreeWidgetItem *itm;

    const AtaIdentifyDevice *idData = reinterpret_cast<const AtaIdentifyDevice*>(hddID.data());

    itm = new QTreeWidgetItem(tree_struct_hdd_id);
    itm->setText(col_group,SARRAY);
    itm->setText(1,"serial_no");
    QByteArray sn(reinterpret_cast<const char*>(idData->serial_no),ATA_ID_SERIAL_STR_SIZE);
    itm->setText(2,QString(sn));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(10*2,8,16,QLatin1Char('0')).arg(ATA_ID_SERIAL_STR_SIZE,8,16,QLatin1Char('0')));
    /// WORD 020     Retired
    itm = new QTreeWidgetItem(tree_struct_hdd_id);
    itm->setText(col_group,SWORD);
    itm->setText(1,"word20");
    itm->setText(2,QString("0x%1").arg(idData->word20,8,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(20*2,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    /// WORD 021     (cache size)
    itm = new QTreeWidgetItem(tree_struct_hdd_id);
    itm->setText(col_group,SWORD);
    itm->setText(1,"cache_size");
    itm->setText(2,QString("0x%1").arg(idData->cache_size,8,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(21*2,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    /// WORD 022     Retired
    itm = new QTreeWidgetItem(tree_struct_hdd_id);
    itm->setText(0,SWORD);
    itm->setText(1,"word22");
    itm->setText(2,QString("0x%1").arg(idData->word22,8,16,QLatin1Char('0')));
    itm->setData(0,Qt::ToolTipRole,QString("%1-%2").arg(22*2,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    /// WORD 023-026 Firmware revision (ASCII String)
    itm = new QTreeWidgetItem(tree_struct_hdd_id);
    itm->setText(0,SARRAY);
    itm->setText(1,"fw_rev");
    QByteArray fw(reinterpret_cast<const char*>(idData->fw_rev),ATA_ID_FIRMVARE_STR_SIZE);
    itm->setText(2,QString(fw));
    itm->setData(0,Qt::ToolTipRole,QString("%1-%2").arg(23*2,8,16,QLatin1Char('0')).arg(ATA_ID_FIRMVARE_STR_SIZE,8,16,QLatin1Char('0')));
    /// WORD 069 Additional Supported
    itm = new QTreeWidgetItem(tree_struct_hdd_id);
    itm->setText(0,SWORD);
    itm->setText(1,"additional_supported");
    itm->setText(2,QString("0x%1").arg(idData->additional_supported,8,16,QLatin1Char('0')));
    itm->setData(0,Qt::ToolTipRole,QString("%1-%2").arg(69*2,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    QTreeWidgetItem *itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                             // bit[1:0]
    itm1->setText(1,QString("%1").arg(idData->additional_supported&0x3,2,2,QLatin1Char('0')));
    itm1->setText(2,"bit[1:0] Zoned Capabilities");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit2
    if(BS(idData->additional_supported,2)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[2] All write cache is non-volatile");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit3
    if(BS(idData->additional_supported,3)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[3] Extended Number of User Addressable Sectors is supported");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit4
    if(BS(idData->additional_supported,4)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[4] Device Encrypts All User Data on the device");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit5
    if(BS(idData->additional_supported,5)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[5] Trimmed LBA range(s) returning zeroed data is supported");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit6
    if(BS(idData->additional_supported,6)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[6] 0 = Optional ATA device 28-bit commands supported");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit7
    if(BS(idData->additional_supported,7)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[7] Reserved for IEEE 1667");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit8
    if(BS(idData->additional_supported,8)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[8] DOWNLOAD MICROCODE DMA is supported");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit9
    if(BS(idData->additional_supported,9)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[9] Obsolete");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit10
    if(BS(idData->additional_supported,10)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[10] WRITE BUFFER DMA is supported");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit11
    if(BS(idData->additional_supported,11)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[11] READ BUFFER DMA is supported");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit12
    if(BS(idData->additional_supported,12)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[12] Obsolete");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit13
    if(BS(idData->additional_supported,13)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[13] Long Physical Sector Alignment Error Reporting Control is supported");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit14
    if(BS(idData->additional_supported,14)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[14] Deterministic data in trimmed LBA range(s) is supported");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(0,SBIT);                                              // bit15
    if(BS(idData->additional_supported,15)) { itm1->setText(1,"1"); } else { itm1->setText(1,"0"); }
    itm1->setText(2,"bit[15] Reserved for CFA");
}

void MainWindow::clear_Sense()
{
    foreach(auto i, tree_struct_sense->takeChildren()) delete i;
}

void MainWindow::fill_Sense(const QByteArray &s)
{
    QTreeWidgetItem *itm;
    QTreeWidgetItem *itm1;
    int bcnt = 0;

    const DSSP5atasense *sdata = reinterpret_cast<const DSSP5atasense*>(s.data());

    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SBYTE);
    itm->setText(col_action,"switches");
    itm->setText(2,QString("0x%1").arg(sdata->switches,2,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(1,8,16,QLatin1Char('0')));
    bcnt += 1;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SBYTE);
    itm->setText(col_action,"ata_ver");
    itm->setText(2,QString("0x%1").arg(sdata->ata_ver,2,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(1,8,16,QLatin1Char('0')));
    bcnt += 1;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SBYTE);
    itm->setText(col_action,"ata_res");
    itm->setText(2,QString("0x%1").arg(sdata->ata_res,2,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(1,8,16,QLatin1Char('0')));
    bcnt += 1;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_pkt_num");
    itm->setText(2,QString("0x%1").arg(sdata->ata_pkt_num,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_flags_return");
    itm->setText(2,QString("0x%1").arg(sdata->ata_flags_return,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_timeout_before");
    itm->setText(2,QString("0x%1").arg(sdata->ata_timeout_before,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_timeout_after");
    itm->setText(2,QString("0x%1").arg(sdata->ata_timeout_after,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_p1");
    itm->setText(2,QString("0x%1").arg(sdata->ata_p1,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_p2");
    itm->setText(2,QString("0x%1").arg(sdata->ata_p2,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_p3");
    itm->setText(2,QString("0x%1").arg(sdata->ata_p3,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_p4");
    itm->setText(2,QString("0x%1").arg(sdata->ata_p4,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_p5");
    itm->setText(2,QString("0x%1").arg(sdata->ata_p5,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SWORD);
    itm->setText(col_action,"ata_p6");
    itm->setText(2,QString("0x%1").arg(sdata->ata_p6,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(2,8,16,QLatin1Char('0')));
    bcnt += 2;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SBYTE);
    itm->setText(col_action,"ata_p7");
    itm->setText(2,QString("0x%1").arg(sdata->ata_p7,4,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(1,8,16,QLatin1Char('0')));
    bcnt += 1;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SBYTE);
    itm->setText(col_action,"ata_flags_p7");
    itm->setText(2,QString("0x%1").arg(sdata->ata_flags_p7,2,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(1,8,16,QLatin1Char('0')));
    bcnt += 1;
    itm = new QTreeWidgetItem(tree_struct_sense);
    itm->setText(col_group,SBYTE);
    itm->setText(col_action,"ata_flags_tail");
    itm->setText(2,QString("0x%1").arg(sdata->ata_flags_tail,2,16,QLatin1Char('0')));
    itm->setData(col_group,Qt::ToolTipRole,QString("%1-%2").arg(bcnt,8,16,QLatin1Char('0')).arg(1,8,16,QLatin1Char('0')));
    bcnt += 1;
    itm1 = new QTreeWidgetItem(itm); itm1->setText(col_group,SBIT);                                              // bit0
    if(BS(sdata->ata_flags_tail,0)) { itm1->setText(col_action,"1"); } else { itm1->setText(col_action,"0"); }
    itm1->setText(col_info,"bit[0] Buffer 1 full");
    itm1 = new QTreeWidgetItem(itm); itm1->setText(col_group,SBIT);                                              // bit7
    if(BS(sdata->ata_flags_tail,7)) { itm1->setText(col_action,"1"); } else { itm1->setText(col_action,"0"); }
    itm1->setText(col_info,"always 1");
}

