#include "qataviaethernet.h"

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTimer>

QAtaViaEthernet::QAtaViaEthernet(QObject *parent) : QObject(parent)
{
    wukong_device_address = new QHostAddress("192.168.0.2");
    udpSocketAta = new QUdpSocket(this);
    udpSocketAta->bind(QHostAddress::Any, WUKONG_UDP_ATA);
    connect(udpSocketAta, &QUdpSocket::readyRead, this, &QAtaViaEthernet::readAtaUdp);
    //
    ataPktCount = 1;
    commandInProgress = false;
    //
    timerUdpAta = new QTimer(this);
    connect(timerUdpAta, &QTimer::timeout, this, &QAtaViaEthernet::slotUdpAtaTimeout);
}

void QAtaViaEthernet::sendIDcmd()
{
    DSSP5atacmd atacmd;
    ataPktCount++;
    atacmd.ata_pkt_num = ataPktCount;
    //
    atacmd.ata_flags = 0x0008; // PIO read with wait befor cmd, after cmd, wait drq and get one sector
    //
    atacmd.ata_timeout_before = 9000;
    atacmd.ata_timeout_after  = 10000;
    atacmd.ata_p1 = 0x00;
    atacmd.ata_p2 = 0x00;
    atacmd.ata_p3 = 0x00;
    atacmd.ata_p4 = 0x00;
    atacmd.ata_p5 = 0x00;
    atacmd.ata_p6 = 0x00;
    atacmd.ata_p7 = MAKE_WORD(ATA_IDENTIFY_DEVICE, 0x50); // command (lo byte), ready mask (hi byte)
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atacmd), sizeof(DSSP5atacmd), *wukong_device_address, WUKONG_UDP_ATA);
    commandInProgress = true;
    cmdTypeInProgress = atacmd_id;
    timerUdpAta->start(ETH_TIMEOUT);
}

void QAtaViaEthernet::sendReadSectorBuffer()
{
    DSSP5atacmd atacmd;
    ataPktCount++;
    atacmd.ata_pkt_num = ataPktCount;
    //
    atacmd.ata_flags = 0x0008; // PIO read with wait befor cmd, after cmd, wait drq and get one sector
    //
    atacmd.ata_timeout_before = 9000;
    atacmd.ata_timeout_after  = 10000;
    atacmd.ata_p1 = 0x00;
    atacmd.ata_p2 = 0x00;
    atacmd.ata_p3 = 0x00;
    atacmd.ata_p4 = 0x00;
    atacmd.ata_p5 = 0x00;
    atacmd.ata_p6 = 0x00;
    atacmd.ata_p7 = MAKE_WORD(CMD_READ_BUFFER, 0x50); // command (lo byte), ready mask (hi byte)
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atacmd), sizeof(DSSP5atacmd), *wukong_device_address, WUKONG_UDP_ATA);
    commandInProgress = true;
    cmdTypeInProgress = atacmd_rd_buff;
    timerUdpAta->start(ETH_TIMEOUT);
}

void QAtaViaEthernet::sendWriteSectorBuffer()
{
    DSSP5atacmd atacmd;
    ataPktCount++;
    atacmd.ata_pkt_num = ataPktCount;
    //
    atacmd.ata_flags = 0x0010; // PIO write with wait befor cmd, after cmd, wait drq and put one sector
    //
    atacmd.ata_timeout_before = 9000;
    atacmd.ata_timeout_after  = 10000;
    atacmd.ata_p1 = 0x00;
    atacmd.ata_p2 = 0x00;
    atacmd.ata_p3 = 0x00;
    atacmd.ata_p4 = 0x00;
    atacmd.ata_p5 = 0x00;
    atacmd.ata_p6 = 0x00;
    atacmd.ata_p7 = MAKE_WORD(CMD_WRITE_BUFFER, 0x50); // command (lo byte), ready mask (hi byte)
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atacmd), sizeof(DSSP5atacmd), *wukong_device_address, WUKONG_UDP_ATA);
    commandInProgress = true;
    cmdTypeInProgress = atacmd_wr_buff;
    timerUdpAta->start(ETH_TIMEOUT);
}

void QAtaViaEthernet::sendSectorToBuffer1(const QByteArray &sect)
{
    if(sect.size() != sector_s) {
        emit error("data is not 512 bytes");
        return;
    }
    DSSP5ata_sector_write atadata;
    atadata.ata_cmd_pkt_num = ataPktCount;
    ataPktCount++;
    atadata.ata_pkt_num = ataPktCount;
    atadata.ata_flags = 0x0040; // write data buffer 1, bit6 = 1
    atadata.ata_buf_fill_num = 1; // use buffer 1
    atadata.ata_cmd_sect_num = 1; // first sector of command
    memcpy(atadata.ata_data, sect.data(), sector_s);
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atadata), sizeof(DSSP5ata_sector_write), *wukong_device_address, WUKONG_UDP_ATA);
}

