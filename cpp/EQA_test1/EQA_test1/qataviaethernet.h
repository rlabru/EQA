#ifndef QATAVIAETHERNET_H
#define QATAVIAETHERNET_H

#include <QObject>

#include "preferences.h"

#define ETH_TIMEOUT 1

QT_BEGIN_NAMESPACE
class QUdpSocket;
class QTimer;
class QHostAddress;
QT_END_NAMESPACE

/*! перечисление ATA команд для связки подачи команды и приема пакетов */
enum atacmd_types
{
    atacmd_nocmd,
    atacmd_sense,
    atacmd_id,
    atacmd_rd_buff,
    atacmd_wr_buff,
    atacmd_rd_buff_cyc,
    atacmd_wr_buff_cyc,
    atacmd_wr_buff_test
};

class QAtaViaEthernet : public QObject
{
    Q_OBJECT
public:
    explicit QAtaViaEthernet(QObject *parent = nullptr);

signals:
    void error(const QString &e);
    void cmdData(int cmd_type, const QByteArray &ba);
    void passCount(quint32 passCnt);

public slots:
    /// get device sense and emit back signal senseData(const QByteArray &s)
    void sendDevSense();
    /// switch device power
    void sendDevPower(bool state);
    /// send ATA_IDENTIFY_DEVICE command to HDD
    void sendIDcmd();
    /// send CMD_READ_BUFFER command to HDD
    void sendReadSectorBuffer();
    /// send CMD_WRITE_BUFFER command to HDD
    void sendWriteSectorBuffer();
    /// sending just one sector via Ethernet to HDD device buffer 1
    void sendSectorToBuffer1(const QByteArray & sect);
    /// sending just one sector via Ethernet to HDD device buffer 1 and read back it
    void sendSectorToBuffer1test(const QByteArray & sect);
    /// start HDD sector buffer read/write cycle
    void startHDDsectorBufferTest();
    /// stop HDD sector buffer read/write cycle
    void stopHDDsectorBufferTest();
    //
private slots:
    /*+pata host*/
    void readAtaUdp();
    void slotUdpAtaTimeout();
    /*-pata host*/

private:
    QHostAddress       *wukong_device_address = nullptr;
    /*+pata host*/
    QTimer             *timerUdpAta = nullptr;
    QUdpSocket         *udpSocketAta = nullptr;
    quint16            ataPktCount;
    bool               commandInProgress;
    atacmd_types       cmdTypeInProgress = atacmd_nocmd;
    void sendSectorToBuffer();
    QByteArray         baTestSect;
    bool               stopTest;
    quint32            testPassCount;
    void sendWriteHDDbufferCyc();
    void sendBuffer1Cyc();
    void sendReadHDDbufferCyc();
    /*-pata host*/
};

/*!
    \struct DSSP5atacmd

    Структура для подачи команды через ethernet UDP
    size: 22 bytes or 0x16

ata_flags[0] - ask sense for stable UDP connection
ata_flags[1] = ar_flag_skip_cmd
ata_flags[2] = ar_flag_skip_after
ata_flags[3] = ar_flag_do_read_PIO   | {bit3, bit4} == 00 or 11 -> do cmd without data
ata_flags[4] = ar_flag_do_write_PIO  |
ata_flags[5] = ar_flag_ext_cmd       | LBA 48 command - not function yet
ata_flags[6] = ar_flag_fill_buffer
*/
typedef struct _DSSP5atacmd
{
    /// WORD +00      packet number for control reply and data correct transfer
    quint16 ata_pkt_num;
    /// WORD +02      flags to define way of command send (read or write cmd and so on...)
    quint16 ata_flags;
    /// WORD +04      wait for ready before command send
    quint16 ata_timeout_before;
    /// WORD +06      wait for ready after command send
    quint16 ata_timeout_after;
    /// WORD +08      write HDD reg +1
    quint16 ata_p1;
    /// WORD +0A      write HDD reg +2
    quint16 ata_p2;
    /// WORD +0C      write HDD reg +3
    quint16 ata_p3;
    /// WORD +0E      write HDD reg +4
    quint16 ata_p4;
    /// WORD +10      write HDD reg +5
    quint16 ata_p5;
    /// WORD +12      write HDD reg +6
    quint16 ata_p6;
    /// WORD +14      write HDD reg +7
    quint16 ata_p7;
} __attribute__((packed)) DSSP5atacmd;

