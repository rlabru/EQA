#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "qataviaethernet.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QThread;
class QtAwesome;
class QUdpSocket;
class QTreeWidgetItem;
QT_END_NAMESPACE

#define WUKONG_UDP  8080

enum ActionColumn
{
    col_group = 0,
    col_action = 1,
    col_info = 2
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void askSense();
    void askPower(bool state);
    void sendHddId();
    void sendReadSectorBuffer();
    void sendWriteSectorBuffer();
    void sendSectorToBuffer1(const QByteArray & sect);
    void sendSectorToBuffer1Test(const QByteArray & sect);
    void startHDDsectorBufferTest();
    void stopHDDsectorBufferTest();

public slots:
    void slotAskSense();
    void slotPowerOn();
    void slotPowerOff();
    void slotHDDID();
    void slotSectBufRd();
    void slotSectBufWr();
    void slotSect2WrBuf();
    void slotSect2WrBufTest();
    void slotSectRdWrBufTest();
    void slotPassCount(quint32 passCnt);
    void slotSectStop();
    void slotError(const QString &e);
    void slotCmdData(int cmd_type, const QByteArray &ba);

private slots:
    /*+struct parse*/
    void slotItemChanged(QTreeWidgetItem *c, QTreeWidgetItem *p);
    /*-struct parse*/

private:
    Ui::MainWindow     *ui                    = nullptr;
    QTreeWidgetItem    *treeUDP_pkt_cnt       = nullptr;
    QTreeWidgetItem    *treeUDP_switch        = nullptr;
    QTreeWidgetItem    *treeUDP_dev_pkt_cnt   = nullptr;
    QTreeWidgetItem    *treeHDD_pass_cnt      = nullptr;
    QTreeWidgetItem    *treeUDP_error         = nullptr;
    quint32            udp_pkt_cnt            = 0;
    //
    QList<QByteArray>  listData;
    int                listShowIndex          = 0;
    QTreeWidgetItem    *treeCMDCNT            = nullptr;
    int                cmdCnt                 = 0;
    /*+eqa*/
    QThread            *worker;
    QAtaViaEthernet    *eqa;
    /*-eqa*/
    /*+struct parse*/
    QTreeWidgetItem    *tree_struct_hdd_id    = nullptr;
    QTreeWidgetItem    *tree_struct_sense     = nullptr;
    void clear_all();
    void clear_ATA_ID();
    void fill_ATA_ID(const QByteArray &hddID);
    void clear_Sense();
    void fill_Sense(const QByteArray &s);
    /*-struct parse*/
};

#define SARRAY "array"
#define SBYTE  "byte"
#define SWORD  "word"
#define SDWORD "dword"
#define SINT   "int"
#define SBIT   "bit"

#define ATA_ID_SERIAL_STR_SIZE    20
#define ATA_ID_MODEL_STR_SIZE     40
#define ATA_ID_FIRMVARE_STR_SIZE  8

/* Bit operations, like in PIC microcontrollers                                     */
/* CyBix 2001                                                                       */
/* ******************************************************************************** */
#define BSF(reg,bit) reg = reg|(1<<bit)    /// Set bit in variable reg
#define BCF(reg,bit) reg = reg&(~(1<<bit)) /// Clear bit in variable reg
#define BS(reg,bit)	( reg&(1<<bit) )       /// Returns true if specifed bit is set
#define BC(reg,bit) ( !(reg&(1<<bit)) )    /// Returns true if specifed bit is clear
/* ******************************************************************************** */