void QAtaViaEthernet::sendSectorToBuffer1test(const QByteArray &sect)
{
    if(sect.size() != sector_s) {
        emit error("data is not 512 bytes");
        return;
    }
    DSSP5ata_sector_write atadata;
    atadata.ata_cmd_pkt_num = ataPktCount;
    ataPktCount++;
    atadata.ata_pkt_num = ataPktCount;
    atadata.ata_flags = 0x0048; // write data buffer 1, bit6 = 1 and bit3 = 1
    atadata.ata_buf_fill_num = 1; // use buffer 1
    atadata.ata_cmd_sect_num = 1; // first sector of command
    memcpy(atadata.ata_data, sect.data(), sector_s);
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atadata), sizeof(DSSP5ata_sector_write), *wukong_device_address, WUKONG_UDP_ATA);
    commandInProgress = true;
    cmdTypeInProgress = atacmd_wr_buff_test;
    timerUdpAta->start(ETH_TIMEOUT);
}

void QAtaViaEthernet::startHDDsectorBufferTest()
{
    testPassCount = 0;
    stopTest = false;
    sendWriteHDDbufferCyc();
}

void QAtaViaEthernet::stopHDDsectorBufferTest()
{
    stopTest = true;
}

void QAtaViaEthernet::sendDevSense()
{
    DSSP5atacmd atacmd;
    commandInProgress = false;
    memset(&atacmd,0,sizeof(DSSP5atacmd));
    atacmd.ata_flags = 0x0001; // ask just sense
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atacmd), sizeof(DSSP5atacmd), *wukong_device_address, WUKONG_UDP_ATA);
    commandInProgress = true;
    cmdTypeInProgress = atacmd_sense;
    timerUdpAta->start(ETH_TIMEOUT);
}

void QAtaViaEthernet::sendDevPower(bool state)
{
    DSSP5atacmd atacmd;
    commandInProgress = false;
    memset(&atacmd,0,sizeof(DSSP5atacmd));
    if(state) {
        atacmd.ata_flags = 0x0005; // sense and power on
    } else {
        atacmd.ata_flags = 0x0003; // sense and power off
    }
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atacmd), sizeof(DSSP5atacmd), *wukong_device_address, WUKONG_UDP_ATA);
    commandInProgress = true;
    cmdTypeInProgress = atacmd_sense;
    timerUdpAta->start(ETH_TIMEOUT);
}

