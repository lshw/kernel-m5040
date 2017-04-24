#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/version.h>
#ifndef NULL
#define NULL 0
#endif
//=============================================================================
#define DRV_VERSION_MAJOR	1
#define DRV_VERSION_MINOR	1
#define DRV_VERSION_BUILD	5
//==============================================================================
#define _ULTRA_DMA133		6
#define _ULTRA_DMA100		5
#define _ULTRA_DMA66		4
#define _ULTRA_DMA33		2
#define _MAX_ULTRA		_ULTRA_DMA133
#define _MAX_PIO_MODE		4
//=============================================================================
#define MAX_ADAPTER		4
#define	MAX_COMMAND_QUEUE_CH	7
#define	MAX_QUEUE_SUPPORT	16
#define MAX_SCATTER		17
#define MAX_XFER_SECTORS		128
#define MAX_CMDLUN		1
#define MAX_SATA_CHANNEL		2
#define MAX_SATA_HDD		5 
#define MAX_CHAIN_LEVEL		1
#define MAXTARGETHDD		((MAX_SATA_CHANNEL * MAX_SATA_HDD) + 2)
#define TIMER_SCALE		(HZ/10)		// 100mS
//=============================================================================
#define MEM_512B			(0x000200)
#define MEM_1KB			(0x000400)
#define MEM_2KB			(0x000800)
#define MEM_4KB			(0x001000)
#define MEM_8KB			(0x002000)
#define MEM_16KB			(0x004000)
#define MEM_32KB			(0x008000)
#define MEM_64KB			(0x010000)
#define MEM_128KB		(0x020000)
#define MEM_256KB		(0x040000)
#define MEM_512KB		(0x080000)
#define MEM_1MB			(0x100000)
#define MEM_2MB			(0x200000)
//=============================================================================
#define MEM_8KB_ORDER           1
#define MEM_16KB_ORDER          2
#define MEM_32KB_ORDER          3
#define MEM_64KB_ORDER          4
#define MEM_128KB_ORDER         5
#define MEM_256KB_ORDER         6
#define MEM_512KB_ORDER         7
#define MEM_1MB_ORDER           8
#define MEM_2MB_ORDER           9
//==============================================================================
#define EXTENSION_MEM		MEM_64KB_ORDER
#define EXTENSION_MEM_LEN	MEM_64KB
#define WORKINGTABLE_MEM_LEN	(MEM_32KB + MEM_16KB)
//==============================================================================
#define SCSIOP_INQUIRY		0x12
#define SCSIOP_REQUEST_SENSE	0x03
#define SCSIOP_READ_CAPACITY	0x25
#define SCSIOP_MODE_SENSE	0x1A	//MODE SENSE (6)
#define SCSIOP_MODE_SENSE10	0x5A	//MODE SENSE (10)
#define SCSIOP_MEDIUM_REMOVAL	0x1E
#define SCSIOP_MODE_SELECT	0x15	//MODE Select (6)
#define SCSIOP_MODE_SELECT10	0x55	//MODE Select (10)
#define SCSIOP_TEST_UNIT_READY	0x00
#define SCSIOP_START_STOP_UNIT	0x1B
#define SCSIOP_SEEK6		0x0B
#define SCSIOP_SEEK		0x2B	//SEEK(10)
#define SCSIOP_VERIFY		0x2F
#define SCSIOP_READ6		0x08	//READ(6)	
#define SCSIOP_READ10		0x28	//READ(10)
#define SCSIOP_READ12		0xA8	//READ(12)
#define SCSIOP_READ16		0x88	//READ(16)
#define SCSIOP_WRITE6		0x0A	//WRITE(6)
#define SCSIOP_WRITE10		0x2A	//WRITE(10)
#define SCSIOP_WRITE12		0xAA	//WRITE(12)
#define SCSIOP_WRITE16		0x8A	//WRITE(16)
#define SCSIOP_SYNC_CACHE	0x35
//vendor define==================================================================
#define Updata_NonData		0x38
#define Fireware_ReadBuffer	0x3C
#define Fireware_WriteBuffer	0x3B
//==============================================================================
#define IDE_READ_PIO		0x20
#define IDE_READ_PIO_EXT		0x24
#define IDE_READ_DMA		0xC8
#define IDE_READ_DMA_EXT		0x25
#define IDE_NCQ_READ		0x60
#define IDE_WRITE_PIO		0x30
#define IDE_WRITE_PIO_EXT	0x34
#define IDE_WRITE_DMA		0xCA
#define IDE_WRITE_DMA_EXT	0x35
#define IDE_NCQ_WRITE		0x61
#define IDE_VERIFY		0x40
#define IDE_VERIFY_EXT		0x42
#define IDE_SEEK			0x70
#define IDE_FLUSH_CACHE		0xE7
#define IDE_FLUSH_CACHE_EXT	0xEA
#define ATAPI_READ_CD		0xBE
#define ATAPI_WRITE_CONTINUE	0xE1
#define I2C_CMD_READ		0xF0
#define I2C_CMD_WRITE		0xF1
//======================vedor define=============================================
//=========================I2C use======================================================
#define STA		 	0x80
#define STO 			0x40
#define RD			0x20
#define WR			0x10
#define AC			0x8
#define IFS			0x4
#define IFM			0x2
#define TIP			0x1
//======================================I2C step
#define	NONE			0
#define R_F1			0x1
#define R_F2			0x2
#define R_F3			0x3
#define R_F4			0x4
#define R_F5			0x5
#define W_F1			0x6
#define W_F2			0x7
#define W_F3			0x8
#define W_F4			0x9
//=============================================================================
#define STATUS_RESET		(DID_RESET << 16)		// 0x00080000
#define STATUS_GOOD		(DID_OK << 16)		// 0x00000000
#define STATUS_BUSY		(DID_BUS_BUSY << 16)	// 0x00020000
#define STATUS_NO_DEVICE		(DID_BAD_TARGET << 16)	// 0x00040000
#define STATUS_CHECK_CONDITION	0x02
#define STATUS_PENDING		0xFFF1