typedef struct _ATAIdentifyDevice
{
  /// <ul>
  /// <li> WORD 000     General configuration bit-significant information
  ///  <ul>
  ///   <li>            bit0    Reserved
  ///   <li>            bit1    Retired
  ///   <li>            bit2    Response incomplete
  ///   <li>            bit3-5  Retired
  ///   <li>            bit6-7  Obsolete
  ///   <li>            bit8-14 Retired
  ///   <li>            bit15   0 = ATA device
  ///  </ul>
  /// <li> WORD 001     Obsolete
  /// <li> WORD 002     Specific configuration
  /// <li> WORD 003     Obsolete
  /// <li> WORD 004-005 Retired
  /// <li> WORD 006     Obsolete
  /// <li> WORD 007-008 Reserved for assignment by the CompactFlash? Association
  /// <li> WORD 009     Retired
  /// </ul>
  unsigned short words000_009[10];

  /// WORD 010-019 Serial number (20 ASCII characters)
  unsigned char  serial_no[ATA_ID_SERIAL_STR_SIZE];

  /// WORD 020     Retired
  unsigned short word20;

  /// WORD 021     (cache size)
  unsigned short cache_size;

  /// WORD 022     Retired
  unsigned short word22;

  /// WORD 023-026 Firmware revision (ASCII String)
  unsigned char  fw_rev[ATA_ID_FIRMVARE_STR_SIZE];

  /// WORD 027-046 Model number (ASCII String)
  unsigned char  model[ATA_ID_MODEL_STR_SIZE];

  /// <ul>
  /// <li> WORD 047
  ///  <ul>
  ///    <li>      bit0-7   00h = Reserved
  ///    <li>      bit8-15  80h    01h-FFh = Maximum number of sectors that shall be transferred per interrupt on READ/WRITE MULTIPLE commands
  ///  </ul>
  /// <li> WORD 048     Trusted Computing feature set options
  ///  <ul>
  ///    <li>      bit0     1=Trusted Computing feature set is supported
  ///    <li>      bit1-13  Reserved for the Trusted Computing Group
  ///    <li>      bit14    Shall be set to one
  ///    <li>      bit15    15 Shall be cleared to zero
  ///  </ul>
  /// <li> WORD 049     Capabilities
  ///              bit0-7   Retired
  ///              bit8     1 = DMA supported
  ///              bit9     1 = LBA supported
  ///              bit10    1 = IORDY may be disabled
  ///              bit11    1 = IORDY supported    0 = IORDY may be supported
  ///              bit12    Reserved for the IDENTIFY PACKET DEVICE command
  ///              bit13    1 = Standby timer values as specified in this standard are supported  0 = Standby timer values shall be managed by the device
  ///              bit14-15 Reserved for the IDENTIFY PACKET DEVICE command
  /// <li> WORD 050     Capabilities
  ///              bit0     Shall be set to one to indicate a device specific Standby timer value minimum
  ///              bit1     Obsolete
  ///              bit2-13  Reserved
  ///              bit14    Shall be set to one
  ///              bit15    Shall be cleared to zero
  /// <li> WORD 051-052 Obsolete
  /// <li> WORD 053     bit0     Obsolete
  ///              bit1     1 = the fields reported in words (70:64) are valid    0 = the fields reported in words (70:64) are not valid
  ///              bit2     1 = the fields reported in word 88 are valid    0 = the fields reported in word 88 are not valid
  ///              bit3-7   Reserved
  ///              bit8-15  Free-fall Control Sensitivity 00h = Vendor's recommended setting
  ///                                                     01h-FFh = Sensitivity level. A larger number is a more sensitive setting
  /// <li> WORD 054-058 Obsolete
  /// <li> WORD 059     bit0-7   xxh = Current setting for number of sectors that shall be transferred per interrupt on R/W Multiple command
  ///              bit8     1 = Multiple sector setting is valid
  ///              bit9-15  Reserved
  /// <li> WORD 060-061 Total number of user addressable sectors
  /// <li> WORD 062     Obsolete
  /// <li> WORD 063     bit0     1 = Multiword DMA mode 0 is supported
  ///              bit1     1 = Multiword DMA mode 1 and below are supported
  ///              bit2     1 = Multiword DMA mode 2 and below are supported
  ///              bit3-7   Reserved
  ///              bit8     1 = Multiword DMA mode 0 is selected 0 = Multiword DMA mode 0 is not selected
  ///              bit9     1 = Multiword DMA mode 1 is selected 0 = Multiword DMA mode 1 is not selected
  ///              bit10    1 = Multiword DMA mode 2 is selected 0 = Multiword DMA mode 2 is not selected
  ///              bit11-15 Reserved
  /// <li> WORD 064     bit0-7   PIO modes supported
  ///              bit8-15  Reserved
  /// <li> WORD 065     Minimum Multiword DMA transfer cycle time per word
  ///              bit0-15  Cycle time in nanoseconds
  /// <li> WORD 066     Manufacturer's recommended Multiword DMA transfer cycle time
  ///              bit0-15  Cycle time in nanoseconds
  /// <li> WORD 067     Minimum PIO transfer cycle time without flow control
  ///              bit0-15  Cycle time in nanoseconds
  /// <li> WORD 068     Minimum PIO transfer cycle time with IORDY flow control
  ///              bit0-15  Cycle time in nanoseconds
  unsigned short words047_068[22];

  /// WORD 069 Additional Supported
  /// <ul>
  /// <li>         bit0-1   Zoned Capabilities
  /// <li>         bit2     All write cache is non-volatile
  /// <li>         bit3     Extended Number of User Addressable Sectors is supported
  /// <li>         bit4     Device Encrypts All User Data on the device
  /// <li>         bit5     Trimmed LBA range(s) returning zeroed data is supported
  /// <li>         bit6     0 = Optional ATA device 28-bit commands supported
  /// <li>         bit7     Reserved for IEEE 1667
  /// <li>         bit8     DOWNLOAD MICROCODE DMA is supported
  /// <li>         bit9     Obsolete
  /// <li>         bit10    WRITE BUFFER DMA is supported
  /// <li>         bit11    READ BUFFER DMA is supported
  /// <li>         bit12    Obsolete
  /// <li>         bit13    Long Physical Sector Alignment Error Reporting Control is supported
  /// <li>         bit14    Deterministic data in trimmed LBA range(s) is supported
  /// <li>         bit15    Reserved for CFA
  /// </ul>
  unsigned short additional_supported;

  /// <li> WORD 070 Reserved
  /// <li> WORD 071-074 Reserved for the IDENTIFY PACKET DEVICE command
  /// <li> WORD 075     bit0-4   Maximum queue depth - 1
  ///              bit5-15
  /// <li> WORD 076     Serial ATA Capabilities
  ///              bit0     Shall be cleared to zero
  ///              bit1     1 = Supports SATA Gen1 Signaling Speed (1.5Gb/s)
  ///              bit2     1 = Supports SATA Gen2 Signaling Speed (3.0Gb/s)
  ///              bit3-7   Reserved for future SATA signaling speed grades
  ///              bit8     1 = Supports native Command Queuing
  ///              bit9     1 = Supports receipt of host initiated power management requests
  ///              bit10    1 = Supports Phy Event Counters
  ///              bit11-15 Reserved for Serial ATA
  /// <li> WORD 077     Reserved for Serial ATA
  /// <li> WORD 078     SATA Features Supported
  ///              bit0     Shall be cleared to zero
  ///              bit1     1 = Device supports non-zero buffer offsets
  ///              bit2     1 = Device supports DMA Setup auto-activation
  ///              bit3     1 = Device supports initiating power management
  ///              bit4     1 = Device supports in-order data delivery
  ///              bit5     Reserved for Serial ATA
  ///              bit6     1 = Device supports Software Settings Preservation
  ///              bit7-15  Reserved for Serial ATA
  /// <li> WORD 079     SATA Features Enabled
  ///              bit0     Shall be cleared to zero
  ///              bit1     1 = Non-zero buffer offsets enabled
  ///              bit2     1 = DMA Setup auto-activation enabled
  ///              bit3     1 = Device initiated power management enabled
  ///              bit4     1 = In-order data delivery enabled
  ///              bit5     Reserved for Serial ATA
  ///              bit6     1 = Software Settings Preservation enabled
  ///              bit7-15   Reserved for Serial ATA
  /// </ul>
  unsigned short words070_079[10];

  /// WORD 080     Major revision number 0000h or FFFFh = device does not report version
  /// <ul>
  /// <li>         bit0     Reserved
  /// <li>         bit1     Obsolete
  /// <li>         bit2     Obsolete
  /// <li>         bit3     Obsolete
  /// <li>         bit4     1 = supports ATA/ATAPI-4
  /// <li>         bit5     1 = supports ATA/ATAPI-5
  /// <li>         bit6     1 = supports ATA/ATAPI-6
  /// <li>         bit7     1 = supports ATA/ATAPI-7
  /// <li>         bit8     1 = supports ATA8-ACS
  /// <li>         bit9-15  Reserved    0000h or FFFFh = device does not report version
  /// </ul>
  unsigned short major_rev_num;

  /// WORD 081     Minor version number
  unsigned short minor_rev_num;

  /// WORD 082     Command set supported
  /// <ul>
  /// <li>         bit0     1 = SMART feature set supported
  /// <li>         bit1     1 = Security Mode feature set supported
  /// <li>         bit2     1 = Removable Media feature set supported
  /// <li>         bit3     1 = mandatory Power Management feature set supported
  /// <li>         bit4     Shall be cleared to zero to indicate that the PACKET Command feature set is not supported
  /// <li>         bit5     1 = write cache supported
  /// <li>         bit6     1 = look-ahead supported
  /// <li>         bit7     1 = release interrupt supported
  /// <li>         bit8     1 = SERVICE interrupt supported
  /// <li>         bit9     1 = DEVICE RESET command supported
  /// <li>         bit10    1 = Host Protected Area feature set supported
  /// <li>         bit11    Obsolete
  /// <li>         bit12    1 = WRITE BUFFER command supported
  /// <li>         bit13    1 = READ BUFFER command supported
  /// <li>         bit14    1 = NOP command supported
  /// <li>         bit15    Obsolete
  /// </ul>
  unsigned short command_set_1;

  /// WORD 083     Command sets supported
  /// <ul>
  /// <li>         bit0     1 = DOWNLOAD MICROCODE command supported
  /// <li>         bit1     1 = READ/WRITE DMA QUEUED supported
  /// <li>         bit2     1 = CFA feature set supported
  /// <li>         bit3     1 = Advanced Power Management feature set supported
  /// <li>         bit4     Obsolete
  /// <li>         bit5     1 = Power-Up In Standby feature set supported
  /// <li>         bit6     1 = SET FEATURES subcommand required to spin-up after power-up
  /// <li>         bit7     Reserved for technical report INCITS TR27-2001
  /// <li>         bit8     1 = SET MAX security extension supported
  /// <li>         bit9     1 = Automatic Acoustic Management feature set supported
  /// <li>         bit10    1 = 48-bit Address feature set supported
  /// <li>         bit11    1 = Device Configuration Overlay feature set supported
  /// <li>         bit12    1 = mandatory FLUSH CACHE command supported
  /// <li>         bit13    1 = FLUSH CACHE EXT command supported
  /// <li>         bit14    Shall be set to one
  /// <li>         bit15    Shall be cleared to zero
  /// </ul>
  unsigned short command_set_2;

  /// WORD 084     Command set/feature supported
  /// <ul>
  /// <li>         bit0     1 = SMART error logging supported
  /// <li>         bit1     1 = SMART self-test supported
  /// <li>         bit2     1 = Media serial number supported
  /// <li>         bit3     1 = Media Card Pass Through Command feature set supported
  /// <li>         bit4     1 = Streaming feature set supported
  /// <li>         bit5     1 = General Purpose Logging feature set supported
  /// <li>         bit6     1 = WRITE DMA FUA EXT and WRITE MULTIPLE FUA EXT commands supported
  /// <li>         bit7     1 = WRITE DMA QUEUED FUA EXT command supported
  /// <li>         bit8     1 = 64-bit World wide name supported
  /// <li>         bit9-10  1 = Obsolete
  /// <li>         bit11    Reserved for technical report INCITS TR37-2004
  /// <li>         bit12    Reserved for technical report INCITS TR37-2004
  /// <li>         bit13    1 = IDLE IMMEDIATE with UNLOAD FEATURE supported
  /// <li>         bit14    Shall be set to one
  /// <li>         bit15    Shall be cleared to zero
  /// </ul>
  unsigned short command_set_extension;

  /// WORD 085     Command set/feature enabled/supported
  /// <ul>
  /// <li>         bit0     1 = SMART feature set enabled
  /// <li>         bit1     1 = Security feature set enabled
  /// <li>         bit2     Obsolete
  /// <li>         bit3     Shall be set to one to indicate that the Mandatory Power Management feature set is supported
  /// <li>         bit4     Shall be cleared to zero to indicate that the PACKET feature set is not supported
  /// <li>         bit5     1 = volatile write cache enabled
  /// <li>         bit6     1 = read look-ahead enabled
  /// <li>         bit7     1 = release interrupt enabled
  /// <li>         bit8     1 = SERVICE interrupt enabled
  /// <li>         bit9     1 = DEVICE RESET command supported
  /// <li>         bit10    1 = Host Protected Area has been established (i.e., the maximum LBA is less than the maximum native LBA)
  /// <li>         bit11    Obsolete
  /// <li>         bit12    1 = WRITE BUFFER command supported
  /// <li>         bit13    1 = READ BUFFER command supported
  /// <li>         bit14    1 = NOP command supported
  /// <li>         bit15    Obsolete
  /// </ul>
  unsigned short cfs_enable_1;

  /// WORD 086     Command set/feature enabled/supported
  /// <ul>
  /// <li>         bit0     1 = DOWNLOAD MICROCODE command supported
  /// <li>         bit1     1 = READ/WRITE DMA QUEUED command supported
  /// <li>         bit2     1 = CFA feature set supported
  /// <li>         bit3     1 = Advanced Power Management feature set enabled
  /// <li>         bit4     Obsolete
  /// <li>         bit5     1 = Power-Up In Standby feature set enabled
  /// <li>         bit6     1 = SET FEATURES subcommand required to spin-up after power-up
  /// <li>         bit7     Reserved for technical report INCITS TR27-2001
  /// <li>         bit8     1 = SET MAX security extension enabled by SET MAX SET PASSWORD
  /// <li>         bit9     1 = Automatic Acoustic Management feature set enabled
  /// <li>         bit10    1 = 48-bit Address features set supported
  /// <li>         bit11    1 = Device Configuration Overlay supported
  /// <li>         bit12    1 = FLUSH CACHE command supported
  /// <li>         bit13    1 = FLUSH CACHE EXT command supported
  /// <li>         bit14    Reserved
  /// <li>         bit15    1 = Words 120:119 are valid
  /// </ul>
  unsigned short word086;

  /// WORD 087     Command set/feature enabled/supported
  /// <ul>
  /// <li>          bit0     1 = SMART error logging supported
  /// <li>          bit1     1 = SMART self-test supported
  /// <li>          bit2     1 = Media serial number is valid
  /// <li>          bit3     1 = Media Card Pass Through Command feature set supported
  /// <li>          bit4     Obsolete
  /// <li>          bit5     1 = General Purpose Logging feature set supported
  /// <li>          bit6     1 = WRITE DMA FUA EXT and WRITE MULTIPLE FUA EXT commands supported
  /// <li>          bit7     1 = WRITE DMA QUEUED FUA EXT command supported
  /// <li>          bit8     1 = 64 bit World wide name supported
  /// <li>          bit9-10  Obsolete
  /// <li>          bit11    Reserved for technical report INCITS TR37-2004
  /// <li>          bit12    Reserved for technical report INCITS TR37-2004
  /// <li>          bit13    1 = IDLE IMMEDIATE with UNLOAD FEATURE supported
  /// <li>          bit14    Shall be set to one
  /// <li>          bit15    Shall be cleared to zero
  /// </ul>
  unsigned short csf_default;

  /// WORD 088     Ultra DMA modes
  ///              bit0     1 = Ultra DMA mode 0 is supported
  ///              bit1     1 = Ultra DMA mode 1 and below are supported
  ///              bit2     1 = Ultra DMA mode 2 and below are supported
  ///              bit3     1 = Ultra DMA mode 3 and below are supported
  ///              bit4     1 = Ultra DMA mode 4 and below are supported
  ///              bit5     1 = Ultra DMA mode 5 and below are supported
  ///              bit6     1 = Ultra DMA mode 6 and below are supported
  ///              bit7     Reserved
  ///              bit8     1 = Ultra DMA mode 0 is selected 0 = Ultra DMA mode 0 is not selected
  ///              bit9     1 = Ultra DMA mode 1 is selected 0 = Ultra DMA mode 1 is not selected
  ///              bit10    1 = Ultra DMA mode 2 is selected 0 = Ultra DMA mode 2 is not selected
  ///              bit11    1 = Ultra DMA mode 3 is selected 0 = Ultra DMA mode 3 is not selected
  ///              bit12    1 = Ultra DMA mode 4 is selected 0 = Ultra DMA mode 4 is not selected
  ///              bit13    1 = Ultra DMA mode 5 is selected 0 = Ultra DMA mode 5 is not selected
  ///              bit14    1 = Ultra DMA mode 6 is selected 0 = Ultra DMA mode 6 is not selected
  ///              bit15    Reserved
  /// WORD 089     Time required for security erase unit completion
  /// WORD 090     Time required for Enhanced security erase completion
  /// WORD 091     Current advanced power management value
  /// WORD 092     Master Password Identifier
  /// WORD 093     Hardware reset result. The contents of bits (12:0) of this word shall change only
  ///              during the execution of a hardware reset. See 7.16.7.44 for more information.
  ///                       7:0 Device 0 hardware reset result. Device 1 shall clear these bits to zero. Device 0 shall set these bits as follows
  ///              bit0     Shall be set to one
  ///              bit1-2   These bits indicate how Device 0 determined the device number
  ///                       00 = Reserved
  ///                       01 = a jumper was used
  ///                       10 = the CSEL signal was used
  ///                       11 = some other method was used or the method is unknown
  ///              bit3     0 = Device 0 failed diagnostics 1 = Device 0 passed diagnostics
  ///              bit4     0 = Device 0 did not detect the assertion of PDIAG- 1 = Device 0 detected the assertion of PDIAG-
  ///              bit5     0 = Device 0 did not detect the assertion of DASP- 1 = Device 0 detected the assertion of DASP-
  ///              bit6     0 = Device 0 does not respond when Device 1 is selected 1 = Device 0 responds when Device 1 is selected
  ///              bit7     Reserved
  ///              bit8     Shall be set to one
  ///              bit9-10  These bits indicate how Device 1 determined the device number:
  ///                       00 = Reserved
  ///                       01 = a jumper was used
  ///                       10 = the CSEL signal was used
  ///                       11 = some other method was used or the method is unknown
  ///              bit11    0 = Device 1 did not assert PDIAG- 1 = Device 1 asserted PDIAG-
  ///              bit12    Reserved
  ///              bit13    1 = device detected CBLID- above VBiHB 0 = device detected CBLID- below VBiL
  ///              bit14    Shall be set to one
  ///              bit15    Shall be cleared to zero
  /// WORD 094     bit0-7   Current automatic acoustic management value
  ///              bit8-15  Vendor's recommended acoustic management value
  /// WORD 095     Stream Minimum Request Size
  /// WORD 096     Streaming Transfer Time - DMA
  /// WORD 097     Streaming Access Latency - DMA and PIO
  /// WORD 098-099 Streaming Performance Granularity
  unsigned short words088_99[12];

  /// WORD 100-103 Total Number of User Addressable Sectors for the 48-bit Address feature set.
  quint16 maxlba[4];

  /// WORD 104     Streaming Transfer Time - PIO
  quint16 streamingTransferTime;

  /// WORD 105     Reserved
  quint16 word105;

  /// WORD 106     Physical sector size / Logical Sector Size
  /// <ul>
  /// <li>         bit0-3   2PXP logical sectors per physical sector
  /// <li>         bit4-11  Reserved
  /// <li>         bit12    1= Device Logical Sector Longer than 256 Words
  /// <li>         bit13    1 = Device has multiple logical sectors per physical sector
  /// <li>         bit14    Shall be set to one
  /// <li>         bit15    Shall be cleared to zero
  /// </ul>
  quint16 phys_sect_sz;

  /// WORD 107     Inter-seek delay for ISO-7779 acoustic testing in microseconds
  /// WORD 108     bit0-11  IEEE OUI (23:12)
  ///              bit12-15 NAA (3:0)
  /// WORD 109     bit0-3   Unique ID (35:32)
  ///              bit4-15  IEEE OUI (11:0)
  /// WORD 110     bit0-15  Unique ID (31:16)
  /// WORD 111     bit0-15  Unique ID (15:0)
  /// WORD 112-115 Reserved for world wide name extension to 128 bits
  /// WORD 116     Reserved for INCITS TR37-2004
  /// WORD 117-118 Words per Logical Sector
  /// WORD 119     Supported Settings (Continued from words 84:82)
  ///              bit0     Reserved for technical report DT1825
  ///              bit1     1 = Write-Read-Verify feature set is supported
  ///              bit2     1 = WRITE UNCORRECTABLE EXT is supported
  ///              bit3     1 = READ and WRITE DMA EXT GPL optional commands are supported
  ///              bit4     1 = The Segmented feature for DOWNLOAD MICROCODE is supported
  ///              bit5     1 = Free-fall Control feature set is supported
  ///              bit6-13  Reserved
  ///              bit14    Shall be set to one
  ///              bit15    Shall be cleared to zero
  /// WORD 120     Command set/feature enabled/supported. (Continued from words 87:85)
  ///              bit0     Reserved for technical report DT1825
  ///              bit1     1 = Write-Read-Verify feature set is enabled
  ///              bit2     1 = WRITE UNCORRECTABLE EXT is supported
  ///              bit3     1 = READ and WRITE DMA EXT GPL optional commands are supported
  ///              bit4     1 = The Segmented feature for DOWNLOAD MICROCODE is supported
  ///              bit5     1 = Free-fall Control feature set is enabled
  ///              bit6-13  Reserved
  ///              bit14    Shall be set to one
  ///              bit15    Shall be cleared to zero
  /// WORD 121-126 Reserved for expanded supported and enabled settings
  /// WORD 127     Obsolete
  /// WORD 128     Security status
  ///              bit0     1 = Security supported
  ///              bit1     1 = Security enabled
  ///              bit2     1 = Security locked
  ///              bit3     1 = Security frozen
  ///              bit4     1 = Security count expired
  ///              bit5     1 = Enhanced security erase supported
  ///              bit6-7   Reserved
  ///              bit8     Security level 0 = High, 1 = Maximum
  ///              bit9-15  Reserved
  /// WORD 129-159 Vendor specific
  /// WORD 160     CFA power mode 1
  ///              bit0-11  Maximum current in ma
  ///              bit12    CFA power mode 1 disabled
  ///              bit13    CFA power mode 1 is required for one or more commands implemented by the device
  ///              bit14    Reserved
  ///              bit15    Word 160 supported
  /// WORD 161-175 Reserved for assignment by the CompactFlash? Association
  /// WORD 176-205 Current media serial number (ASCII String)
  /// WORD 206     SCT Command Transport
  ///              bit0     SCT Command Transport supported
  ///              bit1     SCT Command Transport Long Sector Access supported
  ///              bit2     SCT Command Transport Write Same supported
  ///              bit3     SCT Command Transport Error Recovery Control supported
  ///              bit4     SCT Command Transport Features Control supported
  ///              bit5     SCT Command Transport Data Tables supported
  ///              bit6-11  Reserved
  ///              bit12-15 Vendor Specific
  /// WORD 207-208 Reserved for CE-ATA
  /// WORD 209     Alignment of logical blocks within a larger physical block
  ///              bit0-13  "Logical sector" offset within the first physical sector where the first logical sector is placed
  ///              bit14    Shall be set to one
  ///              bit15    Shall be cleared to zero
  /// WORD 210-211 Write-Read-Verify Sector Count Mode 3 Only
  /// WORD 212-213 Verify Sector Count Mode 2 Only
  /// WORD 214     NV Cache Capabilities
  ///              bit0     1 = NV Cache Power Mode feature set supported
  ///              bit1     1 = NV Cache Power Mode feature set enabled
  ///              bit2-3   Reserved
  ///              bit4     1 = NV Cache feature set enabled
  ///              bit5-7   Reserved
  ///              bit8-11  NV Cache Power Mode feature set version
  ///              bit12-15 NV Cache feature set version
  /// WORD 216     NV Cache Size in Logical Blocks (31:16)
  /// WORD 217     Nominal media rotation rate
  /// WORD 218     Reserved
  /// WORD 219     NV Cache Options
  ///              bit0-7   Device Estimated Time to Spin Up in Seconds
  ///              bit8-15  Reserved
  /// WORD 220     bit0-7   Write-Read-Verify feature set current mode
  ///              bit8-15  Reserved
  /// WORD 221     Reserved
  /// WORD 222     Transport Major revision number. 0000h or FFFFh = device does not report version
  ///              15:12 Transport Type 0 = Parallel, 1 = Serial, 2-15 = Reserved
  ///                   Parallel   Serial
  ///              11:5 Reserved   Reserved
  ///                 4 Reserved   SATA Rev 2.6
  ///                 3 Reserved   SATA Rev 2.5
  ///                 2 Reserved   SATA II: Extensions
  ///                 1 Reserved   SATA 1.0a
  ///                 0 ATA8-APT   ATA8-AST
  /// WORD 223     Transport Minor revision number
  /// WORD 224-233 Reserved for CE-ATA
  /// WORD 234     Minimum number of 512 byte units per DOWNLOAD MICROCODE command for mode 03h
  /// WORD 235     Maximum number of 512 byte units per DOWNLOAD MICROCODE command for mode 03h
  /// WORD 236-254 Reserved
  quint16 reserv_107_254[148];

  /// WORD 255     Integrity word
  /// <ul>
  /// <li>         bit0-7   Signature
  /// <li>         bit8-15  Checksum
  /// </ul>
  quint16 integrityWord;
} __attribute__((packed)) AtaIdentifyDevice;


#endif // MAINWINDOW_H