void QAtaViaEthernet::readAtaUdp()
{
    timerUdpAta->stop(); // возможна ли ситуация, когда пакет пришел, но и таймаут сработал?
                         // значит, в процедуре slotUdpAtaTimeout нужно проверить, состояние,
                         // что все корректно отработано.
                         // А в обратнос случае, нужно что-то дополнительно проверять?
    //
    QByteArray datagram;
    datagram.resize(static_cast<int>(udpSocketAta->pendingDatagramSize()));
    udpSocketAta->readDatagram(datagram.data(), datagram.size());
    switch (cmdTypeInProgress) {
    case atacmd_sense: {
        commandInProgress = false;
//        DSSP5atasense *atasense = reinterpret_cast<DSSP5atasense*>(datagram.data());
//        if( (atasense->ata_flags_return & 0x0001) == 0x0001) { // check if we here after resending sense by slotUdpAtaTimeout
//            emit cmdData(atacmd_sense, datagram);
//        } else {
//            emit error("sense flag error");
//            return;
//        }
        emit cmdData(atacmd_sense, datagram);
        break; }
    case atacmd_id:
        if(commandInProgress) { // process first sense
            commandInProgress = false;
            if(datagram.size() != sizeof(DSSP5atasense)) {
                emit error("atacmd_id sense size error");
                return;
            }
            DSSP5atasense *atasense = reinterpret_cast<DSSP5atasense*>(datagram.data());
            if(atasense->ata_pkt_num != ataPktCount) { // check packet number
                emit error("wrong packet number in sense");
                return;
            }
            timerUdpAta->start(ETH_TIMEOUT+5); // start timeout timer
            // !!! Возможна ситуация, когда и данные приходят и таймаут случается...
        } else { // process return data
            if(datagram.size() != UDP_RET_DATA_SIZE) {
                emit error("UDP data reply size wrong");
                return;
            }
            emit cmdData(atacmd_id, datagram.mid(6, sector_s));
        }
        break;
    case atacmd_rd_buff:
        if(commandInProgress) { // process first sense
            commandInProgress = false;
            if(datagram.size() != sizeof(DSSP5atasense)) {
                emit error("atacmd_rd_buff sense size error");
                return;
            }
            DSSP5atasense *atasense = reinterpret_cast<DSSP5atasense*>(datagram.data());
            if(atasense->ata_pkt_num != ataPktCount) { // check packet number
                emit error("wrong packet number in sense");
                return;
            }
            timerUdpAta->start(ETH_TIMEOUT+5); // start timeout timer
            // !!! Возможна ситуация, когда и данные приходят и таймаут случается...
        } else { // process return data
            if(datagram.size() != UDP_RET_DATA_SIZE) {
                emit error("UDP data reply size wrong");
                return;
            }
            emit cmdData(atacmd_rd_buff, datagram.mid(6, sector_s));
        }
        break;
    case atacmd_wr_buff:
        // send sector for write
        break;
    case atacmd_wr_buff_test:
        commandInProgress = false; // no first sense ???
        emit cmdData(atacmd_wr_buff_test, datagram);
        break;
    case atacmd_wr_buff_cyc:
        if(datagram.size() != sizeof(DSSP5atasense)) { // in both cases got sense here
            emit error("atacmd_wr_buff_cyc sense size error");
            return;
        }
        if(commandInProgress) { // process first sense
            commandInProgress = false;
            DSSP5atasense *atasense = reinterpret_cast<DSSP5atasense*>(datagram.data());
            sendBuffer1Cyc(); // send data sector after send a command
            if(atasense->ata_pkt_num != ataPktCount) { // check packet number
//                emit error("wrong packet number in sense");
//                return;
            }
            // признак поданой команды в sense - взведенный бит 2
//            qDebug()<<"ata_flags_return="<<QString::number(atasense->ata_flags_return,16);
        } else { // write sector finished
            if(stopTest) {
                return;
            }
            sendReadHDDbufferCyc();
        }
        break;
    case atacmd_rd_buff_cyc:
        if(commandInProgress) { // process first sense
            commandInProgress = false;
            if(datagram.size() != sizeof(DSSP5atasense)) {
                emit error("atacmd_rd_buff sense size error");
                return;
            }
            DSSP5atasense *atasense = reinterpret_cast<DSSP5atasense*>(datagram.data());
            if(atasense->ata_pkt_num != ataPktCount) { // check packet number
                emit error("wrong packet number in sense");
                return;
            }
            timerUdpAta->start(ETH_TIMEOUT+5); // start timeout timer
            // !!! Возможна ситуация, когда и данные приходят и таймаут случается...
        } else { // process return data
            if(datagram.size() != UDP_RET_DATA_SIZE) {
                emit error("UDP data reply size wrong");
                return;
            }
            // check buffer
            if(baTestSect != datagram.mid(6, sector_s)) {
                emit error("Sector compare error");
                return;
            }
            testPassCount++;
            emit passCount(testPassCount);
//            if(testPassCount % 0xFF == 0) {
//                emit passCount(testPassCount);
//            }
            if(stopTest) {
                return;
            }
            sendWriteHDDbufferCyc(); // run write
        }
        break;
    default:
        break;
    }
}

void QAtaViaEthernet::slotUdpAtaTimeout()
{
    timerUdpAta->stop();
    switch (cmdTypeInProgress) {
    case atacmd_sense:
        emit error("sense timeout");
        break;
    case atacmd_id:
        if(commandInProgress) { // after command first sense not recived
            emit error("command soft timer timeout");
            // switch to wait hardware timer
        } else { // data sector timeout
            emit error("data sector timeout");
        }
        break;
    case atacmd_rd_buff:
        if(commandInProgress) { // after command first sense not recived
            emit error("command soft timer timeout");
            // switch to wait hardware timer
        } else { // data sector timeout
            emit error("data sector timeout");
        }
        break;
    case atacmd_wr_buff:
        // check for buffer fill
        break;
    case atacmd_wr_buff_test:
        if(commandInProgress) { // after command first sense not recived
            emit error("timeout data");
            // switch to wait hardware timer
        }
        break;
    case atacmd_wr_buff_cyc:
        if(commandInProgress) { // after command first sense not recived
            emit error(tr("timeout write sense, CMD %1").arg(CMD_WRITE_BUFFER,2,16,QLatin1Char('0')));
            // switch to wait hardware timer
        }
        break;
    case atacmd_rd_buff_cyc:
        break;
    default:
        break;
    }
    return;
    // ask for sense
    DSSP5atacmd atacmd;
    memset(&atacmd,0,sizeof(DSSP5atacmd));
    atacmd.ata_flags = 0x0001; // ask just sense
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atacmd), sizeof(DSSP5atacmd), *wukong_device_address, WUKONG_UDP_ATA);
    timerUdpAta->start(1);
}