#define  SAM_STAT_CHECK_CONDITION 0x02
//;===========================================================================
#define ATP8620_DEVID		0x000D
#define DRIVER_NAME		"atp8620"
#if __LITTLE_ENDIAN
#define ADAPTER_INFORMATION	"ACARD ATP8620 PCI/PCI-X SATA Adapter Driver(LE)"
#else
#define ADAPTER_INFORMATION	"ACARD ATP8620 PCI/PCI-X SATA Adapter Driver(BE)"
#endif
#define ADAPTER_NAME		"atp8620"
#define PROC_INFORMATION		"ACARD ATP8620 Driver"
#define PROC_NAME		"atp8620"                   // folder name in /proc/scsi/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#define SCSI_DRV_TEMPLATE 			\
{						\
	next: NULL,				\
	module: NULL,				\
	name: ADAPTER_NAME,			\
	proc_name: PROC_NAME,			\
	proc_info: adapter_proc_info,		\
	info: adapter_info,			\
	bios_param: adapter_biosparam,		\
	detect: adapter_detect, 			\
	release: adapter_release,			\
	queuecommand: adapter_queuecommand,		\
	abort: NULL,				\
	reset: adapter_reset,			\
	command: NULL,				\
	eh_strategy_handler: NULL,			\
	eh_abort_handler: NULL, 			\
	eh_device_reset_handler: NULL,		\
	eh_bus_reset_handler: NULL,		\
	eh_host_reset_handler: NULL,		\
	slave_attach: NULL,			\
	can_queue: MAX_QUEUE_SUPPORT,		\
	this_id: 16,				\
	sg_tablesize: MAX_SCATTER,			\
	cmd_per_lun: MAX_CMDLUN,			\
	present: 0,				\
	unchecked_isa_dma: 0,			\
	use_clustering: ENABLE_CLUSTERING,		\
	use_new_eh_code: 1,			\
	max_sectors: MAX_XFER_SECTORS,		\
}
#define PCI_DEVICE_ID_TABLE 								\
{											\
	{ PCI_VENDOR_ID_ARTOP, ATP8620_DEVID,  PCI_VENDOR_ID_ARTOP, ATP8620_DEVID },	\
}
#define DMA_TO_DEVICE	PCI_DMA_TODEVICE
#else
#define Scsi_Cmnd struct scsi_cmnd
#define SCSI_DRV_TEMPLATE 					\
{								\
	.module			= THIS_MODULE,			\
	.name			= ADAPTER_NAME		,	\
	.proc_name		= PROC_NAME,			\
	.proc_info		= adapter_proc_info,		\
	.info			= adapter_info,			\
	.bios_param		= adapter_biosparam,		\
	.queuecommand		= adapter_queuecommand,		\
	.eh_abort_handler	= NULL,				\
	.eh_host_reset_handler	= adapter_reset,		\
	.can_queue		= MAX_QUEUE_SUPPORT,		\
	.this_id		= 16,				\
	.sg_tablesize		= MAX_SCATTER,			\
	.cmd_per_lun		= MAX_CMDLUN,			\
	.use_clustering		= ENABLE_CLUSTERING,		\
	.max_sectors		= MAX_XFER_SECTORS		\
}
#define PCI_DEVICE_ID_TABLE 						\
{									\
	{ PCI_DEVICE(PCI_VENDOR_ID_ARTOP, ATP8620_DEVID)	},	\
	{ 0, },								\
}
#endif
//===========================================================================
#if __LITTLE_ENDIAN
typedef struct _U64C
{
	u8		U64C_L_LL;
	u8		U64C_L_LH;
	u8		U64C_L_HL;
	u8		U64C_L_HH;
	u8		U64C_H_LL;
	u8		U64C_H_LH;
	u8		U64C_H_HL;
	u8		U64C_H_HH;
}T_U64C,*P_U64C;
typedef struct _U64I
{
	u16		U64I_LL;
	u16		U64I_LH;
	u16		U64I_HL;
	u16		U64I_HH;
}T_U64I,*P_U64I;
typedef struct _U64L
{
	u32		U64L_L;
	u32		U64L_H;
}T_U64L,*P_U64L;
#else
typedef struct _U64C
{
	u8		U64C_H_HH;
	u8		U64C_H_HL;
	u8		U64C_H_LH;
	u8		U64C_H_LL;
	u8		U64C_L_HH;
	u8		U64C_L_HL;
	u8		U64C_L_LH;
	u8		U64C_L_LL;
}T_U64C,*P_U64C;
typedef struct _U64I
{
	u16		U64I_HH;
	u16		U64I_HL;
	u16		U64I_LH;
	u16		U64I_LL;
}T_U64I,*P_U64I;
typedef struct _U64L
{
	u32		U64L_H;
	u32		U64L_L;
}T_U64L,*P_U64L;
#endif
typedef union _U64
{
	T_U64L		U64L;
	T_U64I		U64I;
	T_U64C		U64C;
} T_U64,*P_U64;
//===========================================================================
typedef struct _MailBox 
{
	unsigned int	BufAddress;
	unsigned int	BufAddressU;
	unsigned int	Reserved;
	unsigned int	DataLength;
} T_MailBox,*P_MailBox;
//========================================================================
//===========================================================================
typedef struct _Host2DeviceFIS
{
	unsigned char	FIS_Type;
#if __LITTLE_ENDIAN
	unsigned char	PortMultiplier:4;
	unsigned char	FG_Reserved:3;
	unsigned char	FIS_Flag:1;
#else
	unsigned char	FIS_Flag:1;
	unsigned char	FG_Reserved:3;
	unsigned char	PortMultiplier:4;
#endif
	unsigned char	Command;
	unsigned char	P1x1;
	unsigned char	P1x3;
	unsigned char	P1x4;
	unsigned char	P1x5;
	unsigned char	P1x6;
	unsigned char	P1x3Exp;
	unsigned char	P1x4Exp;
	unsigned char	P1x5Exp;
	unsigned char	P1x1Exp;
	unsigned char	P1x2;
	unsigned char	P1x2Exp;
	unsigned char	Reserved1;
	unsigned char	P3x6;
	unsigned int	Reserved2[12];
}T_Host2DeviceFIS,*P_Host2DeviceFIS;
//===========================================================================
typedef struct _CommandTable
{
	T_Host2DeviceFIS	CommandFIS;		// 64  bytes
	unsigned char	AtapiCommnad[32];		// 32  bytes
	unsigned char	Reserved[32];		// 64  bytes
	T_MailBox	MailBox[24];		// 384 bytes
}T_CommandTable,*P_CommandTable;			// 512 bytes
//===========================================================================
typedef struct _CommandHead
{
#if __LITTLE_ENDIAN
	u8		CommandFISLen:5;
	u8		AtapiPioFIS:1;
	u8		ReadWrite:1;
	u8		PreFetchAble:1;
	u8		Reset:1;
	u8		Bist:1;
	u8		ClearBusy:1;
	u8		Reserved0	:1;
	u8		PortMultiplier:4;
#else	
	u8		PreFetchAble:1;
	u8		ReadWrite:1;
	u8		AtapiPioFIS:1;
	u8		CommandFISLen:5;
	u8		PortMultiplier:4;
	u8		Reserved0:1;
	u8		ClearBusy:1;
	u8		Bist:1;
	u8		Reset:1;
#endif	
	u16		PRDTL;
	u32		PRD_ByteCount;
	u32		CommandTablePtr;
	u32		CommandTablePtrU;
	u32		Reserved1[4];
}T_CommandHead,*P_CommandHead;
//===========================================================================
typedef struct _CommandPtr
{
	P_CommandHead	pCommandHead;
	P_CommandTable	pCommandTable;
	dma_addr_t	pDmaCommandTable;
}T_CommandPtr,*P_CommandPtr;
//===========================================================================
typedef struct _DmaSetupFIS
{
	unsigned char	FIS_Type;
	unsigned char	FIS_Flag;
	unsigned short	Reserved0;
	unsigned int	DmaBufferIdentifierL;
	unsigned int	DmaBufferIdentifierH;
	unsigned int	Reserved1;
	unsigned int	DmaBufferOffset;
	unsigned int	DmaTransferCount;
	unsigned int	Reserved2;
} T_DmaSetupFIS,*P_DmaSetupFIS;
//===========================================================================
typedef struct _PioSetupFIS
{
	unsigned char	FIS_Type;
	unsigned char	FIS_Flag;
	unsigned char	Status;
	unsigned char	Error;
	unsigned char	P1x3;
	unsigned char	P1x4;
	unsigned char	P1x5;
	unsigned char	P1x6;
	unsigned char	P1x3Exp;
	unsigned char	P1x4Exp;
	unsigned char	P1x5Exp;
	unsigned char	Reserved0;
	unsigned char	P1x2;
	unsigned char	P1x2Exp;
	unsigned char	Reserved1;
	unsigned char	E_Status;
	unsigned short	TransferCount;
	unsigned short	Reserved2;
} T_PioSetupFIS,*P_PioSetupFIS;
//===========================================================================
typedef struct _Device2Host
{
	unsigned char	FIS_Type;
	unsigned char	FIS_Flag;
	unsigned char	Status;
	unsigned char	Error;
	unsigned char	P1x3;
	unsigned char	P1x4;
	unsigned char	P1x5;
	unsigned char	P1x6;
	unsigned char	P1x3Exp;
	unsigned char	P1x4Exp;
	unsigned char	P1x5Exp;
	unsigned char	Reserved0;
	unsigned char	P1x2;
	unsigned char	P1x2Exp;
	unsigned short	Reserved1;
	unsigned int	Reserved2;
}T_Device2Host,*P_Device2Host;
//===========================================================================
typedef struct _SetDeviceBitsFIS
{
	unsigned char	FIS_Type;
	unsigned char	PortMultiplier:4;
	unsigned char	FG_Interrupt:4;
	unsigned char	Status;
	unsigned char	Error;
	unsigned int	CompleteMap;
}T_SetDeviceBitsFIS,*P_SetDeviceBitsFIS;
//===========================================================================
typedef struct _ReciveFIS
{
	T_DmaSetupFIS	DmaSetupFIS;		//  28 bytes
	unsigned int	Reserved0;		//   4 bytes
	T_PioSetupFIS	PioSetupFIS;		//  20 bytes
	unsigned int	Reserved1[3];		//  12 bytes
	T_Device2Host	Device2HostFIS;		//  20bytes
	unsigned int	Reserved2;		//   4 bytes
	T_SetDeviceBitsFIS SetDeviceBitsFIS;	//   8 bytes
	unsigned char	Reserved3[32];		//  32 bytes
} T_ReciveFIS,*P_ReciveFIS;				// 128 bytes //256 bytes
//========================================================================
typedef struct _WorkingTable
{
	void		*pNextTable;
	void		*pPrevTable;
	void		*pDevice;
	Scsi_Cmnd 	*pWorkingScsiCmd;
	unsigned long	IoTagID;
	P_MailBox	pMailBox;
	unsigned int	LmailBox;
	unsigned int	LmailBoxX;
	unsigned int	pDmaSingleMapAddr;
	unsigned int	sg_num;
	unsigned int	PriorityLevel;
	T_U64		StartLBA;
	T_U64		TransferLen;
	T_U64		StartLBAOrg;
	T_U64		TransferLenOrg;
	unsigned char	FG_TableActive:1;
	unsigned char	FG_DmaToDevice:1;
	unsigned char	FG_DoIoByDMA:1;
	unsigned char	FG_Updata:1;
	unsigned char	FG_NonData:1;
	unsigned char	FG_Reserved:3;
	unsigned char	TagID;
	unsigned char	SenseKey;
	unsigned char	SenseAsc;
	unsigned char	SenseAscQ;
	unsigned char	XferCmd;
	void *		request_buffer;
	unsigned long	request_bufflen;
	// FOLLOWING IS FOR CDROM =============================
	unsigned char	*pDataBufPtr;
	unsigned long	DataCounter;
	unsigned long	sg_offset;
	unsigned long	sg_data_offset;
	unsigned char	DataBuffer[932];
	unsigned char	OrgCmnd[16];
	void		*pOrgRequesrBuff;
	unsigned long	pOrgRequestBuffLen;
	unsigned long	Length;
	unsigned long	use_sg;
	unsigned long	old_use_sg;
	unsigned char 	Rev[2];
} T_WorkingTable,*P_WorkingTable;// this size should be equal to 4n bytes
//========================================================================
typedef struct _Channel
{
	//===========================================
	unsigned int	MaxDeviceCount:4;
	unsigned int	FG_PortMultiplier:1;
	unsigned int	FG_StopIO:1;
	unsigned int	FG_SataChannel:1;
	unsigned int	FG_Reserved:23;
	unsigned int	FG_ErrD2H:1;
	unsigned int	FG_ErrSDB:1;
	//===========================================
	unsigned int	IdePortP1XX;
	unsigned int	IdePortPSFF;
	unsigned int	IdePortP3XX;
	unsigned int	IdeRegPrdTable;
	unsigned int	IdeRegSpeed;
	P_WorkingTable	pCurRequest;
	//===========================================
	unsigned int	BaseIoPort;
	P_ReciveFIS	pReciveFIS;
	T_CommandPtr	CommandPtr;
	P_CommandPtr	pCommandPtr;
	P_WorkingTable	pPendingIO;
	P_WorkingTable	pExecuteIO[MAX_COMMAND_QUEUE_CH];
	unsigned char	QueueCount;
	unsigned char	QueueStartTag;
	unsigned char	ChannelOrder;
	unsigned char	FG_ErrPMx;
	unsigned char	Update;
	unsigned char	rev[3];
} T_Channel,*P_Channel;
//========================================================================
typedef struct _DeviceInfo
{
	unsigned char	FG_DeviceInstall:1;
	unsigned char	FG_Support48Bits:1;
	unsigned char	FG_QueueSupported:1;
	unsigned char	FG_SerialATA:1;
	unsigned char	FG_AtapiDevice:1;
	unsigned char	FG_DiscDevice:1;
	unsigned char	FG_OnSataChannel:1;
	unsigned char	FG_InternalError:1;
	unsigned char	HD_V1X6;
	unsigned char	MultiBlk;
	unsigned char	PIOxferclk;
	unsigned char	PIO_Mode;
	unsigned char	DMAxferclk;
	unsigned char	DMA_Mode;
	unsigned char	FG_IdeExcept;
	unsigned char	SenseKey;
	unsigned char	SenseAsc;
	unsigned char	SenseAscQ;
	unsigned char	FG_PMId:4;
	unsigned char	FG_Reserved1:4;
	unsigned char	MaxQueueDepth;
	unsigned char	QueueCount;
	unsigned char	HddType; /*fixed by gcc 2008-09-03 to support multiword dma*/
	unsigned char	Reserved1;
	unsigned short	A0Wait;
	unsigned short	DeviceBlockSize;
	T_U64		Capacity;
//	unsigned int	Capacity_L;
//	unsigned int	Capacity_H;
	P_Channel	pChannel;
	unsigned char	ModelName[40];
	unsigned char	FirmWareRev[8];
	unsigned char	SerialNo[20];
} T_DeviceInfo,*P_DeviceInfo;
//========================================================================
typedef struct _HW_DEVICE_EXTENSION 
{
	unsigned short	VendorID;
	unsigned short	DeviceID;
	unsigned short	SubVendorID;
	unsigned short	SubDeviceID;
	unsigned char	ChipRevision;
	unsigned char	IrqNum;
	unsigned int	BaseIoPort0;
	unsigned int	BaseIoPort1;
	unsigned int	BaseIoPort2;
	spinlock_t 	spin_lock;
	struct Scsi_Host *pScsiHost;
	struct pci_dev	*pPciDev;
	struct timer_list AdapterTimer;
	//====================================================================
	T_Channel	IdeChannel;
	T_Channel	SataChannel[MAX_SATA_CHANNEL];
	T_DeviceInfo	DeviceInfo[(MAX_SATA_CHANNEL * MAX_SATA_HDD) + 2];
	//====================================================================
	P_WorkingTable	pWorkingTablePool;
	unsigned char	*pSataChCmdTable[2];
	dma_addr_t	pDmaSataChCmdTable[2];
	unsigned char	*pUncachedExtension;	//Cmd Head+RcvFIS+IdeMailBox+IdentifyBuff
	dma_addr_t	pDmaUncachedExtension;
	unsigned char	*pIdentifyBuffer;
	dma_addr_t	pDmaIdentifyBuffer;
	P_MailBox	pIdeMailBox;
	dma_addr_t	pDmaIdeMailBox;
	unsigned int	IoTagID;
	unsigned int	IoCounter;
	unsigned char	FG_TimerStop;
	unsigned char	FG_ExtensionAllocate;
	Scsi_Cmnd 	*pScsiCmd; // for I2C use
	unsigned char	I2C_STEP;   //for I2C use
	unsigned char	Reserved2;
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;
//========================================================================
typedef struct _IdentifyATADevice 
{				// WORD
   unsigned short	ConfigBitInfo;	// 00
   unsigned short	CylinderNo;	// 01
   unsigned short	Reserved0;	// 02
   unsigned short	HeadNo;		// 03
   unsigned short	Reserved1;	// 04
   unsigned short	Reserved2;	// 05
   unsigned short	SectorNo;	// 06
   unsigned short	Reserved3;	// 07
   unsigned short	Reserved4;	// 08
   unsigned short	Reserved5;	// 09
   unsigned char		SerialNo[20];	// 10-19
   unsigned short	Reserved6;	// 20
   unsigned short	Reserved7;	// 21
   unsigned short	Reserved8;	// 22
   unsigned char		FirmWareRev[8];	// 23-26
   unsigned char		ModelName[40];	// 27-46
#if __LITTLE_ENDIAN
   unsigned char		MultipleBlk;	// 47(LOW BYTE)
   unsigned char		Reserved9;	// 47(HIGH BYTE = 0x80)
#else
   unsigned char		Reserved9;	// 47(HIGH BYTE = 0x80)
   unsigned char		MultipleBlk;	// 47(LOW BYTE)
   
#endif
   unsigned short	ReservedA;	// 48
   unsigned short	Capability;	// 49
   unsigned short	Security;	// 50
#if __LITTLE_ENDIAN
   unsigned char		ReservedB;	// 51(LOW BYTE)
   unsigned char		PIOMode;		// 51(HIGH BYTE)
   unsigned char		ReservedC;		// 52(LOW BYTE)
   unsigned char		DMAMode;		// 52(HIGH BYTE)
#else
   unsigned char		PIOMode;		// 51(HIGH BYTE)
   unsigned char		ReservedB;	// 51(LOW BYTE)
   unsigned char		DMAMode;		// 52(HIGH BYTE)
   unsigned char		ReservedC;		// 52(LOW BYTE)
#endif
   unsigned short	FG_ValidMap;		// 53
//   USHORT FG_Valid5458:1;	// 53
//   USHORT FG_Valid6470:1;
//   USHORT FG_Valid88: 1;
//   USHORT FG_Vendor: 13;
   unsigned short	Logical_CHS[3];		// 54 - 56
   unsigned short	SectorCapacity0;	// 57
   unsigned short	SectorCapacity1;		// 58
   unsigned short	CurrRWMultiCommand;	// 59
   unsigned int		TotalLBA;		// 60 - 61
#if __LITTLE_ENDIAN
   unsigned char		SingleWordDMASupport;	// 62 (LOW BYTE)
   unsigned char		SingleWordDMAActive;	// 62 (HIGH BYTE)
   unsigned char		MultiWordDMASupport;	// 63 (LOW BYTE)	
   unsigned char		MultiWordDMAActive;	// 63 (HIGH BYTE)
   unsigned char		AdvancePIOSupport;	// 64 (LOW BYTE)
   unsigned char		ReservedD;		// 64 (HIGH BYTE)
#else
   unsigned char		SingleWordDMAActive;	// 62 (HIGH BYTE)
   unsigned char		SingleWordDMASupport;	// 62 (LOW BYTE)
   unsigned char		MultiWordDMAActive;	// 63 (HIGH BYTE)
   unsigned char		MultiWordDMASupport;	// 63 (LOW BYTE)	
   unsigned char		ReservedD;		// 64 (HIGH BYTE)
   unsigned char		AdvancePIOSupport;	// 64 (LOW BYTE)
#endif
   unsigned short	MinMDMATransCycle;	// 65
   unsigned short	ReCmdMinMDMATransCycle;	// 66
   unsigned short	ReservedE;		// 67
   unsigned short	MinPioTransCycle;	// 68
   unsigned short	ResveredF[6];		// 69-74
   unsigned short	QueueDepth;		// 75
#if __LITTLE_ENDIAN			// 76
   unsigned short	FG_Reserved76_0:1;
   unsigned short	FG_SATA1_support:1;
   unsigned short	FG_SATA2_support:1;
   unsigned short	FG_Reserved76_47:5;
   unsigned short	FG_NcqSupport:1;
   unsigned short	FG_Reserved76_9F:7;
#else
   unsigned char		FG_Reserved76_9F:7;
   unsigned char		FG_NcqSupport:1;
   unsigned char		FG_Reserved76_47:5;
   unsigned char		FG_SATA2_support:1;
   unsigned char		FG_SATA1_support:1;
   unsigned char		FG_Reserved76_0:1;
#endif
   unsigned short	ReservedG[6];		// 77-82
   unsigned short	Word83;			// 83
   unsigned short	ReservedH[4];		// 84-87
   unsigned short	UltraModeMap;		// 88
   unsigned short	ReservedI[4];		// 89 - 92
   unsigned short	Word93;			// 93
//   USHORT Word93BitIgnore : 13;	// 93 bit 0  - bit 12
//   USHORT Cable80pin:1;		// 93 bit 13 80 PDIG-
//   USHORT Word93Remain:2;	// 93 bit 14 - bit 15
   unsigned short	ReservedJ[6];		// 94 - 99
   unsigned int		LBA_48bits_Low;	// 100 - 101
   unsigned int		LBA_48bits_High;	// 102 - 103
   unsigned short	ReservedK[200];		// 104 - 255
} UIdentify, *PUIdentify;
//========================================================================
typedef struct _UINQUIRYDATA {
   unsigned char DeviceType;
   unsigned char RemovableFlag;//bit7 removedable
   unsigned char VersionInfo;  // bit0-2:ANSI version,bit3-5:ECMA version,bit6-7:ISO version
   unsigned char ResponseDataFormat;
   unsigned char AdditionalLength;
   unsigned char Reserved[2];
   unsigned char Flag;  //bit0:SoftReset,bit1:Commnad Queue,bit5:wide 16bits,bit6:wide 32bits 
   unsigned char VendorId[8];
   unsigned char ProductId[16];
   unsigned char ProductRevisionLevel[4];
   unsigned char VendorSpecific[20];
   unsigned char Reserved3[40];
} UINQUIRYDATA, *PUINQUIRYDATA;
//========================================================================
static int AllocateExtensionMemory(PHW_DEVICE_EXTENSION pHwDeviceExtension);
static void ReleaseExtensionMemory(PHW_DEVICE_EXTENSION pHwDeviceExtension);
//========================================================================
static unsigned char SendIdentifyCmd2IDE(P_DeviceInfo pDeviceInfo,void *pBuffer,unsigned char IdentifyCmd);
static void SetPIOfeature(P_DeviceInfo pHardDiskInfo,PUIdentify pIdentifyBuff,unsigned char MAXMODE);
static void GetUltraMode(P_DeviceInfo pHardDiskInfo,unsigned char UltraMap);
static void SetUltra(P_DeviceInfo pHardDiskInfo,unsigned char MAXMODE);
static void SetUltrafeature(P_DeviceInfo pHardDiskInfo,PUIdentify pIdentifyBuff,unsigned char MAXMODE);
static void SetDMAfeature(P_DeviceInfo pHardDiskInfo,PUIdentify PidentifyBuff,unsigned char MAXMODE);
static void SetChipIo(P_DeviceInfo pHardDiskInfo);
static void ScanATA(PHW_DEVICE_EXTENSION DeviceExtension,P_DeviceInfo pHardDiskInfo);
static void ExecuteIdeATA(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pIdeChannel);
static void HardDiskInterrupt(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pIdeChannel);
static void ScanATAPI(PHW_DEVICE_EXTENSION DeviceExtension,P_DeviceInfo pHardDiskInfo);
static void ExecuteIdeATAPI(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pIdeChannel);
static void DoPioATAPIXfer(P_WorkingTable pWorkingTable);
static void DoDmaATAPIXfer(P_WorkingTable pWorkingTable);
static void CdRomInterrupt(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pIdeChannel);
//========================================================================
//========================================================================
static int SoftwareReset(P_Channel pSataChannel,unsigned char PMx);
static int PMxReset(P_Channel pSataChannel,unsigned char PMx);
static void PortMultiplierEnumerate(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pSataChannel,P_DeviceInfo pDeviceInfo);
static void SendIdentifyCommand(PHW_DEVICE_EXTENSION DeviceExtension,P_DeviceInfo pDeviceInfo,int PMx,unsigned char IdenCmd);
static void ExecuteSataATA(PHW_DEVICE_EXTENSION DeviceExtension,P_WorkingTable pWorkingTable);
static void ExecuteSataATAPI(PHW_DEVICE_EXTENSION DeviceExtension,P_WorkingTable pWorkingTable);
//========================================================================
//========================================================================
static void BuildPrdTable(PHW_DEVICE_EXTENSION DeviceExtension,P_WorkingTable pWorkingTable,P_MailBox pMailBox);
static P_WorkingTable AllocateWorkingTable(PHW_DEVICE_EXTENSION DeviceExtension);
static void ReleaseWorkingTable(P_WorkingTable pWorkingTable);
static void ChannelIoHook(P_WorkingTable pWorkingTable);
static void ChannelIoRemove(P_WorkingTable pWorkingTable);
static void ConvertCommandATA(P_WorkingTable pWorkingTable);
static void ConvertCmdATAPI(P_WorkingTable pWorkingTable);
static void RestoreCmdATAPI(P_WorkingTable pWorkingTable);
static void ConvertSenseKey(P_Channel pChannel,P_WorkingTable pWorkingTable,unsigned char status);
//========================================================================
//========================================================================
static void AddU64(P_U64 Src64_1,P_U64 Src64_2,P_U64 Dest64);
static int  CmpU64(P_U64 Src64_1,P_U64 Src64_2);
static void mDelay(int interval);
static void Debug80Port(unsigned char code,int delaytime);
//========================================================================
//===================updata fireware========================================
static void ExecuteUpdata(PHW_DEVICE_EXTENSION DeviceExtension,P_WorkingTable pWorkingTable);
//========================================================================