/*!
    \struct DSSP5ata_sector_write

    Структура для передачи сектора для записи в HDD через ethernet UDP
    size: 521 bytes or 0x209

ata_flags[6] = ar_flag_fill_buffer
Only if ar_flag_fill_buffer = 1 is active this structure working
*/
typedef struct _DSSP5ata_sector_write
{
    /// WORD +00      packet number for control reply and data correct transfer
    quint16 ata_pkt_num;
    /// WORD +02      flags to define way of command send (read or write cmd and so on...)
    quint16 ata_flags;
    /// WORD +04      packet number of current command
    quint16 ata_cmd_pkt_num;
    /// BYTE +06      write buffer fill number
    quint8  ata_buf_fill_num;
    /// WORD +07      sector number inside command
    quint16 ata_cmd_sect_num;
    /// WORD +09      write data 256 words
    quint8 ata_data[SECTOR_S];
} __attribute__((packed)) DSSP5ata_sector_write;

/*!
    \struct DSSP5atasense

    Структура получения данных от устройства ethernet UDP
    size: 26 bytes or 0x1A

Возврат пакета по сигналу atacmd_done_flag. Если машина отправки (TX FSM) будет занята, то сигнал проигнорируется...

Возвращаемые флаги в ata_flags_return
ata_flags_return[0] - AR_TIMEOUT1
ata_flags_return[1] - AR_TIMEOUT2
ata_flags_return[2] - AR_CMD_SENT
ata_flags_return[3] - AR_TIMEOUT3

ata_flags_return[4] - AR_PIO_RD1:
ata_flags_return[5] - AR_PIO_RD3SET & AR_PIO_WR3SET // # begin of HDD data read cycle
ata_flags_return[6] - AR_PIO_RD3WT: // отправка UDP пакета

ata_flags_return[7] - AR_PIO_WR1: // write setup

ata_flags_tail[0] - buf 1 filled with sector
ata_flags_tail[7] - always '1'
*/
typedef struct _DSSP5atasense
{
    /// BYTE +00
    quint8 switches;
    /// BYTE +01      hardware / structure version
    quint8 ata_ver;
    /// BYTE +02      reserved byte
    quint8 ata_res;
    /// WORD +03      packet number for control reply and data correct transfer
    quint16 ata_pkt_num;
    /// WORD +05      return flags with state bypass positions of ATA FSM
    quint16 ata_flags_return;
    /// WORD +07      wait for ready before command send
    quint16 ata_timeout_before;
    /// WORD +09      wait for ready after command send
    quint16 ata_timeout_after;
    /// WORD +0B      write HDD reg +1
    quint16 ata_p1;
    /// WORD +0D      write HDD reg +2
    quint16 ata_p2;
    /// WORD +0F      write HDD reg +3
    quint16 ata_p3;
    /// WORD +11      write HDD reg +4
    quint16 ata_p4;
    /// WORD +13      write HDD reg +5
    quint16 ata_p5;
    /// WORD +15      write HDD reg +6
    quint16 ata_p6;
    /// BYTE +17      write HDD reg +7
    quint8 ata_p7;
    /// BYTE +18
    quint8 ata_flags_p7;
    /// BYTE +19
    quint8 ata_flags_tail;
} __attribute__((packed)) DSSP5atasense;

#define MAKE_WORD(low,high) static_cast<quint16>( (((quint8) (((quint32) (low)) & 0xff)) | ((quint16) ((quint8) (((quint32) (high)) & 0xff))) << 8))

#define ATA_IDENTIFY_DEVICE              0xEC
#define CMD_READ_BUFFER                  0xE4
#define CMD_WRITE_BUFFER                 0xE8

#define UDP_RET_DATA_SIZE                1030

#endif // QATAVIAETHERNET_H