void QAtaViaEthernet::sendSectorToBuffer()
{
    //
    DSSP5ata_sector_write atadata;
    atadata.ata_cmd_pkt_num = ataPktCount;
    ataPktCount++;
    atadata.ata_pkt_num = ataPktCount;
    atadata.ata_flags = 0x0040; // write data buffer 1, bit6 = 1
    atadata.ata_buf_fill_num = 1; // use buffer 1
    atadata.ata_cmd_sect_num = 1; // first sector of command
    // !!! copy write buffer
    // !!! make write pattern
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atadata), sizeof(DSSP5ata_sector_write), *wukong_device_address, WUKONG_UDP_ATA);
}

void QAtaViaEthernet::sendWriteHDDbufferCyc()
{
    // prepare buffer pattern
    baTestSect.clear();
    baTestSect.fill(ataPktCount % 0xFF,sector_s); // each time different
    // Как подавать запись? Дождаться подачи команды, а только потом слать данные?
    // Это должны быть разные cmdTypeInProgress! Чтобы путанницы не было.
    ataPktCount++;
    // send CMD_WRITE_BUFFER command
    DSSP5atacmd atacmd;
    atacmd.ata_pkt_num = ataPktCount;
    //
    atacmd.ata_flags = 0x0010; // PIO write with wait befor cmd, after cmd, wait drq and put one sector
    //
    atacmd.ata_timeout_before = 9000;
    atacmd.ata_timeout_after  = 10000;
    atacmd.ata_p1 = 0x00;
    atacmd.ata_p2 = 0x00;
    atacmd.ata_p3 = 0x00;
    atacmd.ata_p4 = 0x00;
    atacmd.ata_p5 = 0x00;
    atacmd.ata_p6 = 0x00;
    atacmd.ata_p7 = MAKE_WORD(CMD_WRITE_BUFFER, 0x50); // command (lo byte), ready mask (hi byte)
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atacmd), sizeof(DSSP5atacmd), *wukong_device_address, WUKONG_UDP_ATA);
    // После подачи команды дожидается бит DRQ и заполнение буфера записи
    //
    commandInProgress = true;
    cmdTypeInProgress = atacmd_wr_buff_cyc;
    timerUdpAta->start(ETH_TIMEOUT);
}

void QAtaViaEthernet::sendBuffer1Cyc()
{
    DSSP5ata_sector_write atadata;
    atadata.ata_cmd_pkt_num = ataPktCount;
    ataPktCount++;
    atadata.ata_pkt_num = ataPktCount;
    atadata.ata_flags = 0x0040; // write data buffer 1, bit6 = 1
    atadata.ata_buf_fill_num = 1; // use buffer 1
    atadata.ata_cmd_sect_num = 1; // first sector of command
    memcpy(atadata.ata_data, baTestSect.constData(), sector_s);
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atadata), sizeof(DSSP5ata_sector_write), *wukong_device_address, WUKONG_UDP_ATA);
}

void QAtaViaEthernet::sendReadHDDbufferCyc()
{
    DSSP5atacmd atacmd;
    ataPktCount++;
    atacmd.ata_pkt_num = ataPktCount;
    //
    atacmd.ata_flags = 0x0008; // PIO read with wait befor cmd, after cmd, wait drq and get one sector
    //
    atacmd.ata_timeout_before = 9000;
    atacmd.ata_timeout_after  = 10000;
    atacmd.ata_p1 = 0x00;
    atacmd.ata_p2 = 0x00;
    atacmd.ata_p3 = 0x00;
    atacmd.ata_p4 = 0x00;
    atacmd.ata_p5 = 0x00;
    atacmd.ata_p6 = 0x00;
    // send CMD_READ_BUFFER command
    atacmd.ata_p7 = MAKE_WORD(CMD_READ_BUFFER, 0x50); // command (lo byte), ready mask (hi byte)
    udpSocketAta->writeDatagram(reinterpret_cast<const char*>(&atacmd), sizeof(DSSP5atacmd), *wukong_device_address, WUKONG_UDP_ATA);
    commandInProgress = true;
    cmdTypeInProgress = atacmd_rd_buff_cyc;
    timerUdpAta->start(ETH_TIMEOUT);
}
