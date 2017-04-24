/* $ Id: ATP8620.C , v 0.10 2006/11/14 11:25:00 $
 *
 *  Copyright (C) 2005 ACARD Technology Corp.
*********************************************************
          modify history

*********************************************************
*/
#define NCQ_SUPPORT	1
#define DEBUG_80_PORT	0
#define DISK_NCQ_CHECK	0
#include <linux/module.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/system.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <linux/blk.h>
#include "scsi.h"
#include "hosts.h"
#include "sd.h"
#else
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/pci.h>
#include <linux/blkdev.h>
#include <linux/dma-mapping.h>
#include <linux/stat.h>
#include <linux/list.h>
#include <scsi/scsi_host.h>
#include <asm/system.h>
#include <asm/io.h>
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#endif
#include <linux/stat.h>
#ifndef IRQF_SHARED
	#define IRQF_SHARED SA_SHIRQ
#endif
#ifndef DMA_32BIT_MASK
	#define DMA_32BIT_MASK 0xFFFFFFFFUL
#endif
#include "atp8620.h"
static int oldide=0;
static int __devinit atp8620_init_one(struct pci_dev *dev);

// following function is implement necessary
static int  adapter_initialize(struct pci_dev *pPciDev);
static void adapter_release_resource(void *pScsiHostData);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static int  adapter_reset(Scsi_Cmnd *pScsiCmd, unsigned int reset_flags);
static void adapter_intr_handle(int irq, void *dev_id, struct pt_regs *regs);
#else
static int  adapter_reset(Scsi_Cmnd *pScsiCmd);
static irqreturn_t adapter_intr_handle(int irq, void *dev_id);
#endif
static int  adapter_queuecommand(Scsi_Cmnd *pScsiCmd, void (*done) (Scsi_Cmnd *));
static void adapter_timer(unsigned long para);
//========================================================================
//=== scsidrv.c ============================================================
//========================================================================
static int adapter_probe(struct pci_dev *pPciDev, const struct pci_device_id *ent);
static void adapter_remove (struct pci_dev *pPciDev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static int adapter_biosparam(Scsi_Disk * disk, kdev_t dev, int *ip);

static int adapter_proc_info(char *buffer, char **start, off_t offset, int length,
                             int hostno, int inout);
static int adapter_detect(Scsi_Host_Template *tpnt);
static int adapter_release (struct Scsi_Host *pScsiHost);
#else	
static int adapter_biosparam(struct scsi_device *disk, struct block_device *dev,
                             sector_t capacity, int *ip);
static int adapter_proc_info(struct Scsi_Host *HBAptr, char *buffer, char **start,
                             off_t offset, int length, int inout);
#endif
static int adapter_set_info(char *buffer, int length, struct Scsi_Host *HBAptr);
static const char *adapter_info(struct Scsi_Host *notused);
//========================================================================
typedef struct _ScsiHostData {
	struct pci_dev *pPciDev;
	struct Scsi_Host *pScsiHost;
	void *DeviceExtension;
	unsigned int szDeviceExtension;
} T_ScsiHostData,*P_ScsiHostData;
//========================================================================
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	static Scsi_Host_Template driver_template = SCSI_DRV_TEMPLATE;
	static struct Scsi_Host *AdapterHost[MAX_ADAPTER];
	static int AdapterCounter;
#else
	static struct scsi_host_template  driver_template = SCSI_DRV_TEMPLATE;
#endif
static struct pci_device_id device_id_table[] = PCI_DEVICE_ID_TABLE;
//========================================================================
static struct pci_driver adapter_driver = {
	.id_table	= device_id_table,
	.name	= DRIVER_NAME,
	.probe	= adapter_probe,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	.remove	= adapter_remove,
#else
	.remove	= __devexit_p(adapter_remove),
#endif
};
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
//void msleep(int mdelay_time)
//{
//	int i;
//	
//	for (i = 0 ; i < mdelay_time ; i++) {
//		udelay(1000);
//	}
//	return;
//}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
#define sg_page(sg) ((sg)->page)
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,24)
#define KERNEL_V2625
#else
#define scsi_sg_count(cmd) ((cmd)->use_sg)
#endif
//========================================================================
static int adapter_probe(struct pci_dev *pPciDev, const struct pci_device_id *ent)
{
	int init_status;
	struct Scsi_Host *pScsiHost;

	if (pci_enable_device(pPciDev)) {
		printk("adapter_probe:PCI device can not enable\n\r");
		return -EIO;
	}
        if (pci_set_dma_mask(pPciDev, DMA_32BIT_MASK)) {
		printk("adapter_probe DMA ERROR\n\r");
                return -EIO;
        }
	if (pPciDev->irq == 0) {
		printk("adapter_probe:IRQ incorrecte\n\r");
		return -EIO;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	if (!(pScsiHost = scsi_register(&driver_template, sizeof(T_ScsiHostData)))) {
#else
	if (!(pScsiHost = scsi_host_alloc(&driver_template, sizeof(T_ScsiHostData)))) {
#endif
		printk("adapter_probe:SCSI host allocate fail\n\r");
		return -ENOMEM;
	}
	memset(pScsiHost->hostdata,0,(int)sizeof(T_ScsiHostData));  // clear already in kernel 2.6
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#else
	if (scsi_add_host(pScsiHost, &pPciDev->dev)) {
		printk("adapter_probe:SCSI host add fail\n\r");
		scsi_host_put(pScsiHost);
		return -EIO;
	}
#endif
	((P_ScsiHostData)(pScsiHost->hostdata))->pPciDev = pPciDev;
	((P_ScsiHostData)(pScsiHost->hostdata))->pScsiHost = pScsiHost;
	((P_ScsiHostData)(pScsiHost->hostdata))->DeviceExtension = NULL;
	((P_ScsiHostData)(pScsiHost->hostdata))->szDeviceExtension = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	AdapterHost[AdapterCounter] = pScsiHost;
	AdapterCounter++;
#else
#endif
	pci_set_drvdata(pPciDev,pScsiHost->hostdata);
	if ((init_status = adapter_initialize(pPciDev))!= 0) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		scsi_unregister(pScsiHost);
		AdapterCounter--;
		AdapterHost[AdapterCounter] = NULL;
#else
		scsi_remove_host(pScsiHost);
		scsi_host_put(pScsiHost);
#endif
		printk("adapter_probe:SCSI adapter initialize fail\n\r");
		return init_status;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#else
	scsi_scan_host(pScsiHost);
#endif

if(oldide)
{
PHW_DEVICE_EXTENSION pHwDeviceExtension = ((P_ScsiHostData)(pScsiHost->hostdata))->DeviceExtension;
//	if(!pHwDeviceExtension->DeviceInfo[0].FG_DeviceInstall && !pHwDeviceExtension->DeviceInfo[0].FG_DeviceInstall)
	atp8620_init_one(pPciDev);
}

	return 0;
}
//========================================================================
static void adapter_remove (struct pci_dev *pPciDev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	pci_set_drvdata(pPciDev, NULL);
	AdapterCounter--;
#else
	P_ScsiHostData pScsiHostData;

	pScsiHostData = pci_get_drvdata(pPciDev);
	scsi_remove_host(pScsiHostData->pScsiHost);
	adapter_release_resource(pScsiHostData);
	scsi_host_put(pScsiHostData->pScsiHost);
	pci_set_drvdata(pPciDev, NULL);
#endif
	return;
}
//========================================================================
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)

#include "scsi_module.c"
//========================================================================
static int adapter_detect(Scsi_Host_Template * tpnt)
{
	int i;
	
	for (i = 0 ; i < MAX_ADAPTER ; i++) {
		AdapterHost[i] = NULL;
	}
	AdapterCounter = 0;
	if (pci_module_init(&adapter_driver) == 0) {
		// ADAPTER INITIALIZE SUCCESSFUL
		return AdapterCounter;
	}
	return 0;
}
//========================================================================
static int adapter_release (struct Scsi_Host *pScsiHost)
{
	
	if (AdapterCounter > 0) {
		pci_unregister_driver(&adapter_driver);
	}
	adapter_release_resource(pScsiHost->hostdata);
	scsi_unregister(pScsiHost);
	return 0;
}
//========================================================================
#else
//========================================================================
static int __init adapter_init(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	int i;
	
	for (i = 0 ; i < MAX_ADAPTER ; i++) {
		AdapterHost[i] = NULL;
	}
	AdapterCounter = 0;
#endif
	return pci_register_driver(&adapter_driver);
}
//========================================================================
static void __exit adapter_exit(void)
{
	pci_unregister_driver(&adapter_driver);
	return;
}
//========================================================================
module_init(adapter_init);
module_exit(adapter_exit);
//========================================================================
#endif
//========================================================================
MODULE_DEVICE_TABLE(pci, device_id_table);
//========================================================================
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static int adapter_biosparam(Scsi_Disk * disk, kdev_t dev, int *ip)
{
	unsigned long head,sector,cylinder,hdd_capacity;
	
	hdd_capacity = (unsigned long)(disk->capacity);
#else
static int adapter_biosparam(struct scsi_device *disk, struct block_device *dev,
						  sector_t capacity, int *ip)
{
	unsigned long head,sector,cylinder,hdd_capacity;

	hdd_capacity = (unsigned long)(capacity);
#endif
	head     = 64;
	sector   = 32;
	cylinder = hdd_capacity / (head * sector);
	if (cylinder > 1024) {
		head     = 255;
		sector   = 63;
		cylinder = hdd_capacity / (head * sector);
	}
	ip[0] = (int)head;
	ip[1] = (int)sector;
	ip[2] = (int)cylinder;
	return 0;
}
//========================================================================
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static int adapter_proc_info(char *buffer, char **start, off_t offset, int length,
		      int hostno, int inout)
{
	struct Scsi_Host *pScsiHost;
	static u8 buff[512];
	int i,len = 0;

	pScsiHost = NULL;
	for (i = 0; i < MAX_ADAPTER; i++) {
		if ((pScsiHost = AdapterHost[i]) != NULL) {
			if (pScsiHost->host_no == hostno) {
				break;
			}
			pScsiHost = NULL;
		}
	}
	if (pScsiHost == NULL) {
		len += sprintf(buffer, "Can't find adapter for host number %d\n\r", hostno);
		*start = buffer + offset;	// Start of wanted data 
		len -= offset;		// Start slop
		if (len > length) {
			len = length;	// Ending slop
		}
		return (len);
	}
#else	
static int adapter_proc_info(struct Scsi_Host *pScsiHost, char *buffer,
			char **start, off_t offset, int length, int inout)
{
	static u8 buff[512];
	int len = 0;
	
#endif
	if (inout != 0) {
		//Has data been written to the file?
		return (adapter_set_info(buffer, length, pScsiHost));
	}
	if (offset == 0) {
		memset(buff, 0, sizeof(buff));
	}
	len += sprintf(buffer, "%s Version: %d.%d.%d",PROC_INFORMATION,DRV_VERSION_MAJOR,DRV_VERSION_MINOR,DRV_VERSION_BUILD);
	*start = buffer + offset;	// Start of wanted data
	len -= offset;		// Start slop
	if (len > length) {
		len = length;	// Ending slop
	}
	return (len);
}
//========================================================================
static int adapter_set_info(char *buffer, int length, struct Scsi_Host *HBAptr)
{
	return -EINVAL; 	// Currently this is a no-op
}
//========================================================================
static const char *adapter_info(struct Scsi_Host *notused)
{
	static char buffer[128];

	sprintf(buffer,"%s Version:%d.%d.%d",ADAPTER_INFORMATION,DRV_VERSION_MAJOR,DRV_VERSION_MINOR,DRV_VERSION_BUILD);
	return buffer;
}
//========================================================================
//========================================================================
//========================================================================
MODULE_AUTHOR("Johnny Weng");
MODULE_DESCRIPTION("ACARD ATP8620 LINUX SCSI Driver");
MODULE_LICENSE("GPL");
//========================================================================
static unsigned char ModePage0[16] = {15,0,0,8,0,0,0,0,0,0,2,0,0,0,0,0};
static unsigned char ModePage1[24] = {23,0,0,8,0,0,0,0,0,0,2,0,0x01,0x0a,0xc0,0x08,0x18,0x0a,0,0,0,0,0,0};
static unsigned char ModePage8[24] = {23,0,0,8,0,0,0,0,0,0,2,0,0x08,0x0a,0x04,0,0xff,0xff,0,0,3,0xd7,0xff,0xff};
static unsigned char ModePage1c[24] = {23,0,0,8,0,0,0,0,0,0,2,0,0x1c,0x0a,0,3,0,0,0,0,0,0,0,1};
static unsigned char ModePage0DBD[8] = {7,0,0,0,0,0,0,0};
static unsigned char ModePage1DBD[16] = {15,0,0,0,0x01,0x0a,0xc0,0x08,0x18,0x0a,0,0,0,0,0,0};
static unsigned char ModePage8DBD[16] = {15,0,0,0,0x18,0x0a,0x04,0,0xff,0xff,0,0,3,0xd7,0xff,0xff};
static unsigned char ModePage1cDBD[16] = {15,0,0,0,0x1c,0x0a,0,3,0,0,0,0,0,0,0,1};
//static unsigned char AutoSenseCdb[6] = {3,0,0,0,14,0};
//========================================================================
static int adapter_initialize(struct pci_dev *pPciDev)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension;
	P_ScsiHostData pScsiHostData;
	struct Scsi_Host *pScsiHost;
	P_Channel pChannel;
	P_DeviceInfo pDeviceInfo;
	unsigned char *pUncachedExtension;
	dma_addr_t pDmaUncachedExtension;
	dma_addr_t pDmaCommandTable;
	P_CommandHead pCommandHead;
	P_CommandTable pCommandTable;
	unsigned int base_io,base_io1,base_io2;
	unsigned int signature;
	int i,j,status;
	unsigned char statusX;

	status        = 0;
	pScsiHostData = pci_get_drvdata(pPciDev);
	pScsiHost     = pScsiHostData->pScsiHost;
	if (  (WORKINGTABLE_MEM_LEN/sizeof(T_WorkingTable)) < MAX_QUEUE_SUPPORT ) {
		printk("Queue pool(%d) too small \n\r",(int)(WORKINGTABLE_MEM_LEN / sizeof(T_WorkingTable)) );
		return -ENOMEM;
	}
	//=======================================================================================
	// allocate memory
	//=======================================================================================
	pHwDeviceExtension = (PHW_DEVICE_EXTENSION)__get_free_pages(GFP_ATOMIC,EXTENSION_MEM);
	if (pHwDeviceExtension == NULL) {
		printk("adapter_initialize:Unable to pHwDeviceExtension memory.\n\r");
		return -ENOMEM;
	}
	memset(pHwDeviceExtension,0,EXTENSION_MEM_LEN);
	base_io  = (unsigned int)(pci_resource_start(pPciDev, 0));	// get base IO 
	base_io  &= 0xFFFFFFFE;
	base_io1 = (unsigned int)(pci_resource_start(pPciDev, 1));	// get base IO 1
	base_io1 &= 0xFFFFFFFE;
	base_io2 = (unsigned int)(pci_resource_start(pPciDev, 2));	// get base IO 2
	base_io2 &= 0xFFFFFFFE;
	//=======================================================================================
	// initialize Device Extension
	//=======================================================================================
	pci_read_config_word(pPciDev, 0x00,&(pHwDeviceExtension->VendorID));
	pci_read_config_word(pPciDev, 0x02,&(pHwDeviceExtension->DeviceID));
	pci_read_config_word(pPciDev, 0x2C,&(pHwDeviceExtension->SubVendorID));
	pci_read_config_word(pPciDev, 0x2E,&(pHwDeviceExtension->SubDeviceID));
	pci_read_config_byte(pPciDev, 0x08,&(pHwDeviceExtension->ChipRevision));
	pHwDeviceExtension->IrqNum	= pPciDev->irq;
	pHwDeviceExtension->BaseIoPort0	= base_io;
	pHwDeviceExtension->BaseIoPort1	= base_io1;
	pHwDeviceExtension->BaseIoPort2	= base_io2;
	pHwDeviceExtension->spin_lock	= SPIN_LOCK_UNLOCKED;
	pHwDeviceExtension->pScsiHost	= pScsiHost;
	pHwDeviceExtension->pPciDev	= pPciDev;
	pHwDeviceExtension->pWorkingTablePool = (P_WorkingTable)(((unsigned char *)pHwDeviceExtension)+MEM_4KB);
	if (AllocateExtensionMemory(pHwDeviceExtension) == 0) {
		free_pages((unsigned long)pHwDeviceExtension,EXTENSION_MEM);
		printk("adapter_initialize:Unable to pUncachedExtension memory.\n\r");
		return -ENOMEM;
	}
	pScsiHostData->DeviceExtension   = pHwDeviceExtension;
	pScsiHostData->szDeviceExtension = EXTENSION_MEM_LEN;
	//================================================================
	// register IRQ
	//================================================================
	Debug80Port(0xF1,1);
	if (request_irq(pHwDeviceExtension->IrqNum, adapter_intr_handle,
				IRQF_SHARED, DRIVER_NAME, pScsiHost)) {
		printk("adapter_initialize:Unable to allocate IRQ for adapter.\n\r");
		free_pages((unsigned long)pHwDeviceExtension,EXTENSION_MEM);
		return -EIO;
	}
	Debug80Port(0xF2,1);
	//================================================================
	// Initialize host adapter
	//================================================================
#if 0
	if(!request_region(base_io, 0xFF, DRIVER_NAME)) {
		free_irq(pPciDev->irq,pScsiHost);
		printk("adapter_initialize:Unable to allocate Base IO region 0.\n\r");
		return -EIO;
	}
#endif
	if(!request_region(base_io1, 0xFF, DRIVER_NAME)) {
		free_irq(pPciDev->irq,pScsiHost);
		release_region(base_io, 0xFF);
		printk("adapter_initialize:Unable to allocate Base IO region 1.\n\r");
		return -EIO;
	}
	//================================================================
	// Reset Chip
	//================================================================
	outb(0x01,pHwDeviceExtension->BaseIoPort0 + 0x0004);
	while (inb(pHwDeviceExtension->BaseIoPort0 + 0x0004) & 0x01) {
		udelay(10);
		cpu_relax();
	}
	//================================================================
	// Disable INTA#
	//================================================================
	outb(0x00,pHwDeviceExtension->BaseIoPort0 + 0x0004);
	//================================================================
	// Disable IDE to INTA#
	//================================================================
	outb(0x00,pHwDeviceExtension->BaseIoPort0 + 0x0091);
	//================================================================
	// Initialize IDE channel 
	//================================================================
	pChannel			= &(pHwDeviceExtension->IdeChannel);
	pChannel->IdePortP1XX	= pHwDeviceExtension->BaseIoPort0+0x0080;
	pChannel->IdePortP3XX	= pHwDeviceExtension->BaseIoPort0+0x008E;
	pChannel->IdePortPSFF	= pHwDeviceExtension->BaseIoPort0+0x0090;
	pChannel->IdeRegPrdTable	= pHwDeviceExtension->BaseIoPort0+0x0098;
	pChannel->IdeRegSpeed	= pHwDeviceExtension->BaseIoPort0+0x0094;
	pChannel->QueueCount	= 0;
	pChannel->FG_SataChannel	= 0;
	pChannel->MaxDeviceCount	= 2;
	//============================================
	// Initialize IDE Master Device
	//============================================
	pDeviceInfo		= &(pHwDeviceExtension->DeviceInfo[0]);
	pDeviceInfo->HD_V1X6	= 0xA0;
	pDeviceInfo->pChannel	= pChannel;
	pDeviceInfo->FG_SerialATA	= 0;
	//============================================
	// Initialize IDE Slave Device
	//============================================
	pDeviceInfo++;
	pDeviceInfo->HD_V1X6	= 0xB0;
	pDeviceInfo->pChannel	= pChannel;
	pDeviceInfo->FG_SerialATA	= 0;
	//============================================
	// SCAN IDE PHYSICAL HARD DISK
	//============================================
	pDeviceInfo = &(pHwDeviceExtension->DeviceInfo[0]);
	for (i = 0 ; i < pChannel->MaxDeviceCount ; i++) {
		ScanATA(pHwDeviceExtension,pDeviceInfo);
		if (pDeviceInfo->FG_DeviceInstall) {
			SetChipIo(pDeviceInfo);
		} else {
			ScanATAPI(pHwDeviceExtension,pDeviceInfo);
			if (pDeviceInfo->FG_DeviceInstall) {
				SetChipIo(pDeviceInfo);
			}
		}
		pDeviceInfo++;
	}
	//============================================
	// Initialize SATA channel 
	//============================================
	pChannel   = &(pHwDeviceExtension->SataChannel[0]);
	pUncachedExtension = pHwDeviceExtension->pUncachedExtension;
	pDmaUncachedExtension = pHwDeviceExtension->pDmaUncachedExtension;
	for (i = 0 ; i < MAX_SATA_CHANNEL ; i++) {
		pChannel->BaseIoPort			= pHwDeviceExtension->BaseIoPort1 + (i * 0x80);
		pChannel->FG_SataChannel			= 1;
		pChannel->ChannelOrder			= (unsigned char)(i + 1);
		pChannel->pCommandPtr			= &(pChannel->CommandPtr);
		pChannel->pCommandPtr->pCommandHead	= (P_CommandHead)pUncachedExtension;
		pChannel->pReciveFIS			= (P_ReciveFIS)(pUncachedExtension+MEM_512B);
		pChannel->pCommandPtr->pCommandTable	= (P_CommandTable)(pHwDeviceExtension->pSataChCmdTable[i]);
		pChannel->pCommandPtr->pDmaCommandTable	= pHwDeviceExtension->pDmaSataChCmdTable[i];
		pDmaCommandTable				= pChannel->pCommandPtr->pDmaCommandTable;
		//============================================
		// set Command List to chip
		//============================================
		outl(pDmaUncachedExtension,pChannel->BaseIoPort);
		outl(0,pChannel->BaseIoPort + 0x0004);
		//============================================
		// set Recived FIS to chip
		//============================================
		outl((pDmaUncachedExtension + MEM_512B),pChannel->BaseIoPort + 0x0008);
		outl(0,pChannel->BaseIoPort + 0x000C);
		//============================================
		// initialize SATA channel
		//============================================
		outb(0x01,pChannel->BaseIoPort + 0x002C);
		outb(0x00,pChannel->BaseIoPort + 0x002C);
		outb(0x04,pChannel->BaseIoPort + 0x0074);
		outb(0x17,pChannel->BaseIoPort + 0x0018);
		outl(0,pChannel->BaseIoPort + 0x0014);
		outl(0xFFFFFFFF,pChannel->BaseIoPort + 0x0030);	// Clear SError(SERR);
		//============================================
		// setup Command Table
		//============================================
		pCommandHead  =	pChannel->pCommandPtr->pCommandHead;
		pCommandTable =	pChannel->pCommandPtr->pCommandTable;
		for (j = 0 ; j <= MAX_COMMAND_QUEUE_CH ; j++) {
			pCommandHead->CommandFISLen	= 5;
			pCommandHead->AtapiPioFIS		= 0;
			pCommandHead->ReadWrite		= 0;
			pCommandHead->PreFetchAble	= 0;
			pCommandHead->Reset		= 0;
			pCommandHead->Bist		= 0;
			pCommandHead->ClearBusy		= 0;
			pCommandHead->Reserved0		= 0;
			pCommandHead->PortMultiplier	= 0;
			pCommandHead->PRDTL		= 0;
			pCommandHead->PRD_ByteCount	= 0;
			pCommandHead->CommandTablePtrU	= 0;
#if __LITTLE_ENDIAN
			pCommandHead->CommandTablePtr	= pDmaCommandTable;
#else
			pCommandHead->CommandTablePtr	= cpu_to_le32(pDmaCommandTable);
#endif
			pCommandHead++;
			pCommandTable++;
			pDmaCommandTable += MEM_512B;
		}
		pUncachedExtension += MEM_1KB;
		pDmaUncachedExtension += MEM_1KB;
		//============================================
		// Initialize SATA Device
		//============================================
		pDeviceInfo = &(pHwDeviceExtension->DeviceInfo[(i * MAX_SATA_HDD) + 2]);
		for (j = 0 ; j < MAX_SATA_HDD ; j++) {
			pDeviceInfo->FG_SerialATA		= 1;
			pDeviceInfo->FG_OnSataChannel	= 1;
			pDeviceInfo->FG_PMId		= j;
			pDeviceInfo->pChannel		= pChannel;
			pDeviceInfo++;
		}
		pChannel++;
	}
//	for (i = 0 ; i < 800000 ; i++) {
//		udelay(10);
//		cpu_relax();
//	}
	msleep(5000);
	//================================
	// SCAN SATA PHYSICAL HARD DISK
	//================================
	for (i = 0 ; i < MAX_SATA_CHANNEL ; i++) {
		pDeviceInfo = &(pHwDeviceExtension->DeviceInfo[(i * MAX_SATA_HDD) + 2]);
		pChannel = &(pHwDeviceExtension->SataChannel[i]);
		pChannel->FG_PortMultiplier = 0;
		pChannel->MaxDeviceCount	  = 1;
		// check Port Multiplier
		for (j = 0 ; j < 300000 ; j++) {
			statusX = (inb(pChannel->BaseIoPort + 0x0028) & 0x0F);
			if (statusX == 0x03) {
				break;
			}
			udelay(10);
		}
		if (statusX == 0x03) {
			// physical ready
			//==========================================================================================
			// clear BUSY & DRQ bit to release channel
			//==========================================================================================
			status = inb(pChannel->BaseIoPort + 0x0018) | 0x08;
			outb(status,pChannel->BaseIoPort + 0x0018);
			while(inb(pChannel->BaseIoPort + 0x0018) & 0x08) {
				udelay(10);
				cpu_relax();
			}
			if (SoftwareReset(pChannel,0x0F)) {
				signature = inl(pChannel->BaseIoPort + 0x0024);
				if (signature == 0x96690101) { 
					// found Port Multiplier
					pChannel->FG_PortMultiplier = 1;
					PortMultiplierEnumerate(pHwDeviceExtension,pChannel,pDeviceInfo);
					outb(0x17,pChannel->BaseIoPort + 0x0018);		// CLEAR BSY & DRQ
					outl(0x00000109,pChannel->BaseIoPort + 0x0014);	// enable P0IS.NOTIS, P0IS.SDBS, P0IS.DHRS
					outl(0x11111112,pChannel->BaseIoPort + 0x0074);	// enable PMx D2H & PM bit
				} else if (signature == 0xEB140101) {
					// found ATAPI DEVICE
					SendIdentifyCommand(pHwDeviceExtension,pDeviceInfo,0,0xA1);
					outb(0x17,pChannel->BaseIoPort + 0x0018);		// CLEAR BSY & DRQ
					outl(0x00000009,pChannel->BaseIoPort + 0x0014);	// enable P0IS.SDBS, P0IS.DHRS
					outl(0,pChannel->BaseIoPort + 0x0074);
				} else if (signature == 0x00000101) {
					// found ATA DEVICE
					SendIdentifyCommand(pHwDeviceExtension,pDeviceInfo,0,0xEC);
					outb(0x17,pChannel->BaseIoPort + 0x0018);		// CLEAR BSY & DRQ
					outl(0x00000009,pChannel->BaseIoPort + 0x0014);	// enable P0IS.SDBS, P0IS.DHRS
					outl(0,pChannel->BaseIoPort + 0x0074);
					statusX = inb(pChannel->BaseIoPort + 0x007A) | 0x77;
					outb(statusX,pChannel->BaseIoPort + 0x007A);
				}
			}
		}
		outl(0xFFFFFFFF,pChannel->BaseIoPort + 0x0030);				// clear interrupt
	}
	pci_write_config_word(pPciDev, 0x04, 0x0007);		// Set PCI Command Reigster
	pci_read_config_byte(pPciDev, 0x0D,&statusX);		// Set PCI Latency Timer
	if (statusX < 0x20) {
		pci_write_config_byte(pPciDev,0x0D,0xC0);
	}
	pHwDeviceExtension->pScsiHost->max_id	= 12;
	pHwDeviceExtension->pScsiHost->max_channel	= 0;
	pHwDeviceExtension->pScsiHost->this_id	= 16;
	pHwDeviceExtension->pScsiHost->n_io_port	= 0x80;
	//================================================================
	// enable IDE to INTA#
	//================================================================
	outb(0x08,pHwDeviceExtension->BaseIoPort0 + 0x0091);
	//================================================================
	// enable INTA
	//================================================================
	outb(0x02,pHwDeviceExtension->BaseIoPort0 + 0x0004);
	//================================================================
	// start timer
	//================================================================
	init_timer(&(pHwDeviceExtension->AdapterTimer));
	(pHwDeviceExtension->AdapterTimer).data	= (unsigned long)(pHwDeviceExtension);
	(pHwDeviceExtension->AdapterTimer).function	= adapter_timer;
	(pHwDeviceExtension->AdapterTimer).expires	= jiffies + TIMER_SCALE;
	add_timer(&(pHwDeviceExtension->AdapterTimer));
	Debug80Port(0xF4,1);
	return 0;
}
//========================================================================
static int AllocateExtensionMemory(PHW_DEVICE_EXTENSION pHwDeviceExtension)
{
	unsigned char *pAllocateBuffer;
	dma_addr_t pDmaAllocateBufferr;

	//=== allocate SATA Channel 1 Command Table========================================
	pAllocateBuffer = pci_alloc_consistent(pHwDeviceExtension->pPciDev,MEM_4KB,&pDmaAllocateBufferr);
	if (pAllocateBuffer == NULL) {
		return 0;
	}
	memset(pAllocateBuffer,0,MEM_4KB);
	pHwDeviceExtension->pSataChCmdTable[0]	= pAllocateBuffer;
	pHwDeviceExtension->pDmaSataChCmdTable[0]	= pDmaAllocateBufferr;
	//=== allocate SATA Channel 2 Command Table========================================
	pAllocateBuffer = pci_alloc_consistent(pHwDeviceExtension->pPciDev,MEM_4KB,&pDmaAllocateBufferr);
	if (pAllocateBuffer == NULL) {
		pci_free_consistent(pHwDeviceExtension->pPciDev,
					MEM_4KB,
					pHwDeviceExtension->pSataChCmdTable[0],
					pHwDeviceExtension->pDmaSataChCmdTable[0]);
		return 0;
	}
	memset(pAllocateBuffer,0,MEM_4KB);
	pHwDeviceExtension->pSataChCmdTable[1]	= pAllocateBuffer;
	pHwDeviceExtension->pDmaSataChCmdTable[1]	= pDmaAllocateBufferr;
	//=== allocate Uncache Extension Memory ============================================
	pAllocateBuffer = pci_alloc_consistent(pHwDeviceExtension->pPciDev,MEM_4KB,&pDmaAllocateBufferr);
	if (pAllocateBuffer == NULL) {
		pci_free_consistent(pHwDeviceExtension->pPciDev,
					MEM_4KB,
					pHwDeviceExtension->pSataChCmdTable[0],
					pHwDeviceExtension->pDmaSataChCmdTable[0]);
		pci_free_consistent(pHwDeviceExtension->pPciDev,
					MEM_4KB,
					pHwDeviceExtension->pSataChCmdTable[1],
					pHwDeviceExtension->pDmaSataChCmdTable[1]);
		return 0;
	}
	memset(pAllocateBuffer,0,MEM_4KB);
	pHwDeviceExtension->pUncachedExtension	= pAllocateBuffer;
	pHwDeviceExtension->pDmaUncachedExtension	= pDmaAllocateBufferr;
	//================================================================================
	pHwDeviceExtension->pIdentifyBuffer	= pAllocateBuffer + MEM_2KB;
	pHwDeviceExtension->pDmaIdentifyBuffer	= pDmaAllocateBufferr + MEM_2KB;
	pHwDeviceExtension->pIdeMailBox		= (P_MailBox)(pAllocateBuffer + MEM_2KB+MEM_512B);
	pHwDeviceExtension->pDmaIdeMailBox		= pDmaAllocateBufferr + (MEM_2KB+MEM_512B);
	pHwDeviceExtension->FG_ExtensionAllocate   = 0xFF;
	return 1;
}
//========================================================================
static void ReleaseExtensionMemory(PHW_DEVICE_EXTENSION pHwDeviceExtension)
{
	if (pHwDeviceExtension->FG_ExtensionAllocate) {
		pci_free_consistent(pHwDeviceExtension->pPciDev,
					MEM_4KB,
					pHwDeviceExtension->pSataChCmdTable[0],
					pHwDeviceExtension->pDmaSataChCmdTable[0]);
		pci_free_consistent(pHwDeviceExtension->pPciDev,
					MEM_4KB,
					pHwDeviceExtension->pSataChCmdTable[1],
					pHwDeviceExtension->pDmaSataChCmdTable[1]);
		pci_free_consistent(pHwDeviceExtension->pPciDev,
					MEM_4KB,
					pHwDeviceExtension->pUncachedExtension,
					pHwDeviceExtension->pDmaUncachedExtension);
	}
	pHwDeviceExtension->FG_ExtensionAllocate = 0;
	return;
}
//========================================================================
static void adapter_release_resource(void *pscsihostdata)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension;
	struct pci_dev *pPciDev;
	struct Scsi_Host *pScsiHost;
	P_ScsiHostData pScsiHostData;

	pScsiHostData = (P_ScsiHostData)pscsihostdata;
	if (pScsiHostData->DeviceExtension != NULL) {
		pHwDeviceExtension = (PHW_DEVICE_EXTENSION)(pScsiHostData->DeviceExtension);
		pScsiHost          = pScsiHostData->pScsiHost;
		pPciDev            = pScsiHostData->pPciDev;
		//================================================================
		// stop timer
		//================================================================
		pHwDeviceExtension->FG_TimerStop = 1;
		mDelay(30);
		del_timer_sync(&(pHwDeviceExtension->AdapterTimer));
		//================================================================
		outb(0x00,pHwDeviceExtension->BaseIoPort0 + 0x0004);		// Disable INTA#
		//================================================================
		// release Resource
		//================================================================
		release_region(pHwDeviceExtension->BaseIoPort0, 0xFF);
		release_region(pHwDeviceExtension->BaseIoPort1, 0xFF);
		//================================================================
		// unregister IRQ
		//================================================================
		free_irq(pHwDeviceExtension->IrqNum,pScsiHost);
		//================================================================
		// free memory
		//================================================================
		ReleaseExtensionMemory(pHwDeviceExtension);
		free_pages((unsigned long)pHwDeviceExtension,EXTENSION_MEM);
		pScsiHostData->DeviceExtension   = NULL;
		pScsiHostData->szDeviceExtension = 0;
	}
	return;
}
//========================================================================
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static int adapter_reset(Scsi_Cmnd * pScsiCmd, unsigned int reset_flags)
#else
static int adapter_reset(Scsi_Cmnd * pScsiCmd)
#endif
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension;
	P_WorkingTable pWorkingTable;
	P_Channel pChannel;
	P_DeviceInfo pDeviceInfo;
	struct Scsi_Host *pScsiHost;
	unsigned int signature;
	int i,j;
	unsigned char status;
	
	Debug80Port(0xFA,1);
	pScsiHost = (struct Scsi_Host *)(pScsiCmd->device->host);
	pHwDeviceExtension = (PHW_DEVICE_EXTENSION)(((P_ScsiHostData)(pScsiHost->hostdata))->DeviceExtension);
	//========================================================================
	// STOP TIMER
	//========================================================================
	del_timer_sync(&(pHwDeviceExtension->AdapterTimer));
	//================================================================
	// Reset Chip
	//================================================================
	outb(0x01,pHwDeviceExtension->BaseIoPort0 + 0x0004);
	while (inb(pHwDeviceExtension->BaseIoPort0 + 0x0004) & 0x01) {
		udelay(10);
		cpu_relax();
	}
	//================================================================
	// Disable INTA#
	//================================================================
	outb(0x00,pHwDeviceExtension->BaseIoPort0 + 0x0004);
	//========================================================================
	// Clear all request
	//========================================================================
	pChannel = &(pHwDeviceExtension->IdeChannel);
	pChannel->FG_StopIO	= 0;
	pChannel->pCurRequest	= 0;
	pChannel->pPendingIO	= 0;
	pChannel->QueueCount	= 0;
	pChannel->QueueStartTag	= 0;
	outb(0,pChannel->IdePortPSFF);		//Stop DMA
	//===========================================
	for (j = 0 ; j < MAX_SATA_CHANNEL ; j++) {
		pChannel = &(pHwDeviceExtension->SataChannel[j]);
		pChannel->FG_StopIO	= 0;
		pChannel->FG_ErrD2H	= 0;
		pChannel->FG_ErrSDB	= 0;
		pChannel->pCurRequest	= 0;
		pChannel->pPendingIO	= 0;
		for (i = 0 ; i <= MAX_COMMAND_QUEUE_CH ; i++) {
			pChannel->pExecuteIO[i]	= 0;
		}
		pChannel->QueueCount	= 0;
		pChannel->QueueStartTag	= 0;
		pChannel->FG_ErrPMx	= 0;
	}
	pHwDeviceExtension->IoCounter	= 0;
	//========================================================================
	// Clear all WorkingTable
	//========================================================================
	pWorkingTable = (P_WorkingTable)(pHwDeviceExtension->pWorkingTablePool);
	for (i = 0 ; i < MAX_QUEUE_SUPPORT ; i++) {
		pWorkingTable->FG_TableActive = 0;
		pWorkingTable++;
	}
	//============================================
	// Initialize IDE channel
	//============================================
	pDeviceInfo = &(pHwDeviceExtension->DeviceInfo[0]);
	for (i = 0 ; i < pChannel->MaxDeviceCount ; i++) {
		ScanATA(pHwDeviceExtension,pDeviceInfo);
		if (pDeviceInfo->FG_DeviceInstall) {
			SetChipIo(pDeviceInfo);
		} else {
			ScanATAPI(pHwDeviceExtension,pDeviceInfo);
			if (pDeviceInfo->FG_DeviceInstall) {
				SetChipIo(pDeviceInfo);
			}
		}
		pDeviceInfo++;
	}
	//============================================
	// Initialize SATA channel 
	//============================================
	pChannel   = &(pHwDeviceExtension->SataChannel[0]);
	for (i = 0 ; i < MAX_SATA_CHANNEL ; i++) {
		pChannel = &(pHwDeviceExtension->SataChannel[i]);
		//============================================
		// initialize SATA channel
		//============================================
		outb(0x01,pChannel->BaseIoPort + 0x002C);
		outb(0x00,pChannel->BaseIoPort + 0x002C);
		outb(0x04,pChannel->BaseIoPort + 0x0074);
		outb(0x17,pChannel->BaseIoPort + 0x0018);
		outl(0,pChannel->BaseIoPort + 0x0014);
		//============================================
		outl(0xFFFFFFFF,pChannel->BaseIoPort + 0x0030);	// Clear SError(SERR);
		pDeviceInfo = &(pHwDeviceExtension->DeviceInfo[(i * MAX_SATA_HDD) + 2]);
		//============================================
		// check Port Multiplier
		if ((inb(pChannel->BaseIoPort + 0x0028) & 0x0F) == 0x03) {
			// physical ready
			//==========================================================================================
			// clear BUSY & DRQ bit to release channel
			//==========================================================================================
			status = inb(pChannel->BaseIoPort + 0x0018) | 0x08;
			outb(status,pChannel->BaseIoPort + 0x0018);
			while(inb(pChannel->BaseIoPort + 0x0018) & 0x08) {
				udelay(10);
				cpu_relax();
			}
			if (SoftwareReset(pChannel,0x0F)) {
				signature = inl(pChannel->BaseIoPort + 0x0024);
				if (signature == 0x96690101) {
					// found Port Multiplier
					pChannel->FG_PortMultiplier = 1;
					PortMultiplierEnumerate(pHwDeviceExtension,pChannel,pDeviceInfo);
					outb(0x17,pChannel->BaseIoPort + 0x0018);		// CLEAR BSY & DRQ
					outl(0x00000109,pChannel->BaseIoPort + 0x0014);	// enable P0IS.NOTIS, P0IS.SDBS, P0IS.DHRS
					outl(0x11111112,pChannel->BaseIoPort + 0x0074);	// enable PMx D2H & PM bit
				} else if (signature == 0xEB140101) {
					// found ATAPI DEVICE
					SendIdentifyCommand(pHwDeviceExtension,pDeviceInfo,0,0xA1);
					outb(0x17,pChannel->BaseIoPort + 0x0018);		// CLEAR BSY & DRQ
					outl(0x00000009,pChannel->BaseIoPort + 0x0014);	// enable P0IS.SDBS, P0IS.DHRS
					outl(0,pChannel->BaseIoPort + 0x0074);
				} else if (signature == 0x00000101) {
					// found ATA DEVICE
					SendIdentifyCommand(pHwDeviceExtension,pDeviceInfo,0,0xEC);
					outb(0x17,pChannel->BaseIoPort + 0x0018);		// CLEAR BSY & DRQ
					outl(0x00000009,pChannel->BaseIoPort + 0x0014);	// enable P0IS.SDBS, P0IS.DHRS
					outl(0,pChannel->BaseIoPort + 0x0074);
				}
			}
		}
	}
	//========================================================================
	// RESTART TIMER
	//========================================================================
	(pHwDeviceExtension->AdapterTimer).expires = jiffies + TIMER_SCALE;
	add_timer(&(pHwDeviceExtension->AdapterTimer));
	//========================================================================
	return STATUS_GOOD;
}
static void I2C_READ(void *DeviceExtension,Scsi_Cmnd *pScsiCmd)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	unsigned char status;

	status = inb(pHwDeviceExtension->BaseIoPort0 + 0x00B0);
	outb(pScsiCmd->cmnd[1],pHwDeviceExtension->BaseIoPort0 + 0x00B1);//start bit	
	outb((status|STA|RD|IFM),pHwDeviceExtension->BaseIoPort0 + 0x00B0);//start bit	
	pHwDeviceExtension->pScsiCmd = pScsiCmd;
	pHwDeviceExtension->I2C_STEP = R_F2;	
}
//===================================================================================
static int adapter_queuecommand(Scsi_Cmnd *pScsiCmd, void (*done) (Scsi_Cmnd *))
{
	struct Scsi_Host *pScsiHost;
	struct scatterlist *sg_list;
	PHW_DEVICE_EXTENSION pHwDeviceExtension;
	P_Channel pChannel;
	P_DeviceInfo pDeviceInfo;
	PUINQUIRYDATA pInquiryBuff;
	P_WorkingTable pWorkingTable;
	unsigned char *km_buffer;
	unsigned long km_flags,len,flags;
	unsigned char temp_buffer[250];
	int i,j;

	pScsiHost = (struct Scsi_Host *)(pScsiCmd->device->host);
	pHwDeviceExtension = (PHW_DEVICE_EXTENSION)(((P_ScsiHostData)(pScsiHost->hostdata))->DeviceExtension);
	if (	  (pScsiCmd->device->lun > 0)			// only support lun0
		 ||(pScsiCmd->device->id >= MAXTARGETHDD )
	   ) {
		if((pScsiCmd->device->lun == 7) && (pScsiCmd->device->id ==0)){
			pScsiCmd->scsi_done = done;
		//	pScsiCmd->result = SAM_STAT_GOOD;
			pScsiCmd->result = STATUS_PENDING;
			switch (pScsiCmd->cmnd[0]){
				case I2C_CMD_READ:
					I2C_READ(pHwDeviceExtension,pScsiCmd);
				case I2C_CMD_WRITE:
//					I2C_WRITE(pHwDeviceExtension,pScsiCmd);
				default:
					pScsiCmd->result = STATUS_NO_DEVICE;
					done(pScsiCmd);
					
			}
			//I2C DRIVER 
		}else{
			pScsiCmd->result = STATUS_NO_DEVICE;
			done(pScsiCmd);
			return 0;
		}
	}
	pScsiCmd->result = SAM_STAT_GOOD;
	pDeviceInfo = &(pHwDeviceExtension->DeviceInfo[pScsiCmd->device->id]);
	if (pDeviceInfo->FG_DeviceInstall == 0) {
		pScsiCmd->result = STATUS_NO_DEVICE;
		done(pScsiCmd);
	} else {
	//	printk("adapter_queuecommand cmd %x\n",pScsiCmd->cmnd[0]);
#ifdef KERNEL_V2625
		sg_list = scsi_sglist(pScsiCmd);
		len	= scsi_bufflen(pScsiCmd);
	//	printk("sglist %x len %x\n",(unsigned long)sg_list,len);
#else
		sg_list = (struct scatterlist *)(pScsiCmd->request_buffer);
		len = pScsiCmd->request_bufflen;
#endif
		pChannel = pDeviceInfo->pChannel;
		
		if (len > 250) {
			len = 250;
		}
		memset(temp_buffer,0,sizeof(temp_buffer));
		if (pDeviceInfo->FG_AtapiDevice == 0) {
			// Hard Disk (ATA Device)
			switch (pScsiCmd->cmnd[0]) {
			case INQUIRY:
				temp_buffer[2] = 0x02;
				temp_buffer[3] = 0x02;
				temp_buffer[4] = 0x7d;
				temp_buffer[7] = 0x30;
				pInquiryBuff   = (PUINQUIRYDATA)(temp_buffer);
				if (	  ((pDeviceInfo->ModelName[0] == 'S') || (pDeviceInfo->ModelName[0] == 's'))
					&&((pDeviceInfo->ModelName[1] == 'T') || (pDeviceInfo->ModelName[1] == 't'))
				   ) {
					memcpy(pInquiryBuff->VendorId,"Seagate ",8);
					// set Product name string
					for (i = 0 ; i < 16 ; i++) {
						pInquiryBuff->ProductId[i]	= pDeviceInfo->ModelName[i];
					}
				} else if (  ((pDeviceInfo->ModelName[0] == 'H') || (pDeviceInfo->ModelName[0] == 'I'))
					  &&((pDeviceInfo->ModelName[1] == 'D') || (pDeviceInfo->ModelName[1] == 'C'))
					  ) {
					memcpy(pInquiryBuff->VendorId,"Hitachi ",8);
					// set Product name string
					for (i = 0 ; i < 16 ; i++) {
						pInquiryBuff->ProductId[i]	= pDeviceInfo->ModelName[i];
					}
				} else if (  ((pDeviceInfo->ModelName[0] == 'M') || (pDeviceInfo->ModelName[0] == 'M'))
					  &&((pDeviceInfo->ModelName[1] == 'a') || (pDeviceInfo->ModelName[1] == 'A'))
					  ) {
					memcpy(pInquiryBuff->VendorId,"Maxtor  ",8);
					// set Product name string
					for (i = 0 ; i < 16 ; i++) {
						pInquiryBuff->ProductId[i]	= pDeviceInfo->ModelName[i+7];
					}
				} else if (  ((pDeviceInfo->ModelName[0] == 'W') || (pDeviceInfo->ModelName[0] == 'w'))
					  &&((pDeviceInfo->ModelName[1] == 'D') || (pDeviceInfo->ModelName[1] == 'd'))
					  &&((pDeviceInfo->ModelName[2] == 'C') || (pDeviceInfo->ModelName[1] == 'c'))
					  ) {
					memcpy(pInquiryBuff->VendorId,"WDC     ",8);
					// set Product name string
					for (i = 0 ; i < 16 ; i++) {
						pInquiryBuff->ProductId[i]	= pDeviceInfo->ModelName[i+4];
					}
				} else {
					for (i = 0 ; i < 8 ; i++) {
						pInquiryBuff->VendorId[i] = pDeviceInfo->ModelName[i];
					}
					// set Product name string
					for (i = 0 ; i < 16 ; i++) {
						pInquiryBuff->ProductId[i]	= pDeviceInfo->ModelName[i+8];
					}
				}
				// set Product Revision string
				for	(i = 0 ; i < 4 ; i++) {
					pInquiryBuff->ProductRevisionLevel[i] = pDeviceInfo->FirmWareRev[i];
				}
				for (i = 0 ; i < 19 ; i++) {
					pInquiryBuff->VendorSpecific[i] = pDeviceInfo->SerialNo[i];
				}
				break;
			case READ_CAPACITY:
				i =  ((pDeviceInfo->Capacity).U64L).U64L_L - 1;
				// Max. LBA
				temp_buffer[0] = (unsigned char)(i >> 24);
				temp_buffer[1] = (unsigned char)(i >> 16);
				temp_buffer[2] = (unsigned char)(i >> 8);
				temp_buffer[3] = (unsigned char)i;
				// set block size
				temp_buffer[4] = 0x00;
				temp_buffer[5] = 0x00;
				temp_buffer[6] = 0x02;
				temp_buffer[7] = 0x00;
				break;
			case REQUEST_SENSE:	// request sense
				temp_buffer[0]  = 0x70;
				temp_buffer[2]  = pDeviceInfo->SenseKey;
				temp_buffer[12] = pDeviceInfo->SenseAsc;
				temp_buffer[13] = pDeviceInfo->SenseAscQ;
				pDeviceInfo->SenseKey  = 0;
				pDeviceInfo->SenseAsc  = 0;
				pDeviceInfo->SenseAscQ = 0;
				break;
			case MODE_SENSE:		// mode sense 6
//	printk("CDB = %02X %02X %02X %02X %02X %02X\n",pScsiCmd->cmnd[0],pScsiCmd->cmnd[1],pScsiCmd->cmnd[2],pScsiCmd->cmnd[3],pScsiCmd->cmnd[4],pScsiCmd->cmnd[5]);
				i =  ((pDeviceInfo->Capacity).U64L).U64L_L;
				switch (pScsiCmd->cmnd[2] & 0x3F) {
				case 0x00:
					if (pScsiCmd->cmnd[1] & 0x08) { 
						// DBD = 1
						memcpy(temp_buffer,ModePage0DBD,sizeof(ModePage0DBD));
					} else {
						memcpy(temp_buffer,ModePage0,sizeof(ModePage0));
						temp_buffer[4] = (unsigned char)(i>>24);
						temp_buffer[5] = (unsigned char)(i>>16);
						temp_buffer[6] = (unsigned char)(i>>8);
						temp_buffer[7] = (unsigned char)i;
					}
					break;
				case 0x01:
					if (pScsiCmd->cmnd[1] & 0x08) {
						// DBD = 1
						memcpy(temp_buffer,ModePage1DBD,sizeof(ModePage1DBD));
					} else {
						memcpy(temp_buffer,ModePage1,sizeof(ModePage1));
						temp_buffer[4] = (unsigned char)(i>>24);
						temp_buffer[5] = (unsigned char)(i>>16);
						temp_buffer[6] = (unsigned char)(i>>8);
						temp_buffer[7] = (unsigned char)i;
					}
					break;
				case 0x08:
					if (pScsiCmd->cmnd[1] & 0x08) {
						// DBD = 1
						memcpy(temp_buffer,ModePage8DBD,sizeof(ModePage8DBD));
					} else {
						memcpy(temp_buffer,ModePage8,sizeof(ModePage8));
						temp_buffer[4] = (unsigned char)(i>>24);
						temp_buffer[5] = (unsigned char)(i>>16);
						temp_buffer[6] = (unsigned char)(i>>8);
						temp_buffer[7] = (unsigned char)i;
					}
					break;
				case 0x1C:
					if (pScsiCmd->cmnd[1] & 0x08) {
						// DBD = 1
						memcpy(temp_buffer,ModePage1cDBD,sizeof(ModePage1cDBD));
					} else {
						memcpy(temp_buffer,ModePage1c,sizeof(ModePage1c));
						temp_buffer[4] = (unsigned char)(i>>24);
						temp_buffer[5] = (unsigned char)(i>>16);
						temp_buffer[6] = (unsigned char)(i>>8);
						temp_buffer[7] = (unsigned char)i;
					}
					break;
				case 0x3F:
					if (pScsiCmd->cmnd[1] & 0x08) {
						// DBD = 1
						memcpy(&(temp_buffer[4]),&(ModePage1[12]),12);
						memcpy(&(temp_buffer[16]),&(ModePage8[12]),12);
						memcpy(&(temp_buffer[28]),&(ModePage1c[12]),12);
						memcpy(&(temp_buffer[40]),&(ModePage0[12]),4);
						temp_buffer[0] = 40;
					} else {
						memcpy(&(temp_buffer[0]),&(ModePage1[0]),24);
						memcpy(&(temp_buffer[24]),&(ModePage8[12]),12);
						memcpy(&(temp_buffer[36]),&(ModePage1c[12]),12);
						memcpy(&(temp_buffer[48]),&(ModePage0[12]),4);
						temp_buffer[0] = 52;
						temp_buffer[4] = (unsigned char)(i>>24);
						temp_buffer[5] = (unsigned char)(i>>16);
						temp_buffer[6] = (unsigned char)(i>>8);
						temp_buffer[7] = (unsigned char)i;
					}
					break;
				default:
					pDeviceInfo->SenseKey  = ILLEGAL_REQUEST;
					pDeviceInfo->SenseAsc  = 0x24;
					pDeviceInfo->SenseAscQ = 0x00;
					pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
					break;
				}
				break;
			case ALLOW_MEDIUM_REMOVAL:
			case MODE_SELECT:
			case TEST_UNIT_READY:
			case START_STOP:
			case SEEK_6:
			case SEEK_10:
			case SYNCHRONIZE_CACHE:
				len = 0;
				break;		
			case VERIFY:
			case READ_6:	// read
			case READ_10:
			case READ_12:
			case READ_16:
			case WRITE_6:	// write
			case WRITE_10:
			case WRITE_12:
			case WRITE_16:
				spin_lock_irqsave(&(pHwDeviceExtension->spin_lock),flags);
				pWorkingTable = AllocateWorkingTable(pHwDeviceExtension);
				if (pWorkingTable) {
					pWorkingTable->FG_Updata		= 0;
					pWorkingTable->FG_NonData		= 0;
					pWorkingTable->pDevice			= pDeviceInfo;
					pWorkingTable->pWorkingScsiCmd		= pScsiCmd;
					pWorkingTable->IoTagID			= pHwDeviceExtension->IoTagID;
#ifdef KERNEL_V2625
					pWorkingTable->request_buffer		= scsi_sglist(pScsiCmd);
					pWorkingTable->request_bufflen		= scsi_bufflen(pScsiCmd);
					pWorkingTable->use_sg			= scsi_sg_count(pScsiCmd);//This version always use use_sg
#else
					pWorkingTable->request_buffer		= pScsiCmd->request_buffer;
					pWorkingTable->request_bufflen		= pScsiCmd->request_bufflen;
					pWorkingTable->use_sg			= pScsiCmd->use_sg;
#endif
					switch(pScsiCmd->cmnd[0]) {
						case READ_6:
						case WRITE_6:
							((pWorkingTable->StartLBA).U64L).U64L_H = 0;
							((pWorkingTable->StartLBA).U64L).U64L_L =
								(((((unsigned int)pScsiCmd->cmnd[1])<<16) & 0x000F0000)+
								((((unsigned int)pScsiCmd->cmnd[2])<<8)  & 0x0000FF00)+
								((((unsigned int)pScsiCmd->cmnd[3]))     & 0x000000FF));
							((pWorkingTable->TransferLen).U64L).U64L_H = 0;
							((pWorkingTable->TransferLen).U64L).U64L_L =
									(((unsigned int)pScsiCmd->cmnd[4]) & 0x000000FF);
							break;
						case READ_10:
						case WRITE_10:						
							((pWorkingTable->StartLBA).U64L).U64L_H = 0;
							((pWorkingTable->StartLBA).U64L).U64L_L =
								((((unsigned int)pScsiCmd->cmnd[2])<<24) & 0xFF000000)+
								((((unsigned int)pScsiCmd->cmnd[3])<<16) & 0x00FF0000)+
								((((unsigned int)pScsiCmd->cmnd[4])<<8) & 0x0000FF00)+
								((((unsigned int)pScsiCmd->cmnd[5])) & 0x000000FF);
							((pWorkingTable->TransferLen).U64L).U64L_H = 0;
							((pWorkingTable->TransferLen).U64L).U64L_L =
								((((unsigned int)pScsiCmd->cmnd[7])<<8) & 0x0000FF00) +
								((((unsigned int)pScsiCmd->cmnd[8])) & 0x000000FF);
							break;
						case READ_12:
						case WRITE_12:
							((pWorkingTable->StartLBA).U64L).U64L_H = 0;
							((pWorkingTable->StartLBA).U64L).U64L_L =
								((((unsigned int)pScsiCmd->cmnd[2])<<24) & 0xFF000000)+
								((((unsigned int)pScsiCmd->cmnd[3])<<16) & 0x00FF0000)+
								((((unsigned int)pScsiCmd->cmnd[4])<<8) & 0x0000FF00)+
								((((unsigned int)pScsiCmd->cmnd[5]))    & 0x000000FF);
							((pWorkingTable->TransferLen).U64L).U64L_H = 0;
							((pWorkingTable->TransferLen).U64L).U64L_L =
								((((unsigned int)pScsiCmd->cmnd[6])<<24) & 0xFF000000)+
								((((unsigned int)pScsiCmd->cmnd[7])<<16) & 0x00FF0000)+
								((((unsigned int)pScsiCmd->cmnd[8])<<8) & 0x0000FF00)+
								((((unsigned int)pScsiCmd->cmnd[9]))    & 0x000000FF);
							break;
						case READ_16:
						case WRITE_16:
							((pWorkingTable->StartLBA).U64L).U64L_H =
									((((unsigned int)pScsiCmd->cmnd[2])<<24) & 0xFF000000)+
								((((unsigned int)pScsiCmd->cmnd[3])<<16) & 0x00FF0000)+
								((((unsigned int)pScsiCmd->cmnd[4])<<8) & 0x0000FF00)+
								((((unsigned int)pScsiCmd->cmnd[5]))    & 0x000000FF);
							((pWorkingTable->StartLBA).U64L).U64L_L =
								((((unsigned int)pScsiCmd->cmnd[6])<<24) & 0xFF000000)+
								((((unsigned int)pScsiCmd->cmnd[7])<<16) & 0x00FF0000)+
								((((unsigned int)pScsiCmd->cmnd[8])<<8) & 0x0000FF00)+
								((((unsigned int)pScsiCmd->cmnd[9]))    & 0x000000FF);
							((pWorkingTable->TransferLen).U64L).U64L_H = 0;
							((pWorkingTable->TransferLen).U64L).U64L_L =
								((((unsigned int)pScsiCmd->cmnd[10])<<24) & 0xFF000000)+
								((((unsigned int)pScsiCmd->cmnd[11])<<16) & 0x00FF0000)+
								((((unsigned int)pScsiCmd->cmnd[12])<<8) & 0x0000FF00)+
								((((unsigned int)pScsiCmd->cmnd[13]))    & 0x000000FF);
							break;						
					}
					((pWorkingTable->StartLBAOrg).U64L).U64L_H =
								((pWorkingTable->StartLBA).U64L).U64L_H;
					((pWorkingTable->StartLBAOrg).U64L).U64L_L =
								((pWorkingTable->StartLBA).U64L).U64L_L;
					((pWorkingTable->TransferLenOrg).U64L).U64L_H =
								((pWorkingTable->TransferLen).U64L).U64L_H;
					((pWorkingTable->TransferLenOrg).U64L).U64L_L =
								((pWorkingTable->TransferLen).U64L).U64L_L;
					ConvertCommandATA(pWorkingTable);
					pHwDeviceExtension->IoTagID++;
					if (pDeviceInfo->pChannel->FG_SataChannel) {
						// SATA IO
						if (pDeviceInfo->QueueCount >= pDeviceInfo->MaxQueueDepth) {
							pScsiCmd->result = STATUS_BUSY;
							ReleaseWorkingTable(pWorkingTable);
						} else {
							pScsiCmd->result = STATUS_PENDING;
							pChannel = pDeviceInfo->pChannel;
							if (pChannel->pPendingIO) {
								// there are pending IO in Task Queue
								ChannelIoHook(pWorkingTable);
							} else {
								if (pDeviceInfo->FG_QueueSupported) {
									if (pChannel->FG_StopIO) {
										ChannelIoHook(pWorkingTable);
									} else {
										if (pChannel->QueueCount >= MAX_COMMAND_QUEUE_CH) {
											ChannelIoHook(pWorkingTable);
										} else {
											j = (int)(pChannel->QueueStartTag);
											for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
												if (pChannel->pExecuteIO[j] == 0) {
													// found empty Queue tag
													break;
												}
												if ( ++j >= MAX_COMMAND_QUEUE_CH) {
													j = 0;
												}
											}
											if (i >= MAX_COMMAND_QUEUE_CH) {
												ChannelIoHook(pWorkingTable);
											} else {
												pChannel->pExecuteIO[j] = pWorkingTable;
												pWorkingTable->TagID   = (unsigned char)j;
												if ( ++j >= MAX_COMMAND_QUEUE_CH) {
													j = 0;
												}
												pChannel->QueueStartTag = (unsigned char)j;
												ExecuteSataATA(pHwDeviceExtension,pWorkingTable);
											}
										}
									}
								} else {
									for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
										if (pChannel->pExecuteIO[i] != 0) {
											// found a executing IO
											break;
										}
									}
									if ((i < MAX_COMMAND_QUEUE_CH) || (pChannel->FG_StopIO)) {
										// channel have IO in process
										ChannelIoHook(pWorkingTable);
									} else {
										// send IO to Device
										pChannel->pExecuteIO[0]	= pWorkingTable;
										pWorkingTable->TagID	= 0;
										pChannel->FG_StopIO	= 1;
										ExecuteSataATA(pHwDeviceExtension,pWorkingTable);
									}
								}
							}
							if (pScsiCmd->result != STATUS_PENDING) {
								pChannel->pExecuteIO[pWorkingTable->TagID] = 0;
								ReleaseWorkingTable(pWorkingTable);
							} else {
								pChannel->QueueCount++;
								pDeviceInfo->QueueCount++;
							}
						}
					} else {
						// IDE IO
						pScsiCmd->result = STATUS_PENDING;
						if (pChannel->pPendingIO) {
							// there are pending IO in Task Queue
							ChannelIoHook(pWorkingTable);
						} else {
							if (pChannel->pCurRequest) {
								ChannelIoHook(pWorkingTable);
							} else {
								pChannel->pCurRequest = pWorkingTable;
								ExecuteIdeATA(pHwDeviceExtension,pChannel);
							}
						}
						if (pScsiCmd->result != STATUS_PENDING) {
							ReleaseWorkingTable(pWorkingTable);
							pChannel->pCurRequest = 0;
						} else {
							pChannel->QueueCount++;
							pDeviceInfo->QueueCount++;
						}
					}
				} else {
					pScsiCmd->result = STATUS_BUSY;
				}
				spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
				len = 0;
				break;
			case Updata_NonData:
			case Fireware_ReadBuffer:
			case Fireware_WriteBuffer:
				spin_lock_irqsave(&(pHwDeviceExtension->spin_lock),flags);
				pWorkingTable = AllocateWorkingTable(pHwDeviceExtension);
				if (pWorkingTable) {
					pWorkingTable->FG_Updata		= 1;
					pWorkingTable->FG_NonData		= 0;
					pWorkingTable->pDevice			= pDeviceInfo;
					pWorkingTable->pWorkingScsiCmd		= pScsiCmd;
					pWorkingTable->IoTagID			= pHwDeviceExtension->IoTagID;
#ifdef KERNEL_V2625
					pWorkingTable->request_buffer		= scsi_sglist(pScsiCmd);
					pWorkingTable->request_bufflen		= scsi_bufflen(pScsiCmd);
					pWorkingTable->use_sg			= scsi_sg_count(pScsiCmd);//This version always use use_sg
#else
					pWorkingTable->request_buffer		= pScsiCmd->request_buffer;
					pWorkingTable->request_bufflen		= pScsiCmd->request_bufflen;
					pWorkingTable->use_sg			= pScsiCmd->use_sg;
#endif
/*
					switch(pScsiCmd->cmnd[0]) {						
						case Fireware_ReadBuffer:
						case Fireware_WriteBuffer:						
							((pWorkingTable->StartLBA).U64L).U64L_H = 0;
							((pWorkingTable->StartLBA).U64L).U64L_L = 0;								
							((pWorkingTable->TransferLen).U64L).U64L_H = 0;
							((pWorkingTable->TransferLen).U64L).U64L_L =
								((((unsigned int)pScsiCmd->cmnd[7])<<8) & 0x0000FF00) +
								((((unsigned int)pScsiCmd->cmnd[8])) & 0x000000FF);
							break;
						case Updata_NonData:
							break;
*/					
				//	((pWorkingTable->StartLBAOrg).U64L).U64L_H =
				//				((pWorkingTable->StartLBA).U64L).U64L_H;
				//	((pWorkingTable->StartLBAOrg).U64L).U64L_L =
				//				((pWorkingTable->StartLBA).U64L).U64L_L;
				//	((pWorkingTable->TransferLenOrg).U64L).U64L_H =
				//				((pWorkingTable->TransferLen).U64L).U64L_H;
				//	((pWorkingTable->TransferLenOrg).U64L).U64L_L =
				//				((pWorkingTable->TransferLen).U64L).U64L_L;
				//	ConvertCommandATA(pWorkingTable);
					pHwDeviceExtension->IoTagID++;
					if (pDeviceInfo->pChannel->FG_SataChannel) {
						// SATA IO
						if (pDeviceInfo->QueueCount >= pDeviceInfo->MaxQueueDepth) {
							pScsiCmd->result = STATUS_BUSY;
							ReleaseWorkingTable(pWorkingTable);
						} else {
							pScsiCmd->result = STATUS_PENDING;
							pChannel = pDeviceInfo->pChannel;
							if (pChannel->pPendingIO) {
								// there are pending IO in Task Queue
								ChannelIoHook(pWorkingTable);							
							} else {								
									for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
										if (pChannel->pExecuteIO[i] != 0) {
											// found a executing IO
											break;
										}
									}
									if ((i < MAX_COMMAND_QUEUE_CH) || (pChannel->FG_StopIO)) {
										// channel have IO in process
										ChannelIoHook(pWorkingTable);
									} else {
										// send IO to Device
										pChannel->pExecuteIO[0]	= pWorkingTable;
										pWorkingTable->TagID	= 0;
										pChannel->FG_StopIO	= 1;
										ExecuteUpdata(pHwDeviceExtension,pWorkingTable);
									}
								}
							}
							if (pScsiCmd->result != STATUS_PENDING) {
								pChannel->pExecuteIO[pWorkingTable->TagID] = 0;
								ReleaseWorkingTable(pWorkingTable);
							} else {
								pChannel->QueueCount++;
								pDeviceInfo->QueueCount++;
							}
						}					 
				} else {
					pScsiCmd->result = STATUS_BUSY;
				}
				spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
				len = 0;
				break;
			default:
				pDeviceInfo->SenseKey  = ILLEGAL_REQUEST;
				pDeviceInfo->SenseAsc  = 0x20;
				pDeviceInfo->SenseAscQ = 0x00;
				pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
				break;
			}

			if ( (scsi_sg_count(pScsiCmd)) && (len > 0) ) {

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
				memcpy(sg_list->address,temp_buffer,len);
#else
				local_irq_save(km_flags);
				km_buffer = kmap_atomic(sg_page(sg_list),KM_USER0) + sg_list->offset;
//				km_buffer = kmap_atomic(sg_list->page,KM_USER0) + sg_list->offset;
				memcpy(km_buffer,temp_buffer,len);
				kunmap_atomic(km_buffer - sg_list->offset,KM_USER0);
				local_irq_restore(km_flags);
#endif
			}
#ifdef KERNEL_V2625
			
#else
			else {
				memcpy(pScsiCmd->request_buffer,temp_buffer,len);
			}
#endif
			if (pScsiCmd->result != STATUS_PENDING) {
				done(pScsiCmd);
			} else {
				pScsiCmd->scsi_done = done;
			}
		} else {
			// CD/DVD (ATAPI Device)
			if (  (pScsiCmd->cmnd[0] == REQUEST_SENSE) 
			   && (pDeviceInfo->FG_InternalError)  ) {
				if (len > 14) {
					len = 14;
				}
				memset(temp_buffer,0,len);
				temp_buffer[0]  = 0x70;
				temp_buffer[2]  = pDeviceInfo->SenseKey;
				temp_buffer[12] = pDeviceInfo->SenseAsc;
				temp_buffer[13] = pDeviceInfo->SenseAscQ;

				if ( (scsi_sg_count(pScsiCmd)) && (len > 0) ) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
					memcpy(sg_list->address,temp_buffer,len);
#else
					local_irq_save(km_flags);
					km_buffer = kmap_atomic(sg_page(sg_list),KM_USER0) + sg_list->offset;
//					km_buffer = kmap_atomic(sg_list->page,KM_USER0) + sg_list->offset;
					memcpy(km_buffer,temp_buffer,len);
					kunmap_atomic(km_buffer - sg_list->offset,KM_USER0);
					local_irq_restore(km_flags);
#endif
				} else {
					
#ifndef KERNEL_V2625
					memcpy(pScsiCmd->request_buffer,temp_buffer,len);
#endif
				}
				pDeviceInfo->FG_InternalError = 0;
				pDeviceInfo->SenseKey  = 0;
				pDeviceInfo->SenseAsc  = 0;
				pDeviceInfo->SenseAscQ = 0;
			} else {
				spin_lock_irqsave(&(pHwDeviceExtension->spin_lock),flags);
				pWorkingTable = AllocateWorkingTable(pHwDeviceExtension);
				if (pWorkingTable) {
					pWorkingTable->FG_Updata	= 0;
					pWorkingTable->FG_NonData	= 0;
					pWorkingTable->pDevice		= pDeviceInfo;
					pWorkingTable->pWorkingScsiCmd	= pScsiCmd;
					pWorkingTable->IoTagID		= pHwDeviceExtension->IoTagID;
#ifdef KERNEL_V2625
					pWorkingTable->request_buffer		= scsi_sglist(pScsiCmd);
					pWorkingTable->request_bufflen		= scsi_bufflen(pScsiCmd);
					pWorkingTable->use_sg			= scsi_sg_count(pScsiCmd);//This version always use use_sg
#else
					pWorkingTable->request_buffer		= pScsiCmd->request_buffer;
					pWorkingTable->request_bufflen		= pScsiCmd->request_bufflen;
					pWorkingTable->use_sg			= pScsiCmd->use_sg;
#endif
//					pWorkingTable->pDataBufPtr	= vbuf;
					if (pWorkingTable->use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
						pWorkingTable->pDataBufPtr = sg_list->address;
#else
						pWorkingTable->pDataBufPtr = 0;
#endif
					} else {
						pWorkingTable->pDataBufPtr = pWorkingTable->request_buffer;
					}
					pWorkingTable->DataCounter = pWorkingTable->request_bufflen;
					pScsiCmd->result = STATUS_PENDING;
					pHwDeviceExtension->IoTagID++;
					if (pDeviceInfo->pChannel->FG_SataChannel) {
						// SATA IO
						pScsiCmd->result = STATUS_PENDING;
						pChannel = pDeviceInfo->pChannel;
						if (pChannel->pPendingIO != 0) {
							// there are pending IO in Task Queue
							ChannelIoHook(pWorkingTable);
						} else {
							for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
								if (pChannel->pExecuteIO[i] != 0) {
									// found a executing IO
									break;
								}
							}
							if (i < MAX_COMMAND_QUEUE_CH) {
								// channel have IO in process
								pChannel->pPendingIO = pWorkingTable;
							} else {
								// send IO to Device
								pChannel->pExecuteIO[0] = pWorkingTable;
								pWorkingTable->TagID = 0;
								pChannel->FG_StopIO = 1;
								ExecuteSataATAPI(pHwDeviceExtension,pWorkingTable);
							}
						}
						pChannel->QueueCount++;
						pDeviceInfo->QueueCount++;
					} else {
						// IDE IO
						pScsiCmd->result = STATUS_PENDING;
						if (pChannel->pPendingIO != 0) {
							// there are pending IO in Task Queue
							ChannelIoHook(pWorkingTable);
						} else {
							if (pChannel->pCurRequest) {
								ChannelIoHook(pWorkingTable);
							} else {
								pChannel->pCurRequest = pWorkingTable;
								ExecuteIdeATAPI(pHwDeviceExtension,pChannel);
							}
						}
					}
				} else {
					pScsiCmd->result = STATUS_BUSY;
				}
				spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
			}
			if (pScsiCmd->result != STATUS_PENDING) {
				done(pScsiCmd);
			} else {
				pScsiCmd->scsi_done = done;
			}
		}
	}
	return 0;
}
//========================================================================
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
static void adapter_intr_handle(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t adapter_intr_handle(int irq, void *dev_id)
#endif
{
	struct Scsi_Host *pScsiHost;
	PHW_DEVICE_EXTENSION pHwDeviceExtension;
	P_Channel pIdeChannel,pSataChannel;
	P_WorkingTable pWorkingTable,pWorkingTable1;
	P_DeviceInfo pIdeDevice,pSataDevice;
	P_ReciveFIS pReciveFIS;
	Scsi_Cmnd *pScsiCmd,*pScsiCmdX;
	struct scatterlist *sg_list;
	PUINQUIRYDATA pInquirybuf;
	unsigned char *pBuf,*km_buffer;
	unsigned char *SrbDataBuffer;
	unsigned char *SrbExtOriginalBuffer;
	unsigned long flags,km_flags;
	unsigned int D2H_status,interrupt_status;
	unsigned int CompleteMap,CompleteMapSDBFG,cancel_ci;
	unsigned short BlockLen;
	int i,j,km_sg_used;
	unsigned short int_status,word;
	unsigned char status,PortMultiplier;
	unsigned char temp[5],len;

	km_buffer = NULL;
	km_flags = 0;
	pScsiHost = (struct Scsi_Host *)dev_id;
	pHwDeviceExtension = (PHW_DEVICE_EXTENSION)(((P_ScsiHostData)(pScsiHost->hostdata))->DeviceExtension);
	spin_lock_irqsave(&(pHwDeviceExtension->spin_lock),flags);
	if (inb(pHwDeviceExtension->BaseIoPort0 + 0x0B) & 0x40) {
		pScsiCmd = 	pHwDeviceExtension->pScsiCmd;
		status = inb(pHwDeviceExtension->BaseIoPort0 + 0x00B0);
		switch (pHwDeviceExtension->I2C_STEP){	
			case R_F2:	
				outb(0x40,pHwDeviceExtension->BaseIoPort0 + 0x0b);	
				outb(pScsiCmd->cmnd[2],pHwDeviceExtension->BaseIoPort0 + 0x00B1);//register addr	
				outb((status|WR|IFM),pHwDeviceExtension->BaseIoPort0 + 0x00B0);	//clear intr				
				pHwDeviceExtension->I2C_STEP = R_F3;
				break;
			case R_F3:
				outb(0x40,pHwDeviceExtension->BaseIoPort0 + 0x0b);//clear intr
				outb(pScsiCmd->cmnd[1],pHwDeviceExtension->BaseIoPort0 + 0x00B1);//device id	
				outb((status|WR|IFM),pHwDeviceExtension->BaseIoPort0 + 0x00B0);
				pHwDeviceExtension->I2C_STEP = R_F4;
				break;
			case R_F4:
				outb(0x40,pHwDeviceExtension->BaseIoPort0 + 0x0b);//clear intr
				outb((status|RD|IFM),pHwDeviceExtension->BaseIoPort0 + 0x00B0);
				pHwDeviceExtension->I2C_STEP = R_F5;
				break;
			case R_F5:
				len = 8;
				outb(0x40,pHwDeviceExtension->BaseIoPort0 + 0x0b);//clear intr				
				temp[0] = inb(pHwDeviceExtension->BaseIoPort0 + 0x00B2);//received data
				pHwDeviceExtension->I2C_STEP = NONE;
				if ( (scsi_sg_count(pScsiCmd)) && (len > 0) ) {
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
					memcpy(sg_list->address,temp,len);
#else
					local_irq_save(km_flags);
					km_buffer = kmap_atomic(sg_page(sg_list),KM_USER0) + sg_list->offset;
	//				km_buffer = kmap_atomic(sg_list->page,KM_USER0) + sg_list->offset;
					memcpy(km_buffer,temp,len);
					kunmap_atomic(km_buffer - sg_list->offset,KM_USER0);
					local_irq_restore(km_flags);
#endif
				}
#ifdef KERNEL_V2625
				
#else
				else {
					memcpy(pScsiCmd->request_buffer,temp,len);
				}
#endif
				break;
			case W_F1:
				break;
			case W_F2:
				break;
			case W_F3:
				break;
		}
		spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		return;
#else
		return IRQ_NONE;
#endif
		
	}else if ((int_status = inw(pHwDeviceExtension->BaseIoPort0 + 0x08)) == 0) {
		// It's not our interrupt
		spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		return;
#else
		return IRQ_NONE;
#endif
	}
	if (int_status & 0x0100) {
		// IDE interrupt income
		pIdeChannel = &(pHwDeviceExtension->IdeChannel);
		// clear interrupt
		outb(0x00,pIdeChannel->IdePortPSFF);					// STOP DMA
		status = inb(pIdeChannel->IdePortPSFF + 0x0002) | 0x04;
		outb(status,pIdeChannel->IdePortPSFF + 0x0002);
		status = inb(pIdeChannel->IdePortP1XX + 0x0007);			// clear PCI interrupt
		if (pIdeChannel->pCurRequest != 0) {
			pWorkingTable	= pIdeChannel->pCurRequest;
			pIdeDevice	= pWorkingTable->pDevice;
			pScsiCmd	= pWorkingTable->pWorkingScsiCmd;
			if (pIdeDevice->FG_AtapiDevice == 0) {
				// IDE hard disk
				HardDiskInterrupt(pHwDeviceExtension,pIdeChannel);
			} else {
				// IDE atapi device
				CdRomInterrupt(pHwDeviceExtension,pIdeChannel);
			}
			if (pScsiCmd->result != STATUS_PENDING) {
				pIdeChannel->QueueCount--;
				pIdeDevice->QueueCount--;
				pIdeChannel->pCurRequest = 0;
				ReleaseWorkingTable(pWorkingTable);
				pScsiCmd->scsi_done(pScsiCmd);
			}
			while((pIdeChannel->pCurRequest == 0) && (pIdeChannel->pPendingIO != 0)) {
				pIdeChannel->pCurRequest = pIdeChannel->pPendingIO;
				pIdeChannel->pPendingIO  = pIdeChannel->pPendingIO->pNextTable;
				pIdeChannel->pCurRequest->pNextTable = 0;
				pIdeDevice = pIdeChannel->pCurRequest->pDevice;
				if (pIdeDevice->FG_AtapiDevice) {
					ExecuteIdeATAPI(pHwDeviceExtension,pIdeChannel);
				} else {
					ExecuteIdeATA(pHwDeviceExtension,pIdeChannel);
				}
				pScsiCmdX = pIdeChannel->pCurRequest->pWorkingScsiCmd;
				if (pScsiCmdX->result == SAM_STAT_CHECK_CONDITION) {
					pIdeChannel->QueueCount--;
					pIdeDevice->QueueCount--;
					ReleaseWorkingTable(pIdeChannel->pCurRequest);
					pIdeChannel->pCurRequest = 0;
					pScsiCmdX->scsi_done(pScsiCmdX);
				}
			}
		}
		spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		return;
#else
		return IRQ_HANDLED;
#endif
	}
	pSataChannel = &(pHwDeviceExtension->SataChannel[0]);
	for (i = 0 ; i < MAX_SATA_CHANNEL ; i++) {
		if (int_status & 0x0001) {
			interrupt_status	= inl(pSataChannel->BaseIoPort + 0x0010);
			D2H_status	= inl(pSataChannel->BaseIoPort + 0x0070);
			D2H_status	= (D2H_status & 0x11111110) | (interrupt_status & 0x00000001); //D2H int flag & D2H Reg FIS int
			if ((interrupt_status & 0x00400308) || (D2H_status)) {
				break;
			}
		}
		int_status >>= 1;
		pSataChannel++;
	}
	if (i >= MAX_SATA_CHANNEL) {
		spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		return;
#else
		return IRQ_NONE;
#endif
	}
	//================================================================
	// SATA interrupt income
	//================================================================
	while ((interrupt_status = inl(pSataChannel->BaseIoPort + 0x0010)) == 0xFFFFFFFF) {
		printk("interrupt_status get wrong value\n\r");
		udelay(100);
		cpu_relax();
	}
	
	if (interrupt_status & 0x0001FE08) {
		// SET DEVICE BITS FIS , Queue command complete 
		CompleteMapSDBFG = inl(pSataChannel->BaseIoPort + 0x0044);
		PortMultiplier = 0;
		if (interrupt_status & 0x0001FE00) {
			// some one send status 0x41 back(error bit assert)
			pSataChannel->FG_ErrPMx |= ((unsigned char)(interrupt_status >> 9));
			pSataChannel->FG_StopIO = 0x01;
			outl(0x0001FE00,pSataChannel->BaseIoPort + 0x0010);
			pSataChannel->FG_ErrSDB = 1;
		} else {
			CompleteMap = CompleteMapSDBFG;
			for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
				if (CompleteMap & 0x00000001) {
					// found a command complete
					pWorkingTable = pSataChannel->pExecuteIO[i];
					if (pWorkingTable) {
						pSataDevice = pWorkingTable->pDevice;
						pReciveFIS  = pSataChannel->pReciveFIS + pSataDevice->FG_PMId;
						pScsiCmd	   = pWorkingTable->pWorkingScsiCmd;
			//			printk("q command %x end status %x\n",pScsiCmd->cmnd[0],pReciveFIS->SetDeviceBitsFIS.Status);	
						if (pReciveFIS->SetDeviceBitsFIS.Status & 0x01) {
							// command execute error
							pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
							ConvertSenseKey(pSataChannel,pWorkingTable,pReciveFIS->Device2HostFIS.Error);
						} else {
							
							pScsiCmd->result = SAM_STAT_GOOD;
							pSataDevice->SenseKey	= 0;
							pSataDevice->SenseAsc	= 0;
							pSataDevice->SenseAscQ	= 0;
						}
						pSataChannel->QueueCount--;
						pSataDevice->QueueCount--;
						if (pWorkingTable->use_sg) {
							sg_list = (struct scatterlist *)pWorkingTable->request_buffer;

			        			pci_unmap_sg(pHwDeviceExtension->pPciDev,
									sg_list,
									pWorkingTable->use_sg,
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
									scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
# else
									pScsiCmd->sc_data_direction
# endif
								   );

						} else {
							pci_unmap_single(pHwDeviceExtension->pPciDev,
									pWorkingTable->pDmaSingleMapAddr,
									pWorkingTable->request_bufflen,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
									scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
#else
									pScsiCmd->sc_data_direction
#endif
								       );
						}
						ReleaseWorkingTable(pWorkingTable);
						pScsiCmd->scsi_done(pScsiCmd);
						pSataChannel->pExecuteIO[i] = 0;
					} else {
						pSataChannel->pExecuteIO[i] = 0;	//??
					}
				}
				CompleteMap >>= 1;
				if (CompleteMap == 0) {
					break;
				}	
			}
			// clear SDBFG bits
			outl(CompleteMapSDBFG,pSataChannel->BaseIoPort + 0x0044);
			// clear interrupt flag
			outl(0xFFFE01F6,pSataChannel->BaseIoPort + 0x0010);
		}
	} else if (D2H_status) {
		if (pSataChannel->FG_StopIO == 0) {
			// NCQ Hard Disk
			for (i = 0 ; i < 8 ; i++) {
				if (D2H_status & (0x00000001 << (i * 4))) {
					pReciveFIS = pSataChannel->pReciveFIS + i;
					if (pReciveFIS->SetDeviceBitsFIS.Status & 0x01) {
						// command error
						pSataChannel->FG_ErrPMx |= (0x01 << i);
						pSataChannel->FG_StopIO = 0x01;
						pSataChannel->FG_ErrD2H = 1;
					}
				}
			}
			//=== clear interrupt ==========================================================================
			if (D2H_status & 0x11111110) {
				// clear interrupt flag for PM1 - PM7
				outl((D2H_status & 0xFFFFFFF0),pSataChannel->BaseIoPort + 0x0070);
			}
			if (D2H_status & 0x00000001) {
				// clear interrupt flage for PM0
				outl(0xEEEEEEE1,pSataChannel->BaseIoPort + 0x0010);
			}
		} else {
			// None NCQ Hard Disk/ATAPI device
			pWorkingTable	= pSataChannel->pExecuteIO[0];
			pSataDevice	= pWorkingTable->pDevice;
			pReciveFIS	= ((pSataChannel->pReciveFIS) + (pSataDevice->FG_PMId));
			pScsiCmd		= pWorkingTable->pWorkingScsiCmd;
		//	printk("d2h command %x end status %x\n",pScsiCmd->cmnd[0],pReciveFIS->Device2HostFIS.Status);
			if (pSataDevice->FG_AtapiDevice) { // CDROM/DVD
				if (pReciveFIS->Device2HostFIS.Status & 0x01) {
					// command execute error
					pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
				} else {					
					switch (pScsiCmd->cmnd[0]) {
					case REQUEST_SENSE:
						sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
						if (pWorkingTable->use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
							pBuf  = sg_list->address;
#else
							local_irq_save(km_flags);
							km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//							km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
							pBuf = km_buffer;
#endif
						} else {
							pBuf = pWorkingTable->request_buffer;
						}
						if ((pBuf[2] == 0x06) && (pBuf[12]==0x28)) {
							pSataDevice->DeviceBlockSize = 2048;
						} else if ((pBuf[2] == 0x02) && (pBuf[12]==0x3a)) {
							pSataDevice->DeviceBlockSize = 2048;
						}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#else
						if (pWorkingTable->use_sg) {
							kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
							local_irq_restore(km_flags);
						}
#endif
						break;
				 	case INQUIRY:
						sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
						if (pWorkingTable->use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
							pBuf  = sg_list->address;
#else
							local_irq_save(km_flags);
							km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//							km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
							pBuf = km_buffer;
#endif
						} else {
							pBuf = pWorkingTable->request_buffer;
						}
						pInquirybuf = (PUINQUIRYDATA)pBuf;
						pInquirybuf->VersionInfo &= 0xF8;
						pInquirybuf->VersionInfo |= 0x02;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#else
						if (pWorkingTable->use_sg) {
							kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
							local_irq_restore(km_flags);
						}
#endif
						break;
/*
					case 0x46:
						SrbDataBuffer = (unsigned char *)pWorkingTable->request_buffer;
						sg_list = (struct scatterlist *)(pWorkingTable->pOrgRequesrBuff);
						if (pWorkingTable->old_use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
							SrbExtOriginalBuffer  = sg_list->address;
#else
							local_irq_save(km_flags);
							km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//							km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
							SrbExtOriginalBuffer = km_buffer;
#endif
						} else {
							SrbExtOriginalBuffer = pWorkingTable->pOrgRequesrBuff;
						}
						for (i = 0 ; i < pWorkingTable->request_bufflen ; i++) {
							*SrbExtOriginalBuffer = *SrbDataBuffer;
							SrbExtOriginalBuffer++;
							SrbDataBuffer++;
						}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#else
						if (pWorkingTable->old_use_sg) {
							kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
							local_irq_restore(km_flags);
						}
#endif
						break;
*/
					case MODE_SENSE_10:		// mode sense 10
						SrbDataBuffer = (unsigned char *)pWorkingTable->request_buffer;
						sg_list = (struct scatterlist *)(pWorkingTable->pOrgRequesrBuff);
						km_sg_used = 0;
						if (pWorkingTable->old_use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
							SrbExtOriginalBuffer  = sg_list->address;
#else
							local_irq_save(km_flags);
							km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//							km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
							SrbExtOriginalBuffer = km_buffer;
							km_sg_used = 1;
#endif
						} else {
							SrbExtOriginalBuffer = pWorkingTable->pOrgRequesrBuff;
						}
						BlockLen = pSataDevice->DeviceBlockSize;
						word = ((((unsigned short)SrbDataBuffer[0]) << 8) & 0xff00)+
						       ((((unsigned short)SrbDataBuffer[1])     ) & 0x00ff);
						if (pWorkingTable->OrgCmnd[0] == MODE_SENSE) {
							// rebuild mode parameter header
							SrbExtOriginalBuffer[0] = word - 3;		// data length	
							SrbExtOriginalBuffer[1] = SrbDataBuffer[2];	// medium type
							SrbExtOriginalBuffer[2] = 0;		// device-specific parameter
							SrbExtOriginalBuffer[3] = 0;		// block descriptor length
							i=4;
							// rebuild block descriptor
							if ((pWorkingTable->OrgCmnd[1] & 0x08) == 0) {
								SrbExtOriginalBuffer[0] += 8;
								// set block descriptor length
								SrbExtOriginalBuffer[3] = 8;
								// build descriptor block
								//SrbExtOriginalBuffer[4] = 0x83;
								for (i = 4 ; i <= 8 ; i++) {
									SrbExtOriginalBuffer[i] = 0;
								}
								SrbExtOriginalBuffer[9]  = (unsigned char)(BlockLen>>16);
								SrbExtOriginalBuffer[10] = (unsigned char)(BlockLen>>8);
								SrbExtOriginalBuffer[11] = (unsigned char)BlockLen;
								if ((pWorkingTable->OrgCmnd[2] & 0xc0) == 0x40) {
									// PC = 0x01
									SrbExtOriginalBuffer[9]  = 0xFF;
									SrbExtOriginalBuffer[10] = 0xFF;
									SrbExtOriginalBuffer[11] = 0xFF;
								}
								i = 12;
							}
							SrbExtOriginalBuffer += (unsigned char)i;
							SrbDataBuffer += 8;
							// moving page list data
							for (i = 0 ; i < (pWorkingTable->pOrgRequestBuffLen - 12) ; i++) {
								*SrbExtOriginalBuffer = *SrbDataBuffer;
								SrbExtOriginalBuffer++;
								SrbDataBuffer++;
							}
						} else {
							// MODE_SENSE_10
							if (	(((unsigned short)(pWorkingTable->request_bufflen)) > (word + 16)) &&
									 ((pScsiCmd->cmnd[1] & 0x08) == 0)
							   ) {
								// host buffer > data + block descriptor length + head len
								// and DBD bit = 0
								word = word + 8;			// add block descriptor length
								// rebuild parameter header
								SrbExtOriginalBuffer[0] = (unsigned char)(word >> 8);	// data length	
								SrbExtOriginalBuffer[1] = (unsigned char)word;	
								for (i = 2 ; i <= 5 ; i++) {
									SrbExtOriginalBuffer[i] = SrbDataBuffer[i];
								}
								SrbExtOriginalBuffer[6] = 0;
								SrbExtOriginalBuffer[7] = 8;
								word = word - 8;
								// copy sense data
								for (i = 0 ; i < word ; i++) {
									SrbExtOriginalBuffer[i+16] = SrbDataBuffer[i+8];
								}
								// rebuild block descriptor
								for (i = 8 ; i <= 12 ; i++) {
									SrbExtOriginalBuffer[i] = 0;
								}	
								SrbExtOriginalBuffer[13] = (unsigned char)(BlockLen>>16);
								SrbExtOriginalBuffer[14] = (unsigned char)(BlockLen>>8);
								SrbExtOriginalBuffer[15] = (unsigned char)BlockLen;
								if ((pWorkingTable->OrgCmnd[2] & 0xc0) == 0x40) {
									SrbExtOriginalBuffer[13] = 0xFF;
									SrbExtOriginalBuffer[14] = 0xFF;
									SrbExtOriginalBuffer[15] = 0xFF;
								}
								pWorkingTable->OrgCmnd[0] = 0;
							} else {
								word +=2; // total transfer length
								if (word > pWorkingTable->request_bufflen) {
									word = (unsigned short)(pWorkingTable->request_bufflen);
								}
								for (i = 0 ; i < word ; i++) {
									SrbExtOriginalBuffer[i] = SrbDataBuffer[i];
								}	
							}
						}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
#else
						if (km_sg_used) {
							kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
							local_irq_restore(km_flags);
						}
#endif						
						break;
					}
					pScsiCmd->result = SAM_STAT_GOOD;
					pSataDevice->SenseKey = 0;
					pSataDevice->SenseAsc = 0;
					pSataDevice->SenseAscQ = 0;
				}
				if (pWorkingTable->request_bufflen) {
					if (pWorkingTable->use_sg) {
						sg_list = (struct scatterlist *)pWorkingTable->request_buffer;

		        			pci_unmap_sg(pHwDeviceExtension->pPciDev,
								sg_list,
								pWorkingTable->use_sg,
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
								scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
# else
								pScsiCmd->sc_data_direction
# endif
							   );

					} else {
						pci_unmap_single(pHwDeviceExtension->pPciDev,
								pWorkingTable->pDmaSingleMapAddr,
								pWorkingTable->request_bufflen,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
								scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
#else
								pScsiCmd->sc_data_direction
#endif
							       );
					}
				}
				RestoreCmdATAPI(pWorkingTable);
				pSataChannel->QueueCount--;
				pSataChannel->FG_StopIO = 0;
				pSataDevice->QueueCount--;
				pSataChannel->pExecuteIO[0] = 0;
				ReleaseWorkingTable(pWorkingTable);
				pScsiCmd->scsi_done(pScsiCmd);
				// clear interrupt flag
				if (D2H_status & 0x11111110) {
					// clear interrupt flag for PM1 - PM7
					outl((D2H_status & 0x33333330),pSataChannel->BaseIoPort + 0x0070);
				} else {
					// clear interrupt flage for PM0
					outb(0x03,pSataChannel->BaseIoPort + 0x0010);
				}
			} else {
				// Hard Disk
				if (pReciveFIS->Device2HostFIS.Status & 0x01) {
					// command execute error
					pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
					ConvertSenseKey(pSataChannel,pWorkingTable,pReciveFIS->Device2HostFIS.Error);
					pSataDevice->FG_InternalError = 1;
				} else {
					pScsiCmd->result		= SAM_STAT_GOOD;
					pSataDevice->SenseKey	= 0;
					pSataDevice->SenseAsc	= 0;
					pSataDevice->SenseAscQ	= 0;
				}
				if(pWorkingTable->FG_NonData){

				}else{
					if (pWorkingTable->use_sg) {
						sg_list = (struct scatterlist *)pWorkingTable->request_buffer;
	
						pci_unmap_sg(pHwDeviceExtension->pPciDev,
								sg_list,
								pWorkingTable->use_sg,
	# if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
								scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
	# else
								pScsiCmd->sc_data_direction
	# endif
							);
	
					} else {
						pci_unmap_single(pHwDeviceExtension->pPciDev,
								pWorkingTable->pDmaSingleMapAddr,
								pWorkingTable->request_bufflen,
	#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
								scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
	#else
								pScsiCmd->sc_data_direction
	#endif
							);
					}
				}
				pSataChannel->QueueCount--;
				pSataChannel->FG_StopIO = 0;
				pSataDevice->QueueCount--;
				pSataChannel->pExecuteIO[0] = 0;
				ReleaseWorkingTable(pWorkingTable);
				pScsiCmd->scsi_done(pScsiCmd);
				// clear interrupt flag
				if (D2H_status & 0x11111110) {
					// clear interrupt flag for PM1 - PM7
					outl((D2H_status & 0x11111110),pSataChannel->BaseIoPort + 0x0070);
				} else { // clear interrupt flage for PM0
					outb(0x01,pSataChannel->BaseIoPort + 0x0010);
				}
			}
		}
	}
	if (pSataChannel->FG_ErrPMx) {
		for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
			if (pSataChannel->pExecuteIO[i]) {
				pWorkingTable = pSataChannel->pExecuteIO[i];
				pSataDevice = pWorkingTable->pDevice;
				if ((pSataChannel->FG_ErrPMx & (0x01 << (pSataDevice->FG_PMId))) == 0) {
					// found a none error command is still executeing
					break;
				}
			}
		}
		if (i >= MAX_COMMAND_QUEUE_CH) {
			// all normal command were finished
			for (j = 0 ; j < 8 ; j++) {
				if (pSataChannel->FG_ErrPMx & (0x01 << j)) {
					PortMultiplier = (unsigned char)j;
					//==========================================================================================
					// disable INTA#
					//==========================================================================================
					outb(0,pHwDeviceExtension->BaseIoPort0 + 0x0004);
					cancel_ci = 0;
					for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
						// found out the IO match current PM number;
						if (pSataChannel->pExecuteIO[i]) {
							if (((P_DeviceInfo)((pSataChannel->pExecuteIO[i])->pDevice))->FG_PMId == PortMultiplier) {
								cancel_ci |= (0x00000001 << i);
							}
						}
					}
					//==========================================================================================
					outb(PortMultiplier,pSataChannel->BaseIoPort+0x003F);	// set Port multiple number to b27-24
					outl(cancel_ci,pSataChannel->BaseIoPort + 0x0040);	// force clear CI bits
					//==========================================================================================
					for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
						if (( cancel_ci >> i) & 0x00000001) {
							pWorkingTable1 = pSataChannel->pExecuteIO[i];
							pSataChannel->pExecuteIO[i] = 0;
							// insert this Io to first Pending IO
							pWorkingTable1->pNextTable = pSataChannel->pPendingIO;
							pWorkingTable1->pPrevTable = 0;
							pSataChannel->pPendingIO = pWorkingTable1;
							if (pWorkingTable1->pNextTable) {
								((P_WorkingTable)(pWorkingTable1->pNextTable))->pPrevTable = pWorkingTable1;
							}
						}
					}
					PMxReset(pSataChannel,PortMultiplier);
					//==========================================================================================
					outb(PortMultiplier,pSataChannel->BaseIoPort+0x003F);	// set Port multiple number to b27-24
					outl(cancel_ci,pSataChannel->BaseIoPort + 0x0040);	// force clear CI bits
					//==========================================================================================
					// clear BUSY & DRQ bit to release channel
					//==========================================================================================
					status = inb(pSataChannel->BaseIoPort + 0x0018) | 0x08;
					outb(status,pSataChannel->BaseIoPort + 0x0018);
					while(inb(pSataChannel->BaseIoPort + 0x0018) & 0x08) {
						udelay(10);
						cpu_relax();
					}
					//==========================================================================================
					// Enable INTA#
					//==========================================================================================
					outb(0x02,pHwDeviceExtension->BaseIoPort0 + 0x0004);// Enable INTA#
					pSataChannel->FG_StopIO = 0;
					pSataChannel->FG_ErrPMx = 0;
					pSataChannel->FG_ErrSDB = 0;
					pSataChannel->FG_ErrD2H = 0;
				}
			}
		}
	}
	if (pSataChannel->FG_StopIO) {
		spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
		return;
#else
		return IRQ_HANDLED;
#endif
	}
	if (pSataChannel->pPendingIO) {
		pWorkingTable = pSataChannel->pPendingIO;
		pSataDevice = pWorkingTable->pDevice;
		
		if ((pSataDevice->FG_QueueSupported) && (!pWorkingTable->FG_Updata)) {
			j = (int)(pSataChannel->QueueStartTag);
			for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
				if (pSataChannel->pExecuteIO[j] == 0) {
					// found empty Queue tag
					break;
				}
				if ( ++j >= MAX_COMMAND_QUEUE_CH) {
					j = 0;
				}
			}
			pSataChannel->pExecuteIO[j] = pWorkingTable;
			pWorkingTable->TagID	  = (unsigned char)j;
			if ( ++j >= MAX_COMMAND_QUEUE_CH) {
				j = 0;
			}
			pSataChannel->QueueStartTag = (unsigned char)j;
			ExecuteSataATA(pHwDeviceExtension,pWorkingTable);
			ChannelIoRemove(pWorkingTable);
		} else {
			// search executing IO
			for (i = 0 ; i < MAX_COMMAND_QUEUE_CH ; i++) {
				if (pSataChannel->pExecuteIO[i] != 0) {
					// found a executing IO
					break;
				}
			}
			if (i >= MAX_COMMAND_QUEUE_CH) {
				// send IO to Device
				pSataChannel->pExecuteIO[0] = pWorkingTable;
				pWorkingTable->TagID	  = 0;
				pSataChannel->FG_StopIO	  = 1;
				ExecuteSataATA(pHwDeviceExtension,pWorkingTable);
				ChannelIoRemove(pWorkingTable);
			}
		}
	}
	if (interrupt_status & 0x00400040) {
		// PhyRdy signal changed & new ComInit incomes.
		outl(0xFFFFFFFF,pSataChannel->BaseIoPort+0x30);	// clear interrupt
	}
	spin_unlock_irqrestore(&(pHwDeviceExtension->spin_lock),flags);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	return;
#else
	return IRQ_HANDLED;
#endif
}
//========================================================================
static void adapter_timer(unsigned long para)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension;
	unsigned char FG_SendWorkingTable;
	
	pHwDeviceExtension = (PHW_DEVICE_EXTENSION)para;
	FG_SendWorkingTable = 0;
	
	if (FG_SendWorkingTable) {
//		SendWorkingTableOut(pHwDeviceExtension);
	}
	if (!(pHwDeviceExtension->FG_TimerStop)) {
		(pHwDeviceExtension->AdapterTimer).expires = jiffies + TIMER_SCALE;
		add_timer(&(pHwDeviceExtension->AdapterTimer));
	}
	return;
}
//========================================================================
//	IDE function
//========================================================================
static unsigned char SendIdentifyCmd2IDE(P_DeviceInfo pDeviceInfo,void *pBuffer,unsigned char IdentifyCmd)
{
	P_Channel pIdeChannel;
	PUIdentify pIdentifyBuff;
	unsigned short *pTempBuf;
	int i;
	unsigned char status;

	pIdeChannel = pDeviceInfo->pChannel;
	pIdentifyBuff = (PUIdentify)pBuffer;
	outb(pDeviceInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	udelay(300);
	status = inb(pIdeChannel->IdePortP1XX + 0x0007);
	if (  (status == 0x7F) || (status & 0x80)   ) {
		return 0;
	}
	//  issure Identify command
	outb(pDeviceInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	outb(IdentifyCmd,pIdeChannel->IdePortP1XX + 0x0007);
	for (i = 0 ; i < 20000 ; i++) {
		// wait 2 S
		cpu_relax();
		udelay(100);		// delay 100us
		status = inb(pIdeChannel->IdePortP1XX + 0x0007);
		if (status & 0x80)
			continue;	// BUS BUSY
		if (status & 0x08)	
			break;		// DRQ ASSERT
		if (status & 0x01)	
			break;		// ERROR BIT ASSERT
	}
	if (  (status == 0x08)||(status == 0x58)||(status == 0x48)   ) {
		pTempBuf = (unsigned short *)pBuffer;
		for (i = 0 ; i < 256 ; i++) {
			*pTempBuf = (unsigned short)inw(pIdeChannel->IdePortP1XX);
			pTempBuf++;
		}
		status = inb(pIdeChannel->IdePortPSFF + 0x0002) | 0x04;
		outb(status,pIdeChannel->IdePortPSFF + 0x0002);
		status = inb(pIdeChannel->IdePortP1XX+ 0x0007);
		return 1;
	}	
	status = inb(pIdeChannel->IdePortPSFF + 0x0002) | 0x04;
	outb(status,pIdeChannel->IdePortPSFF + 0x0002);
	status = inb(pIdeChannel->IdePortP1XX + 0x0007);
	return 0;
}
//===================================================================================================
static unsigned char PioTimming[5] = { 0x6e,0x58,0x44,0x33,0x31 };
static void SetPIOfeature(P_DeviceInfo pHardDiskInfo,PUIdentify pIdentifyBuff,unsigned char MAXMODE)
{	
	unsigned short PIOxferclk;
	unsigned char PIOmode,status;
	int i,j;
	P_Channel pIdeChannel;

	pIdeChannel = pHardDiskInfo->pChannel;
	pHardDiskInfo->MultiBlk = 1;
	if (pIdentifyBuff->MultipleBlk > 1) {
		// set multiple block
		outb(pHardDiskInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
		for (i = 0 ; i < 100000 ; i++) {
			// wait 5 seconds for bus free
			udelay(50);
			cpu_relax();
			status = inb(pIdeChannel->IdePortP1XX + 0x0007);
			if (status & 0x80)
				continue;
			break;
		}
		if ((status & 0x80) == 0) {
			outb(pIdentifyBuff->MultipleBlk,pIdeChannel->IdePortP1XX + 0x0002);
			outb(0xC6,pIdeChannel->IdePortP1XX + 0x0007);
			for (i = 0 ; i < 200000 ; i++) {
				// wait 10 seconds for command complete
				udelay(50);
				cpu_relax();
				status = inb(pIdeChannel->IdePortP1XX + 0x0007);
				if (status & 0x80)
					continue;
				break;
			}
			if ((status & 0x89) == 0) {
				pHardDiskInfo->MultiBlk = pIdentifyBuff->MultipleBlk;
			}
		}
	}// end of set multiple block
	if (pIdentifyBuff->PIOMode & 0x04) {
		pHardDiskInfo->PIO_Mode	= 2;
	} else if (pIdentifyBuff->PIOMode & 0x02) {
		pHardDiskInfo->PIO_Mode	= 1;
	} else {
		pHardDiskInfo->PIO_Mode	= 0;
	}
	if (pIdentifyBuff->FG_ValidMap & 0x0002) {
		// set PIO mode depanded on minimum PIO transfer cycle time
		PIOxferclk = pIdentifyBuff->MinPioTransCycle / 30;
		if (( pIdentifyBuff->MinPioTransCycle % 30 ) > 0) {
			PIOxferclk++;
		}
		if (PIOxferclk <= 4) {
			// 120 ns
			pHardDiskInfo->PIO_Mode	= 4;
		} else if (PIOxferclk <= 6) {
			// 180 ns
			pHardDiskInfo->PIO_Mode	= 3;
		} else if (PIOxferclk <= 8) {
			// 240 ns
			pHardDiskInfo->PIO_Mode	= 2;
		} else if (PIOxferclk <= 13) {
			// 383 ns
			pHardDiskInfo->PIO_Mode	= 1;
		} else {
			// 600 ns
			pHardDiskInfo->PIO_Mode	= 0;
		}
	}
	if (pHardDiskInfo->PIO_Mode > MAXMODE) {
		pHardDiskInfo->PIO_Mode = MAXMODE;
	}
	pHardDiskInfo->PIOxferclk = PioTimming[pHardDiskInfo->PIO_Mode];
	// end of set PIO mode
	// set PIO mode feature
	PIOmode = pHardDiskInfo->PIO_Mode | 0x08;
	// wait for bus free
	outb(pHardDiskInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	for (j = 0 ; j < 2 ; j++) {
		for (i = 0 ; i < 100000 ; i++) {
			// wait 5 seconds for bus free
			udelay(50);
			cpu_relax();
			status = inb(pIdeChannel->IdePortP1XX + 0x0007);
			if (status & 0x80)
				continue;
			break;
		}
		if ((status & 0x80) == 0) {
			// bus is free now
			outb(0x03,pIdeChannel->IdePortP1XX + 0x0001);
			outb(PIOmode,pIdeChannel->IdePortP1XX + 0x0002);
			outb(0xEF,pIdeChannel->IdePortP1XX + 0x0007);
			// wait for command complete
			for (i = 0 ; i < 200000 ; i++) {
				// wait 10 seconds for command complete
				udelay(50);
				cpu_relax();
				status = inb(pIdeChannel->IdePortP1XX + 0x0007);
				if (status & 0x80)
					continue;
				break;
			}
			if ((status & 0x89) == 0) {
				return ;
			}
		}
	}
	pHardDiskInfo->PIO_Mode = 0;
	pHardDiskInfo->PIOxferclk = 0x6d;
	return;
}
//===================================================================================================
static void GetUltraMode(P_DeviceInfo pHardDiskInfo,unsigned char UltraMap)
{
	pHardDiskInfo->DMA_Mode = 0;
	if (UltraMap & 0x40) {
		// Ultra 133
		pHardDiskInfo->DMA_Mode	= 7;
	} else if (UltraMap & 0x20) {
		// Ultra 100
		pHardDiskInfo->DMA_Mode	= 6;
	} else if (UltraMap & 0x10) {
		// Ultra 66
		pHardDiskInfo->DMA_Mode	= 5;
	} else if (UltraMap & 0x8) {
		// Ultra 44
		pHardDiskInfo->DMA_Mode	= 4;
	} else if (UltraMap & 0x04) {
		// Ultra 33
		pHardDiskInfo->DMA_Mode	= 3;
	} else if (UltraMap & 0x02) {
		// Ultra 1 
		pHardDiskInfo->DMA_Mode	= 2;
	} else if (UltraMap & 0x01) {
		// Ultra 0 
		pHardDiskInfo->DMA_Mode	= 1;
	} else {
		// no ultra support
		return;
	}
}
//===================================================================================================
static void SetUltra(P_DeviceInfo pHardDiskInfo,unsigned char MAXMODE)
{
	unsigned char	Ultramode,status;
	P_Channel	pIdeChannel;
	int		i,j;

	pIdeChannel = pHardDiskInfo->pChannel;
	// set DMA mode feature
	if (pHardDiskInfo->DMA_Mode == 0) {
		return;
	}
	Ultramode = pHardDiskInfo->DMA_Mode;
	Ultramode--;
	if (Ultramode > MAXMODE) {
		Ultramode = MAXMODE;
		pHardDiskInfo->DMA_Mode = Ultramode+1;
	}
	Ultramode |= 0x40;
	// wait for bus free
	outb(pHardDiskInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	for (j = 0 ; j < 2 ; j++) {
		for (i = 0 ; i < 100000 ; i++) {
			// wait 5 seconds for bus free
			cpu_relax();
			udelay(50);
			status = inb(pIdeChannel->IdePortP1XX + 0x0007);
			if (status & 0x80)
				continue;
			break;
		}
		if ((status & 0x80) == 0) {
			outb(0x03,pIdeChannel->IdePortP1XX + 0x0001);
			outb(Ultramode,pIdeChannel->IdePortP1XX + 0x0002);
			outb(0xEF,pIdeChannel->IdePortP1XX + 0x0007);
			// wait for command complete
			for (i = 0 ; i < 200000 ; i++) {
				// wait 10 seconds for command complete
				cpu_relax();
				udelay(50);
				status = inb(pIdeChannel->IdePortP1XX + 0x0007);
				if (status & 0x80)
					continue;
				break;
			}
			if ((status & 0x89) == 0) {
				pHardDiskInfo->DMA_Mode |= 0xc0;
				return ;
			}
		}
	}
	pHardDiskInfo->DMA_Mode = 0;
	return;
}
//===================================================================================================
static void SetUltrafeature(P_DeviceInfo pHardDiskInfo,PUIdentify pIdentifyBuff,unsigned char MAXMODE)
{ 
	GetUltraMode(pHardDiskInfo,pIdentifyBuff->UltraModeMap);
	SetUltra(pHardDiskInfo,MAXMODE);
	return;
}
//===================================================================================================
static void SetDMAfeature(P_DeviceInfo pHardDiskInfo,PUIdentify PidentifyBuff,unsigned char MAXMODE)
{ 
	unsigned char DMAmode,status;
	int i;
	P_Channel pIdeChannel;

	
	pIdeChannel = pHardDiskInfo->pChannel;
	pHardDiskInfo->DMA_Mode = 0;
	if ((PidentifyBuff->MultiWordDMASupport & 0x04) > 0) {
		// 120ns
		pHardDiskInfo->DMA_Mode	= 2;
		pHardDiskInfo->DMAxferclk	= 0x31;  //(90ns/30ns)
	} else if ((PidentifyBuff->MultiWordDMASupport & 0x02) > 0) {
		// 150ns
		pHardDiskInfo->DMA_Mode	= 1;
		pHardDiskInfo->DMAxferclk	= 0x33;  //(90ns/90ns)
	} else {
		// 480ns
		pHardDiskInfo->DMA_Mode	= 0;
		pHardDiskInfo->DMAxferclk	= 0x6e;  //(180ns/420ns)
	}
	if (pHardDiskInfo->DMA_Mode > MAXMODE)
	{
		pHardDiskInfo->DMA_Mode	= MAXMODE;
	}
	// end of set DMA mode
	// set DMA mode feature
	DMAmode = pHardDiskInfo->DMA_Mode | 0x20;
	// wait for bus free
	outb(pHardDiskInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	for (i = 0 ; i < 10000 ; i++) {
		// wait 5 seconds for bus free
		cpu_relax();
		udelay(500);
		status = inb(pIdeChannel->IdePortP1XX + 0x0007);
		if (status & 0x80)
			continue;
		break;
	}
	if ((status & 0x80) == 0) {
		outb(0x03,pIdeChannel->IdePortP1XX + 0x0001);
		outb(DMAmode,pIdeChannel->IdePortP1XX + 0x0002);
		outb(0xEF,pIdeChannel->IdePortP1XX + 0x0007);
		// wait for command complete
		for (i = 0 ; i < 20000 ; i++) {
			// wait 10 seconds for command complete
			cpu_relax();
			udelay(500);
			status = inb(pIdeChannel->IdePortP1XX + 0x0007);
			if (status & 0x80)
				continue;
			break;
		}
		if ((status & 0x89) == 0) {
			pHardDiskInfo->DMA_Mode |= 0x80;
			return ;
		}
	}
	pHardDiskInfo->DMA_Mode = 0;
	return;
}
//===================================================================================================
static void SetChipIo(P_DeviceInfo pHardDiskInfo)
{
	P_Channel pIdeChannel = pHardDiskInfo->pChannel;
	unsigned char UltraMode,PioM,PioS;

	UltraMode = inb(pIdeChannel->IdeRegSpeed + 0x0003);
	if (pHardDiskInfo->HD_V1X6 & 0x10) { // slave device
		if(pHardDiskInfo->HddType == 0xC0){
			outb(pHardDiskInfo->PIOxferclk,pIdeChannel->IdeRegSpeed + 1);	// set PIO
			UltraMode &= 0x0F;			// keep master ultra mode
			UltraMode |= (((pHardDiskInfo->DMA_Mode) & 0x07) << 4);
		}else if(pHardDiskInfo->HddType == 0x80){
			outb(pHardDiskInfo->DMAxferclk,pIdeChannel->IdeRegSpeed + 1);
			UltraMode &= 0x0F;	
		}
		
	} else {
		// master device
		if(pHardDiskInfo->HddType == 0xC0){
			outb(pHardDiskInfo->PIOxferclk,pIdeChannel->IdeRegSpeed);	// set PIO
			UltraMode &= 0xF0;		// keep slave ultra mode
			UltraMode |= ((pHardDiskInfo->DMA_Mode) & 0x07);
		}else if(pHardDiskInfo->HddType == 0x80){
			outb(pHardDiskInfo->DMAxferclk,pIdeChannel->IdeRegSpeed);
			UltraMode &= 0xF0;
		}
	}
	outb(UltraMode,pIdeChannel->IdeRegSpeed + 3);	// set PIO
	PioM = inb(pIdeChannel->IdeRegSpeed);	// get master PIO
	PioS = inb(pIdeChannel->IdeRegSpeed + 1);	// get slave PIO
	if (PioM < PioS) {
		PioM = PioS;
	}
	outb(PioM,pIdeChannel->IdeRegSpeed + 2);	// set Command PIO
	return;
}
//===================================================================================================
static void ScanATA(PHW_DEVICE_EXTENSION DeviceExtension,P_DeviceInfo pHardDiskInfo)
{
	PUIdentify pIdentifyBuff;
	P_Channel pIdeChannel;
	int j;
	unsigned char TempBuf[512],status;

	pIdeChannel = pHardDiskInfo->pChannel;
	outb(pHardDiskInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	udelay(100);
	status = inb(pIdeChannel->IdePortP1XX + 0x0007);
	if (  (status == 0x7F) || (status & 0x80)   ) {
		return;
	}
	pIdentifyBuff = (PUIdentify)TempBuf;
	if (SendIdentifyCmd2IDE(pHardDiskInfo,pIdentifyBuff,0xEC)) {
		
		// got ATA device
		if (	  ((pIdentifyBuff->ConfigBitInfo & 0x8000) == 0)
			&&(pIdentifyBuff->FG_ValidMap & 0x0004)
			|| (pIdentifyBuff->ConfigBitInfo & 0x848A)
		   ) {	// only support ULTRA device
			pHardDiskInfo->FG_DeviceInstall	= 1;
			pHardDiskInfo->HD_V1X6		|= 0x40;
#if __LITTLE_ENDIAN
#else
			pIdentifyBuff->TotalLBA = ((pIdentifyBuff->TotalLBA >> 16) & 0x0000FFFF) +
							((pIdentifyBuff->TotalLBA << 16) & 0xFFFF0000);
#endif
			((pHardDiskInfo->Capacity).U64L).U64L_H = 0;
			((pHardDiskInfo->Capacity).U64C).U64C_L_HH = (u8)(pIdentifyBuff->TotalLBA >> 24);
			((pHardDiskInfo->Capacity).U64C).U64C_L_HL = (u8)(pIdentifyBuff->TotalLBA >> 16);
			((pHardDiskInfo->Capacity).U64C).U64C_L_LH = (u8)(pIdentifyBuff->TotalLBA >> 8);
			((pHardDiskInfo->Capacity).U64C).U64C_L_LL = (u8)(pIdentifyBuff->TotalLBA);
#if __LITTLE_ENDIAN
			for (j = 0 ; j < 40 ; j += 2) {
				pHardDiskInfo->ModelName[j]   = pIdentifyBuff->ModelName[j+1];
				pHardDiskInfo->ModelName[j+1] = pIdentifyBuff->ModelName[j];
			}
			for (j = 0 ; j < 20 ; j += 2) {
				pHardDiskInfo->SerialNo[j]   = pIdentifyBuff->SerialNo[j+1];
				pHardDiskInfo->SerialNo[j+1] = pIdentifyBuff->SerialNo[j];
			}
			for (j = 0 ; j < 8 ; j += 2) {
				pHardDiskInfo->FirmWareRev[j]   = pIdentifyBuff->FirmWareRev[j+1];
				pHardDiskInfo->FirmWareRev[j+1] = pIdentifyBuff->FirmWareRev[j];
			}
#else			
			for (j = 0 ; j < 20 ; j++) {
				pHardDiskInfo->SerialNo[j]	= pIdentifyBuff->SerialNo[j];
			}
			for (j = 0 ; j < 40 ; j++) {
				pHardDiskInfo->ModelName[j]	= pIdentifyBuff->ModelName[j];
			}
			for (j = 0 ; j < 8 ; j++) {
				pHardDiskInfo->FirmWareRev[j]	= pIdentifyBuff->FirmWareRev[j];
			}
#endif
			//=====================================================================================
			// check 48 bit LBA capability
			//=====================================================================================
			if (pIdentifyBuff->Word83 & 0x0400) {
#if __LITTLE_ENDIAN
#else
				pIdentifyBuff->LBA_48bits_High = ((pIdentifyBuff->LBA_48bits_High >> 16) & 0x0000FFFF) +
							((pIdentifyBuff->LBA_48bits_High << 16) & 0xFFFF0000);
				pIdentifyBuff->LBA_48bits_Low = ((pIdentifyBuff->LBA_48bits_Low >> 16) & 0x0000FFFF) +
							((pIdentifyBuff->LBA_48bits_Low << 16) & 0xFFFF0000);
#endif
				if ( ((pIdentifyBuff->LBA_48bits_Low == 0)	         && (pIdentifyBuff->LBA_48bits_High == 0))			||
				     ((pIdentifyBuff->LBA_48bits_Low == 0xFFFFFFFF) && (pIdentifyBuff->LBA_48bits_High == 0xFFFFFFFF))
				   ) {
					pHardDiskInfo->FG_Support48Bits = 0;
				} else {
					pHardDiskInfo->FG_Support48Bits = 1;
					((pHardDiskInfo->Capacity).U64C).U64C_H_HH = (u8)(pIdentifyBuff->LBA_48bits_High >> 24);
					((pHardDiskInfo->Capacity).U64C).U64C_H_HL = (u8)(pIdentifyBuff->LBA_48bits_High >> 16);
					((pHardDiskInfo->Capacity).U64C).U64C_H_LH = (u8)(pIdentifyBuff->LBA_48bits_High >> 8);
					((pHardDiskInfo->Capacity).U64C).U64C_H_LL = (u8)(pIdentifyBuff->LBA_48bits_High);
					((pHardDiskInfo->Capacity).U64C).U64C_L_HH = (u8)(pIdentifyBuff->LBA_48bits_Low >> 24);
					((pHardDiskInfo->Capacity).U64C).U64C_L_HL = (u8)(pIdentifyBuff->LBA_48bits_Low >> 16);
					((pHardDiskInfo->Capacity).U64C).U64C_L_LH = (u8)(pIdentifyBuff->LBA_48bits_Low >> 8);
					((pHardDiskInfo->Capacity).U64C).U64C_L_LL = (u8)(pIdentifyBuff->LBA_48bits_Low);
				}
			} else {
				pHardDiskInfo->FG_Support48Bits	= 0;
			}
//	printk("Capacity : %ld(%08lX) \n\r",pHardDiskInfo->Capacity.U64L.U64L_L,pHardDiskInfo->Capacity.U64L.U64L_L);
//	printk("Capacity : %ld(%08lX) \n\r",pIdentifyBuff->LBA_48bits_Low,pIdentifyBuff->LBA_48bits_Low);
			//=====================================================================================
			// set device's PIO & DMA feature
			//=====================================================================================
			pHardDiskInfo->DMA_Mode	= 0;			
			SetPIOfeature(pHardDiskInfo,pIdentifyBuff,_MAX_PIO_MODE);
			pHardDiskInfo->HddType = 0;
			if (pIdentifyBuff->FG_ValidMap & 0x0004)	/* support ULTRA mode(valid 88)*/
			{
				printk("ULTRA Capability %x\n",pIdentifyBuff->Capability);
				SetUltrafeature(pHardDiskInfo,pIdentifyBuff,_MAX_ULTRA);
				pHardDiskInfo->HddType = 0xC0;
			//	pHardDiskInfo->HddType = 0xC0;
			}
			else if (	  (pIdentifyBuff->Capability & 0x0100) 
					&&(pIdentifyBuff->MultiWordDMASupport&0x07)
					&&(pHardDiskInfo->DMA_Mode==0)
				   )
			{	
				printk("multiword DMA Capability %x\n",pIdentifyBuff->Capability);
				SetDMAfeature(pHardDiskInfo,pIdentifyBuff,0x0F);
				pHardDiskInfo->HddType = 0x80;
			//	pHardDiskInfo->HddType = 0x80;
			}
			SetUltrafeature(pHardDiskInfo,pIdentifyBuff,_MAX_ULTRA);
			pHardDiskInfo->DeviceBlockSize	= 512;
			pHardDiskInfo->FG_IdeExcept	= 0;
			pHardDiskInfo->A0Wait		= 0;
			printk("found ATA : %.30s\n",pHardDiskInfo->ModelName);
			return;
		}// end of ATA device
	}// end of send EC command
	
	pHardDiskInfo->FG_DeviceInstall = 0;
	return;
}
//========================================================================
static void ExecuteIdeATA(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pIdeChannel)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_WorkingTable pWorkingTable;
	P_DeviceInfo pDeviceInfo;
	Scsi_Cmnd *pScsiCmd;
	unsigned char V1X6;

	pWorkingTable = pIdeChannel->pCurRequest;
	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	pDeviceInfo = pWorkingTable->pDevice;
	BuildPrdTable(pHwDeviceExtension,pWorkingTable,pWorkingTable->pMailBox);
	if (pScsiCmd->result == SAM_STAT_CHECK_CONDITION) {
		return;
	}
	switch(pWorkingTable->XferCmd) {
	// execute 48 bits command
	case IDE_READ_DMA_EXT:		// READ DMA EXT
	case IDE_WRITE_DMA_EXT:		// WRITE DMA EXT
	case IDE_VERIFY_EXT:		// VERIFY EXT
		// set IDE register
		outb((((pWorkingTable->TransferLen).U64C).U64C_L_LH),pIdeChannel->IdePortP1XX + 0x0002);
		outb((((pWorkingTable->TransferLen).U64C).U64C_L_LL),pIdeChannel->IdePortP1XX + 0X0002);
		outb((((pWorkingTable->StartLBA).U64C).U64C_L_HH),pIdeChannel->IdePortP1XX + 0x0003);
		outb((((pWorkingTable->StartLBA).U64C).U64C_L_LL),pIdeChannel->IdePortP1XX + 0x0003);
		outb((((pWorkingTable->StartLBA).U64C).U64C_H_LL),pIdeChannel->IdePortP1XX + 0x0004);
		outb((((pWorkingTable->StartLBA).U64C).U64C_L_LH),pIdeChannel->IdePortP1XX + 0x0004);
		outb((((pWorkingTable->StartLBA).U64C).U64C_H_LH),pIdeChannel->IdePortP1XX + 0x0005);
		outb((((pWorkingTable->StartLBA).U64C).U64C_L_HL),pIdeChannel->IdePortP1XX + 0x0005);
		outb((pDeviceInfo->HD_V1X6),pIdeChannel->IdePortP1XX + 0x0006);
		break;
	// execute 28 bits command
	case IDE_READ_DMA:		// READ DMA
	case IDE_WRITE_DMA:		// WRITE DMA
	case IDE_VERIFY:			// VERIFY 
	case IDE_SEEK:			// SEEK
		outb((((pWorkingTable->TransferLen).U64C).U64C_L_LL),pIdeChannel->IdePortP1XX + 0X0002);
		outb((((pWorkingTable->StartLBA).U64C).U64C_L_LL),pIdeChannel->IdePortP1XX + 0x0003);
		outb((((pWorkingTable->StartLBA).U64C).U64C_L_LH),pIdeChannel->IdePortP1XX + 0x0004);
		outb((((pWorkingTable->StartLBA).U64C).U64C_L_HL),pIdeChannel->IdePortP1XX + 0x0005);
		V1X6 = (pDeviceInfo->HD_V1X6) | ((((pWorkingTable->StartLBA).U64C).U64C_L_HH) & 0x0F);
		outb(V1X6,pIdeChannel->IdePortP1XX + 0x0006);
		break;
	default:
		outb(pDeviceInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
		break;
	}
	switch(pWorkingTable->XferCmd) {
	case IDE_READ_DMA:	// READ DMA
	case IDE_READ_DMA_EXT:	// READ DMA EXT
		outb(0x08,pIdeChannel->IdePortPSFF);					// stop DMA
		outl(pWorkingTable->LmailBox,pIdeChannel->IdeRegPrdTable);		// set PRD table
		outb(pWorkingTable->XferCmd,pIdeChannel->IdePortP1XX + 0x0007);	// send IDE command
		outb(0x09,pIdeChannel->IdePortPSFF);					// start DMA
		break;
	case IDE_WRITE_DMA:	// WRITE DMA
	case IDE_WRITE_DMA_EXT:	// WRITE DMA EXT
		outb(0x00,pIdeChannel->IdePortPSFF);					// stop DMA
		outl(pWorkingTable->LmailBox,pIdeChannel->IdeRegPrdTable);		// set PRD table
		outb(pWorkingTable->XferCmd,pIdeChannel->IdePortP1XX + 0x0007);	// send IDE command
		outb(0x01,pIdeChannel->IdePortPSFF);					// start DMA
		break;
	default:
		outb(pWorkingTable->XferCmd,pIdeChannel->IdePortP1XX + 0X0007);	// send IDE command
		break;
	}
	return;
}
//========================================================================
static void HardDiskInterrupt(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pIdeChannel)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	struct scatterlist *sg_list;
	P_WorkingTable pWorkingTable;
	Scsi_Cmnd *pScsiCmd;
	unsigned char status;

	pWorkingTable = pIdeChannel->pCurRequest;
	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
	status = inb(pIdeChannel->IdePortP1XX + 0x0007);
	if (status & 0x01) {
		// IO error
		status = inb(pIdeChannel->IdePortP1XX + 0x0001);
		ConvertSenseKey(pIdeChannel,pWorkingTable,status);
		pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
	} else { // Command Complete
		pScsiCmd->result = SAM_STAT_GOOD;
	}
	if (pWorkingTable->use_sg) {
		sg_list = (struct scatterlist *)pWorkingTable->request_buffer;

	        pci_unmap_sg(pHwDeviceExtension->pPciDev,
				sg_list,
				pWorkingTable->use_sg,
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
				scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
# else
				pScsiCmd->sc_data_direction
# endif
			   );

	} else {
		pci_unmap_single(pHwDeviceExtension->pPciDev,
				pWorkingTable->pDmaSingleMapAddr,
				pWorkingTable->request_bufflen,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
				scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
#else
				pScsiCmd->sc_data_direction
#endif
			       );
	}
	return;
}
//========================================================================
static void ScanATAPI(PHW_DEVICE_EXTENSION DeviceExtension,P_DeviceInfo pDeviceInfo)
{
	PUIdentify pIdentifyBuff;
	P_Channel pIdeChannel;
	int j;
	unsigned char status,TempBuf[512];

	pIdeChannel = pDeviceInfo->pChannel;
	outb(pDeviceInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	udelay(100);
	status = inb(pIdeChannel->IdePortP1XX + 0x0007);
	if (  (status == 0x7F) || (status & 0x80)   ) {
		return;
	}
	pIdentifyBuff = (PUIdentify)TempBuf;
	//=============================================================================================
	// RESET IDE DEVICE 
	//=============================================================================================
	outb(pDeviceInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	outb(0x08,pIdeChannel->IdePortP1XX + 0x0007);
	msleep(250);
	if (SendIdentifyCmd2IDE(pDeviceInfo,pIdentifyBuff,0xA1)) {
		// Got ATAPI device
		if ((pIdentifyBuff->ConfigBitInfo & 0x8000) == 0x8000) {
			// ATAPI device confirm
			pDeviceInfo->FG_DeviceInstall = 1;
			pDeviceInfo->FG_AtapiDevice   = 1;
			//=============================================================================================
			// SET DEVICE'S PARAMETER
			//=============================================================================================
#if __LITTLE_ENDIAN
			for (j = 0 ; j < 40 ; j += 2) {
				pDeviceInfo->ModelName[j]   = pIdentifyBuff->ModelName[j+1];
				pDeviceInfo->ModelName[j+1] = pIdentifyBuff->ModelName[j];
			}
			for (j = 0 ; j < 20 ; j += 2) {
				pDeviceInfo->SerialNo[j]   = pIdentifyBuff->SerialNo[j+1];
				pDeviceInfo->SerialNo[j+1] = pIdentifyBuff->SerialNo[j];
			}
			for (j = 0 ; j < 8 ; j += 2) {
				pDeviceInfo->FirmWareRev[j]   = pIdentifyBuff->FirmWareRev[j+1];
				pDeviceInfo->FirmWareRev[j+1] = pIdentifyBuff->FirmWareRev[j];
			}
#else			
			for (j = 0 ; j < 20 ; j++) {
				pDeviceInfo->SerialNo[j]	= pIdentifyBuff->SerialNo[j];
			}
			for (j = 0 ; j < 40 ; j++) {
				pDeviceInfo->ModelName[j]	= pIdentifyBuff->ModelName[j];
			}
			for (j = 0 ; j < 8 ; j++) {
				pDeviceInfo->FirmWareRev[j]	= pIdentifyBuff->FirmWareRev[j];
			}
#endif
			switch (pIdentifyBuff->ConfigBitInfo & 0x0060) {
			case 0x0000:
				pDeviceInfo->A0Wait = 3000+500;
				break;
			case 0x0020:
				pDeviceInfo->A0Wait = 0;
				break;
			case 0x0040:
				pDeviceInfo->A0Wait = 50+500;
				break;
			default:
				pDeviceInfo->A0Wait = 3000+500;
			}
			if (pDeviceInfo->A0Wait == 0) {
				pDeviceInfo->FG_DeviceInstall = 0;
				return;
			}
			if ((pIdentifyBuff->ConfigBitInfo & 0x1f00)==0) {
				// Direct access device
				pDeviceInfo->DeviceBlockSize = 512;
			} else if ((pIdentifyBuff->ConfigBitInfo & 0x1f00)==0x0500) {
				// CD-ROM device
				pDeviceInfo->FG_DiscDevice = 1;
				pDeviceInfo->DeviceBlockSize = 2048;
			} else {
				pDeviceInfo->FG_DeviceInstall = 0;
				pDeviceInfo->DeviceBlockSize = 0;
			}
			pDeviceInfo->FG_Support48Bits = 0;
			pDeviceInfo->FG_IdeExcept = 0;
			((pDeviceInfo->Capacity).U64L).U64L_L = 0;
			((pDeviceInfo->Capacity).U64L).U64L_H = 0;
			if (pDeviceInfo->FG_DeviceInstall) {
				// set device's IO mode
				SetPIOfeature(pDeviceInfo,pIdentifyBuff,_MAX_PIO_MODE);
				if (pIdentifyBuff->FG_ValidMap & 0x0004) {
					SetUltrafeature(pDeviceInfo,pIdentifyBuff,_ULTRA_DMA33);
				}
				if (	  (pIdentifyBuff->Capability & 0x0100) 
					&&(pIdentifyBuff->MultiWordDMASupport&0x07)
					&&(pDeviceInfo->DMA_Mode==0)
				   ) {
					// set multiword DMA 
					SetDMAfeature(pDeviceInfo,pIdentifyBuff,0x0F);
				}
				pDeviceInfo->FG_IdeExcept		=	0;
			} // end of pHardDisk->HddFlag !=0
			return;
		} // end of ATAPI device confirm
	} // end of send A1 command
	pDeviceInfo->FG_DeviceInstall = 0;
	return;
}
//========================================================================
static void ExecuteIdeATAPI(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pIdeChannel)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_DeviceInfo pCdRomInfo;
	P_WorkingTable pWorkingTable;
	Scsi_Cmnd *pScsiCmd;
	
	pWorkingTable = pIdeChannel->pCurRequest;
	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	pCdRomInfo = pWorkingTable->pDevice;
	pIdeChannel = pCdRomInfo->pChannel;
	ConvertCmdATAPI(pWorkingTable);
	if (	(pScsiCmd->cmnd[0] == READ_6) ||
		(pScsiCmd->cmnd[0] == READ_10) ||
		(pScsiCmd->cmnd[0] == WRITE_6) ||
		(pScsiCmd->cmnd[0] == WRITE_10) ||
		(pScsiCmd->cmnd[0] == ATAPI_READ_CD) ||
		(pScsiCmd->cmnd[0] == ATAPI_WRITE_CONTINUE)
	   ) {
		if (pCdRomInfo->DMA_Mode) {
			// DO DMA I/O
			BuildPrdTable(pHwDeviceExtension,pWorkingTable,pWorkingTable->pMailBox);
			DoDmaATAPIXfer(pWorkingTable);
			if (pScsiCmd->result == SAM_STAT_CHECK_CONDITION) {
				RestoreCmdATAPI(pWorkingTable);
			}
		} else {
			// DO PIO I/O
			DoPioATAPIXfer(pWorkingTable);
			if (pScsiCmd->result == SAM_STAT_CHECK_CONDITION) {
				RestoreCmdATAPI(pWorkingTable);
			}
		}
	} else {
		DoPioATAPIXfer(pWorkingTable);
		if (pScsiCmd->result == SAM_STAT_CHECK_CONDITION) {
			RestoreCmdATAPI(pWorkingTable);
		}
	}
	return;
}
//===================================================================================================
static void RestoreCmdATAPI(P_WorkingTable pWorkingTable)
{
	P_DeviceInfo pCdRomInfo;
	Scsi_Cmnd *pScsiCmd;
	int i;
	
	pCdRomInfo	= pWorkingTable->pDevice;
	pScsiCmd		= pWorkingTable->pWorkingScsiCmd;
	switch (pScsiCmd->cmnd[0]) {
	case ATAPI_READ_CD:
		if (	  (pWorkingTable->OrgCmnd[0] == READ_10)
			||(pWorkingTable->OrgCmnd[0] == 0xD8)
		   ) {
			// restore cdb
			for (i = 0 ; i < 12 ; i++) {
				pScsiCmd->cmnd[i] = pWorkingTable->OrgCmnd[i];
			}
		}
		break;
/*
	case 0x46:
		for (i = 0 ; i < 12 ; i++) {
			pScsiCmd->cmnd[i] = pWorkingTable->OrgCmnd[i];
		}
		// restore SRB's DataBuffer
		pWorkingTable->request_buffer  = pWorkingTable->pOrgRequesrBuff;
		pWorkingTable->request_bufflen = pWorkingTable->pOrgRequestBuffLen;
		pWorkingTable->use_sg      = pWorkingTable->old_use_sg;
		break;
*/
	case MODE_SENSE_10:
		if (pWorkingTable->OrgCmnd[0] == MODE_SENSE) {
			// restore cdb
			for (i = 0 ; i < 12 ; i++) {
				pScsiCmd->cmnd[i] = pWorkingTable->OrgCmnd[i];
			}
			// restore SRB's DataBuffer
			pWorkingTable->request_buffer  = pWorkingTable->pOrgRequesrBuff;
			pWorkingTable->request_bufflen = pWorkingTable->pOrgRequestBuffLen;
			pWorkingTable->use_sg 	 =pWorkingTable->old_use_sg;
//			pScsiCmd->use_sg          = pWorkingTable->use_sg;
		} else {
			// reset SRB's DataBuffer
			pWorkingTable->request_buffer  = pWorkingTable->pOrgRequesrBuff;
			pWorkingTable->request_bufflen = pWorkingTable->pOrgRequestBuffLen;
			pWorkingTable->use_sg      = pWorkingTable->old_use_sg;
		}
		pWorkingTable->OrgCmnd[0]=0;
		break;
	case MODE_SELECT_10:
		for (i = 0 ; i < 12 ; i++) {
			pScsiCmd->cmnd[i] = pWorkingTable->OrgCmnd[i];
		}
		// reset SRB's DataBuffer
		pWorkingTable->request_buffer  = pWorkingTable->pOrgRequesrBuff;
		pWorkingTable->request_bufflen = pWorkingTable->pOrgRequestBuffLen;
		pWorkingTable->use_sg      = pWorkingTable->old_use_sg;
		pWorkingTable->OrgCmnd[0] = 0;
		break;
	case 0xB9:
	case 0xBA:
		for (i = 0 ; i < 12 ; i++) {
			pScsiCmd->cmnd[i] = pWorkingTable->OrgCmnd[i];
		}
		break;
	}
	return;
}
//===================================================================================================
static void DoPioATAPIXfer(P_WorkingTable pWorkingTable)
{
	P_DeviceInfo pCdRomInfo = pWorkingTable->pDevice;
	P_Channel pIdeChannel = pCdRomInfo->pChannel;
	Scsi_Cmnd *pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	int i,j;
	unsigned short IdeCommand;
	unsigned char status;

	// make sure the IDE bus is free
	outb(pCdRomInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	udelay(200);
	status = inb(pIdeChannel->IdePortP1XX + 0x0007);
	if (  (status == 0x7F) || (status & 0x80)   ) {
		return;
	}
	outb(0,pIdeChannel->IdePortP1XX + 0x0001);		// PIO transfer
	outb(0xFF,pIdeChannel->IdePortP1XX + 0x0004);	// no byte count limit
	outb(0xFE,pIdeChannel->IdePortP1XX + 0x0005);
	outb(pCdRomInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	outb(0xA0,pIdeChannel->IdePortP1XX + 0x0007);
	for (i = 0 ; i < 250000 ; j++) {
		// wait drq income with 2.5 S
		status = inb( pIdeChannel->IdePortP1XX + 0x0007);
		if ((status & 0x80) == 0) {
			if (status & 0x08) {
				break;
			}
		}
		udelay(10);
		cpu_relax();
	}
	if ((status & 0x88) == 0x08) {
		do {
			status = inb(pIdeChannel->IdePortP1XX + 0x0002);
		} while ((status & 0x03) != 0x01);
		// send CDB
		for (i = 0 ; i < 12 ; i+=2) {
			IdeCommand = ((((unsigned short)(pScsiCmd->cmnd[i+1])) << 8) & 0xFF00)+
				     ((((unsigned short)(pScsiCmd->cmnd[i]  ))     ) & 0x00FF);
			outw(IdeCommand,pIdeChannel->IdePortP1XX);
		}
		pWorkingTable->FG_DoIoByDMA = 0;
		return;
	}
	// DRQ NOT READY
	pCdRomInfo->FG_InternalError = 1;
	pCdRomInfo->SenseKey = NOT_READY;
	pCdRomInfo->SenseAsc = 0x00;
	pCdRomInfo->SenseAscQ = 0x00;
	pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
	return;
}
//===================================================================================================
static void DoDmaATAPIXfer(P_WorkingTable pWorkingTable)
{
	P_DeviceInfo pCdRomInfo = pWorkingTable->pDevice;
	P_Channel pIdeChannel = pCdRomInfo->pChannel;
	Scsi_Cmnd *pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	int i,j;
	unsigned short IdeCommand;
	unsigned char status,dma_cmd;

	// make sure the IDE bus is free
	outb(pCdRomInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	udelay(100);
	status = inb(pIdeChannel->IdePortP1XX + 0x0007);
	if (  (status == 0x7F) || (status & 0x80)   ) {
		return;
	}
	outb(0x01,pIdeChannel->IdePortP1XX + 0x0001);	// DMA transfer
	outb(0xFF,pIdeChannel->IdePortP1XX + 0x0004);// no byte count limit
	outb(0xFF,pIdeChannel->IdePortP1XX + 0x0005);
	outb(pCdRomInfo->HD_V1X6,pIdeChannel->IdePortP1XX + 0x0006);
	outb(0xA0,pIdeChannel->IdePortP1XX + 0x0007);
	for (i = 0 ; i < 250000 ; j++) {
		// wait drq income with 2.5 S
		status = inb( pIdeChannel->IdePortP1XX + 0x0007);
		if ((status & 0x80) == 0) {
			if (status & 0x08) {
				break;
			}
		}
		udelay(10);
		cpu_relax();
	}
	if ((status & 0x88) == 0x08) {
		do {
			status = inb(pIdeChannel->IdePortP1XX + 0x0002);
		}
		while ((status & 0x03) != 0x01);
		switch(pScsiCmd->cmnd[0]) {
		case READ_6:
		case READ_10:
		case ATAPI_READ_CD:
			dma_cmd = 0x09;	// DMA read
			break;
		default:
			dma_cmd = 0x01;	// DMA write
			break;
		}
		// STOP DMA
		outb((dma_cmd&0x08),(pIdeChannel->IdePortPSFF));
		// send CDB
		for (i = 0 ; i < 12 ; i+=2) {
			IdeCommand = ((((unsigned short)(pScsiCmd->cmnd[i+1])) << 8) & 0xFF00)+
				     ((((unsigned short)(pScsiCmd->cmnd[i]  ))     ) & 0x00FF);
			outw(IdeCommand,pIdeChannel->IdePortP1XX);
		}
		outl(pWorkingTable->LmailBox,pIdeChannel->IdeRegPrdTable);	// set PRD table
		// START DMA
		outb(dma_cmd,(pIdeChannel->IdePortPSFF));
		pWorkingTable->FG_DoIoByDMA = 1;
		return;
	}
	// DRQ NOT READY
	pCdRomInfo->FG_InternalError = 1;
	pCdRomInfo->SenseKey = NOT_READY;
	pCdRomInfo->SenseAsc = 0x00;
	pCdRomInfo->SenseAscQ = 0x00;
	pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
	return;
}
//========================================================================
static void CdRomInterrupt(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pIdeChannel)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_WorkingTable pWorkingTable = pIdeChannel->pCurRequest;
	P_DeviceInfo pCdRomInfo = pWorkingTable->pDevice;
	Scsi_Cmnd *pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	struct scatterlist *sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
	PUINQUIRYDATA pInquirybuf;
	unsigned long km_flags;
	unsigned char *pBuf,*km_buffer;
	unsigned char *SrbDataBuffer,*SrbExtOriginalBuffer;
	int i,km_sg_used = 0;
	unsigned short word,TransferBlock,sg_len,BlockLen,LastWord;
	unsigned char port_status;
#if __LITTLE_ENDIAN
#else
	u16 temp16;
#endif

	km_buffer = NULL;
	km_flags = 0;
	port_status = inb(pIdeChannel->IdePortP3XX) & 0xE9;
	if (port_status & 0x01) {
		// error bit assert
		printk("ide command %x end chk status %x\n",pScsiCmd->cmnd[0],port_status);
		pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
	} else if (port_status & 0x08) {
		// drq assert
		port_status   = inb(pIdeChannel->IdePortP1XX + 0x0002);
		TransferBlock =  ((((unsigned short)(inb(pIdeChannel->IdePortP1XX + 0x0005)))<<8) & 0xFF00)+
				((((unsigned short)(inb(pIdeChannel->IdePortP1XX + 0x0004)))   ) & 0x00FF);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
		if (	  ((pScsiCmd->sc_data_direction & SCSI_DATA_NONE) == SCSI_DATA_READ)
			||(	  ((pScsiCmd->sc_data_direction & SCSI_DATA_NONE) == SCSI_DATA_UNKNOWN)
				&&((port_status & 0x02) == 0x02)
		   	 )
		   ) {
#else
		if ((port_status & 0x02) == 0x02) {
#endif
			// read data form device
			if (pWorkingTable->use_sg) {
				sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
				sg_list += pWorkingTable->sg_offset;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
				pBuf = sg_list->address + pWorkingTable->sg_data_offset;
#else
				local_irq_save(km_flags);
				km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//				km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
				pBuf = km_buffer;
				pBuf += pWorkingTable->sg_data_offset;
#endif
				sg_len  = sg_list->length - pWorkingTable->sg_data_offset;
				if (TransferBlock > pWorkingTable->DataCounter) {
					word = pWorkingTable->DataCounter;
				} else {
					word = TransferBlock;
				}
				TransferBlock -= word;
				while(word) {
					if (sg_len >= word) {
						for (i = 0 ; i < (word/2) ; i++) {
#if __LITTLE_ENDIAN
							*((unsigned short *)pBuf) = inw(pIdeChannel->IdePortP1XX);
#else
							temp16 = inw(pIdeChannel->IdePortP1XX);
							*pBuf = (unsigned char)temp16;
							*(pBuf+1) = (unsigned char)(temp16 >> 8);
#endif							
							pBuf += 2;
						}
					//	printk("word %d cnt %x TransferBlock %x datacnt %x\n",word,pWorkingTable->sg_offset,TransferBlock,pWorkingTable->DataCounter);
						pWorkingTable->sg_data_offset += word;
						word = 0;
					} else {
						word -= sg_len;
						for (i = 0 ; i < (sg_len/2) ; i++) {
#if __LITTLE_ENDIAN
							*((unsigned short *)pBuf) = inw(pIdeChannel->IdePortP1XX);
#else
							temp16 = inw(pIdeChannel->IdePortP1XX);
							*pBuf = (unsigned char)temp16;
							*(pBuf+1) = (unsigned char)(temp16 >> 8);
#endif							
							pBuf += 2;
						}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
						sg_list++;
						pBuf = sg_list->address;
#else
						kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
#ifdef KERNEL_V2625
						sg_list = sg_next(sg_list);
#else
						sg_list++;
#endif
						km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//						km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
						pBuf = km_buffer;
#endif
						sg_len = sg_list->length;
						pWorkingTable->sg_offset++;
						pWorkingTable->sg_data_offset = 0;
					}
				}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
#else
				kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
				local_irq_restore(km_flags);
#endif
			//	printk("TransferBlock %x\n",TransferBlock);
				for (i = 0 ; i < (TransferBlock/2) ; i++) {
					inw(pIdeChannel->IdePortP1XX);
			//		printk("temp %x\n",temp);
				}
				/*
				LastWord = TransferBlock%2;
				if(LastWord){
					inw(pIdeChannel->IdePortP1XX);
				}
*/
			} else {
				if (TransferBlock > pWorkingTable->DataCounter) {
					if (pWorkingTable->DataCounter) {
						for (i = 0 ; i < ((pWorkingTable->DataCounter)/2) ; i++) {
#if __LITTLE_ENDIAN
							*((unsigned short *)(pWorkingTable->pDataBufPtr)) = inw(pIdeChannel->IdePortP1XX);
#else
							temp16 = inw(pIdeChannel->IdePortP1XX);
							*(pWorkingTable->pDataBufPtr) = (unsigned char)(temp16);
							*((pWorkingTable->pDataBufPtr)+1) = (unsigned char)(temp16 >> 8);
#endif						
							pWorkingTable->pDataBufPtr += 2;
						}
						TransferBlock -= (unsigned short)(pWorkingTable->DataCounter);
						pWorkingTable->DataCounter = 0;
						
						for (i = 0 ; i < (TransferBlock/2) ; i++) {
							inw(pIdeChannel->IdePortP1XX);
						}
						LastWord = TransferBlock%2;
						if(LastWord){
							inw(pIdeChannel->IdePortP1XX);
						}
					}
				} else {
					for (i = 0 ; i < (TransferBlock/2) ; i++) {
#if __LITTLE_ENDIAN
						*((unsigned short *)(pWorkingTable->pDataBufPtr)) = inw(pIdeChannel->IdePortP1XX);
#else
						temp16 = inw(pIdeChannel->IdePortP1XX);
						*(pWorkingTable->pDataBufPtr) = (unsigned char)(temp16);
						*((pWorkingTable->pDataBufPtr)+1) = (unsigned char)(temp16 >> 8);
#endif						
						pWorkingTable->pDataBufPtr += 2;
					}
					pWorkingTable->DataCounter -= ((unsigned long)TransferBlock);
					
				}
			}
		} else {
			// write data to device
			if (pWorkingTable->use_sg) {
				sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
				sg_list += pWorkingTable->sg_offset;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
				pBuf  = sg_list->address + pWorkingTable->sg_data_offset;
#else
				local_irq_save(km_flags);
				km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//				km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
				pBuf = km_buffer;
				pBuf += pWorkingTable->sg_data_offset;
#endif
				sg_len = sg_list->length - pWorkingTable->sg_data_offset;
				if (TransferBlock > pWorkingTable->DataCounter) {
					word = pWorkingTable->DataCounter;
				} else {
					word = TransferBlock;
				}
				TransferBlock -= word;
				while(word) {
					if (sg_len >= word) {
						for (i = 0 ; i < (word/2) ; i++) {
#if __LITTLE_ENDIAN
							outw(*((unsigned short *)pBuf),pIdeChannel->IdePortP1XX);
#else						
							temp16 = ((((u16)(*pBuf)) << 8) & 0xFF00) + (((u16)(*(pBuf+1))) & 0x00FF);
							outw(temp16,pIdeChannel->IdePortP1XX);
#endif
							pBuf += 2;
						}
						pWorkingTable->sg_data_offset += word;
						word = 0;
					} else {
						word -= sg_len;
						for (i = 0 ; i < (sg_len/2) ; i++) {
#if __LITTLE_ENDIAN
							outw(*((unsigned short *)pBuf),pIdeChannel->IdePortP1XX);
#else						
							temp16 = ((((u16)(*pBuf)) << 8) & 0xFF00) + (((u16)(*(pBuf+1))) & 0x00FF);
							outw(temp16,pIdeChannel->IdePortP1XX);
#endif
							pBuf += 2;
						}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
						sg_list++;
						pBuf = sg_list->address;
#else
						kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
#ifdef KERNEL_V2625
						sg_list = sg_next(sg_list);
#else
						sg_list++;
#endif
						km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//						km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
						pBuf = km_buffer;
#endif
						sg_len  = sg_list->length;
						pWorkingTable->sg_offset++;
						pWorkingTable->sg_data_offset = 0;
					}
				}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
#else
				kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
				local_irq_restore(km_flags);
#endif
				for (i = 0 ; i < (TransferBlock/2) ; i++) {
					outw(0,pIdeChannel->IdePortP1XX);
				}
/*
				LastWord = TransferBlock%2;
				if(LastWord){
					outw(0,pIdeChannel->IdePortP1XX);
				}
*/
			} else {
				if (TransferBlock > pWorkingTable->DataCounter) {
					if (pWorkingTable->DataCounter) {
						for (i = 0 ; i < ((pWorkingTable->DataCounter)/2) ; i++) {
#if __LITTLE_ENDIAN
							outw(*((unsigned short *)(pWorkingTable->pDataBufPtr)),pIdeChannel->IdePortP1XX);
#else						
							temp16 = ((((u16)(*pWorkingTable->pDataBufPtr)) << 8) & 0xFF00) + 
								  (((u16)(*((pWorkingTable->pDataBufPtr)+1))) & 0x00FF);
							outw(temp16,pIdeChannel->IdePortP1XX);
#endif
							pWorkingTable->pDataBufPtr += 2;
						}
						TransferBlock -= (unsigned short)(pWorkingTable->DataCounter);
						pWorkingTable->DataCounter = 0;
					}
					for (i = 0 ; i <= TransferBlock ; i++) {
						outw(0,pIdeChannel->IdePortP1XX);
					}
/*
					LastWord = TransferBlock%2;
					if(LastWord){
						outw(0,pIdeChannel->IdePortP1XX);
					}
*/
				} else {
					for (i = 0 ; i < (TransferBlock/2) ; i++) {
#if __LITTLE_ENDIAN
						outw(*((unsigned short *)(pWorkingTable->pDataBufPtr)),pIdeChannel->IdePortP1XX);
#else						
						temp16 = ((((u16)(*pWorkingTable->pDataBufPtr)) << 8) & 0xFF00) + 
							  (((u16)(*((pWorkingTable->pDataBufPtr)+1))) & 0x00FF);
						outw(temp16,pIdeChannel->IdePortP1XX);
#endif
						pWorkingTable->pDataBufPtr += 2;
					}
					pWorkingTable->DataCounter -= ((unsigned long)TransferBlock);
				}
			}
		}
	} else {
	//	printk("ide command %x end\n",pScsiCmd->cmnd[0]);
		// complete
		switch (pScsiCmd->cmnd[0]) {
		case REQUEST_SENSE:
			sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
			if (pWorkingTable->use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
				pBuf  = sg_list->address;
#else
				local_irq_save(km_flags);
				km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//				km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
				pBuf = km_buffer;
#endif
			} else {
				pBuf = pWorkingTable->request_buffer;
			}
			if ((pBuf[2] == 0x06) && (pBuf[12]==0x28)) {
				pCdRomInfo->DeviceBlockSize = 2048;
			} else if ((pBuf[2] == 0x02) && (pBuf[12]==0x3a)) {
					pCdRomInfo->DeviceBlockSize = 2048;
			}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
#else
			if (pWorkingTable->use_sg) {
				kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
				local_irq_restore(km_flags);
			}
#endif
			break;
		case INQUIRY:
			sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
			if (pWorkingTable->use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
				pBuf = sg_list->address;
#else
				local_irq_save(km_flags);
				km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//				km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
				pBuf = km_buffer;
#endif
			} else {
				pBuf = pWorkingTable->request_buffer;
			}
			pInquirybuf = (PUINQUIRYDATA)pBuf;
			pInquirybuf->VersionInfo &= 0xF8;
			pInquirybuf->VersionInfo |= 0x02;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
#else
			if (pWorkingTable->use_sg) {
				kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
				local_irq_restore(km_flags);
			}
#endif
			break;
/*
		case 0x46:
			SrbDataBuffer = pWorkingTable->request_buffer;
			sg_list = (struct scatterlist *)(pWorkingTable->pOrgRequesrBuff);
			if (pWorkingTable->old_use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
				SrbExtOriginalBuffer  = sg_list->address;
#else
				local_irq_save(km_flags);
				km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//				km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
				SrbExtOriginalBuffer = km_buffer;
#endif
			} else {
				SrbExtOriginalBuffer = pWorkingTable->pOrgRequesrBuff;
			}
			for (i = 0 ; i < pWorkingTable->request_bufflen ; i++) {
				*SrbExtOriginalBuffer = *SrbDataBuffer;
				SrbExtOriginalBuffer++;
				SrbDataBuffer++;
			}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
#else
			if (pWorkingTable->old_use_sg) {
				kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
				local_irq_restore(km_flags);
			}
#endif
			break;
*/
		case MODE_SENSE_10: //xx7
			SrbDataBuffer = pWorkingTable->request_buffer;
			sg_list = (struct scatterlist *)(pWorkingTable->pOrgRequesrBuff);
			km_sg_used = 0;
			if (pWorkingTable->old_use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
				SrbExtOriginalBuffer = sg_list->address;
#else
				local_irq_save(km_flags);
				km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//				km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
				SrbExtOriginalBuffer = km_buffer;
				km_sg_used = 1;
#endif
			} else {
				SrbExtOriginalBuffer = pWorkingTable->pOrgRequesrBuff;
			}
			BlockLen = pCdRomInfo->DeviceBlockSize;
			word = ((((unsigned short)SrbDataBuffer[0]) << 8) & 0xff00)+
			       ((((unsigned short)SrbDataBuffer[1])     ) & 0x00ff);
			if (pWorkingTable->OrgCmnd[0] == MODE_SENSE) {
				// rebuild mode parameter header
				SrbExtOriginalBuffer[0] = word - 3;		// data length	
				SrbExtOriginalBuffer[1] = SrbDataBuffer[2];	// medium type
				SrbExtOriginalBuffer[2] = 0;			// device-specific parameter
				SrbExtOriginalBuffer[3] = 0; 			// block descriptor length
				i=4;
				// rebuild block descriptor
				if ((pWorkingTable->OrgCmnd[1] & 0x08) == 0) {
					SrbExtOriginalBuffer[0] += 8;
					// set block descriptor length
					SrbExtOriginalBuffer[3] = 8;
					// build descriptor block
					//SrbExtOriginalBuffer[4] = 0x83;
					for (i = 4 ; i <= 8 ; i++) {
						SrbExtOriginalBuffer[i] = 0;
					}
					SrbExtOriginalBuffer[9]  = (unsigned char)(BlockLen>>16);
					SrbExtOriginalBuffer[10] = (unsigned char)(BlockLen>>8);
					SrbExtOriginalBuffer[11] = (unsigned char)BlockLen;
					if ((pWorkingTable->OrgCmnd[2] & 0xc0) == 0x40) {
						// PC = 0x01
						SrbExtOriginalBuffer[9]  = 0xFF;
						SrbExtOriginalBuffer[10] = 0xFF;
						SrbExtOriginalBuffer[11] = 0xFF;
					}
					i = 12;
				}
				SrbExtOriginalBuffer += (unsigned char)i;
				SrbDataBuffer += 8;
				// moving page list data
				for (i = 0 ; i < (pWorkingTable->pOrgRequestBuffLen - 12) ; i++) {
					*SrbExtOriginalBuffer = *SrbDataBuffer;
					SrbExtOriginalBuffer++;
					SrbDataBuffer++;
				}
			} else {
				// MODE_SENSE_10
				for (i = 0 ; i < word ; i++) {
						SrbExtOriginalBuffer[i] = SrbDataBuffer[i];
				}	
				
				if (	(((unsigned short)(pWorkingTable->request_bufflen)) > (word + 16)) &&
						 ((pScsiCmd->cmnd[1] & 0x08) == 0)
				   ) {
					// host buffer > data + block descriptor length + head len
					// and DBD bit = 0
					word = word + 8;			// add block descriptor length
					// rebuild parameter header
					SrbExtOriginalBuffer[0] = (unsigned char)(word >> 8);			// data length	
					SrbExtOriginalBuffer[1] = (unsigned char)word;	
					for (i = 2 ; i <= 5 ; i++) {
						SrbExtOriginalBuffer[i] = SrbDataBuffer[i];
					}
					SrbExtOriginalBuffer[6] = 0;
					SrbExtOriginalBuffer[7] = 8;
					word = word - 8;
					// copy sense data
					for (i = 0 ; i < word ; i++) {
						SrbExtOriginalBuffer[i+16] = SrbDataBuffer[i+8];
					}
					// rebuild block descriptor
					for (i = 8 ; i <= 12 ; i++) {
						SrbExtOriginalBuffer[i] = 0;
					}	
					SrbExtOriginalBuffer[13] = (unsigned char)(BlockLen>>16);
					SrbExtOriginalBuffer[14] = (unsigned char)(BlockLen>>8);
					SrbExtOriginalBuffer[15] = (unsigned char)BlockLen;
					if ((pWorkingTable->OrgCmnd[2] & 0xc0) == 0x40) {
						SrbExtOriginalBuffer[13] = 0xFF;
						SrbExtOriginalBuffer[14] = 0xFF;
						SrbExtOriginalBuffer[15] = 0xFF;
					}
					pWorkingTable->OrgCmnd[0]=0;
				} else {
					word +=2; // total transfer length
					if (word > pWorkingTable->request_bufflen) {
						word = (unsigned short)(pWorkingTable->request_bufflen);
					}
					for (i = 0 ; i < word ; i++) {
						SrbExtOriginalBuffer[i] = SrbDataBuffer[i];
					}	
				}
				for (i = 0 ; i < 12 ; i++) {
					pScsiCmd->cmnd[i] = pWorkingTable->OrgCmnd[i];
				}

			}
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
#else
			if (km_sg_used) {
				kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
				local_irq_restore(km_flags);
			}
#endif
			break;
		}
		pScsiCmd->result = SAM_STAT_GOOD;
	}
	if (pScsiCmd->result != STATUS_PENDING) {
		
		if (pWorkingTable->FG_DoIoByDMA) {
			if (pWorkingTable->use_sg) {
				sg_list = (struct scatterlist *)pWorkingTable->request_buffer;

			        pci_unmap_sg(pHwDeviceExtension->pPciDev,
						sg_list,
						pWorkingTable->use_sg,
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
						scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
# else
						pScsiCmd->sc_data_direction
# endif
					   );

			} else {
				pci_unmap_single(pHwDeviceExtension->pPciDev,
						pWorkingTable->pDmaSingleMapAddr,
						pWorkingTable->request_bufflen,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
						scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
#else
						pScsiCmd->sc_data_direction
#endif
					       );
			}
		}
		RestoreCmdATAPI(pWorkingTable);
	}
	return;
}
//=======================================================================================
static int SoftwareReset(P_Channel pSataChannel,unsigned char PMx)
{
	P_CommandHead pCommandHead;
	P_CommandTable pCommandTable;
	u32 ci_map;

	if ((inb(pSataChannel->BaseIoPort + 0x0028) & 0x0F) != 0x03) {
		// physical not ready
		return 0;
	}
	//===============================================================================
	pCommandHead  = pSataChannel->pCommandPtr->pCommandHead + MAX_COMMAND_QUEUE_CH;
	pCommandTable = pSataChannel->pCommandPtr->pCommandTable + MAX_COMMAND_QUEUE_CH;
	ci_map = (0x00000001 << MAX_COMMAND_QUEUE_CH);
	//===============================================================================
	pCommandHead->PortMultiplier = (PMx & 0x0F);
	pCommandHead->CommandFISLen = 0x05;
	pCommandHead->ClearBusy = 1;
	pCommandTable->CommandFIS.FIS_Type = 0x27;
	pCommandTable->CommandFIS.PortMultiplier = (PMx & 0x0F);	// Port Multiplier
	pCommandTable->CommandFIS.FIS_Flag = 0;
	pCommandTable->CommandFIS.P3x6 = 0x04;			// RST bit = 1
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	while (inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) {
		udelay(100);
	}
	//===============================================================================
	udelay(10);
	//===============================================================================
	pCommandHead->ClearBusy = 0;
	pCommandTable->CommandFIS.P3x6 = 0;	// RST bit = 0
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	while (inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) {
		udelay(100);
	}
	//===============================================================================
	udelay(100);
	outb(inb(pSataChannel->BaseIoPort + 0x0010),pSataChannel->BaseIoPort + 0x0010);	// clear error
	return 1;
}
//=======================================================================================
static int PMxReset(P_Channel pSataChannel,unsigned char PMx)
{
	P_CommandHead pCommandHead;
	P_CommandTable pCommandTable;
	int i;
	unsigned int signature,ci_map;
	unsigned char status,status74;

	//===============================================================================
	// CLOverride force BSY,DRQ clear to 0
	//===============================================================================
	status = inb(pSataChannel->BaseIoPort + 0x0018) | 0x08;
	outb(status,pSataChannel->BaseIoPort + 0x0018);
	while((inb(pSataChannel->BaseIoPort + 0x0018) & 0x08) != 0) {
		udelay(100); // delay 1ms
		cpu_relax();
	}
	//===============================================================================
	pCommandHead  = pSataChannel->pCommandPtr->pCommandHead  + MAX_COMMAND_QUEUE_CH;
	pCommandTable = pSataChannel->pCommandPtr->pCommandTable + MAX_COMMAND_QUEUE_CH;
	ci_map = (0x00000001 << MAX_COMMAND_QUEUE_CH);
	//===============================================================================
	pCommandHead->PortMultiplier = 0x0F;
	pCommandHead->CommandFISLen = 0x05;
	pCommandTable->CommandFIS.FIS_Type = 0x27;
	pCommandTable->CommandFIS.PortMultiplier = 0x0F;		// Port Multiplier
	pCommandTable->CommandFIS.FIS_Flag = 1;
	pCommandTable->CommandFIS.P1x1 = 0x02;
	pCommandTable->CommandFIS.P1x2 = 0x01; // DET = 1
	pCommandTable->CommandFIS.P1x3 = 0;
	pCommandTable->CommandFIS.P1x4 = 0;
	pCommandTable->CommandFIS.P1x5 = 0;
	pCommandTable->CommandFIS.P1x6 = PMx;
	pCommandTable->CommandFIS.Command = 0xE8;		// Write Port Multiplier
	//=========================================================================================
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	while((inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) != 0) {
		// wait for CI bit clear
		udelay(100);
		cpu_relax();
	}
	outb(inb(pSataChannel->BaseIoPort + 0x0010),pSataChannel->BaseIoPort + 0x0010);	//clear interrupt
	//===============================================================================
	pCommandTable->CommandFIS.P1x2 = 0; // DET = 0
	//===============================================================================
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	while((inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) != 0) {
		// wait for CI bit clear
		udelay(100);
	}
	outb(inb(pSataChannel->BaseIoPort + 0x0010),pSataChannel->BaseIoPort + 0x0010);	//clear interrupt
	status74 =  inb(pSataChannel->BaseIoPort + 0x0074);		// backup PM bit
	outb((status74 | 0x04),pSataChannel->BaseIoPort + 0x0074);	// disable PM BIT bit 1,enable PASS D2H bit2
	for (i = 0 ; i < 10000 ; i++) {
		//===============================================================================
		pCommandTable->CommandFIS.P1x1 = 0;
		pCommandTable->CommandFIS.P1x2 = 0;
		pCommandTable->CommandFIS.P1x3 = 0;
		pCommandTable->CommandFIS.P1x4 = 0;
		pCommandTable->CommandFIS.P1x5 = 0;
		pCommandTable->CommandFIS.P1x6 = PMx;
		pCommandTable->CommandFIS.Command = 0xE4;
		//===============================================================================
		outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
		while((inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) != 0) {
			// wait for CI bit clear
			udelay(100);
		}
		outb(inb(pSataChannel->BaseIoPort + 0x0010),pSataChannel->BaseIoPort + 0x0010);	//clear interrupt
		//===============================================================================
		status = inb(pSataChannel->BaseIoPort + 0x0024) & 0x0F;
		if (status == 0x03) {
			// physical ready ===================================================
			pCommandTable->CommandFIS.P1x1 = 0x01;	// clear all error flag
			pCommandTable->CommandFIS.P1x2 = 0xFF;
			pCommandTable->CommandFIS.P1x3 = 0xFF;
			pCommandTable->CommandFIS.P1x4 = 0xFF;
			pCommandTable->CommandFIS.P1x5 = 0xFF;
			pCommandTable->CommandFIS.Command = 0xE8;
			//=======================================================================
			outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
			while((inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) != 0) {
				// wait for CI bit clear
				udelay(100);
			}
			while((inb(pSataChannel->BaseIoPort + 0x0010) & 0x01) == 0) {
				// wait D2H interrupt
				udelay(100);
			}
			for (i = 0 ; i < 10000 ; i++) {
				// wait signature
				signature = inl(pSataChannel->BaseIoPort + 0x0024);
				if ((signature == 0x00000101) || (signature == 0xEB140101)) {
					// HDD or ATAPI send D2H back
					break;
				}
				udelay(100);
			}
			//=======================================================================
			outb(inb(pSataChannel->BaseIoPort + 0x0010),pSataChannel->BaseIoPort + 0x0010);	//clear interrupt
			outb(status74,pSataChannel->BaseIoPort + 0x0074); // restore Reg74
			return 1;
		}
		udelay(100);
	}
	outb(status74,pSataChannel->BaseIoPort + 0x0074); // restore Reg74
	return 0;
}
//=======================================================================================
static void PortMultiplierEnumerate(PHW_DEVICE_EXTENSION DeviceExtension,P_Channel pSataChannel,P_DeviceInfo pDeviceInfo)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_CommandHead pCommandHead;
	P_CommandTable pCommandTable;
	int i;
	unsigned int ci_map,status;
	unsigned char PM_FanOut;

	//===============================================================================
	pCommandHead = pSataChannel->pCommandPtr->pCommandHead  + MAX_COMMAND_QUEUE_CH;
	pCommandTable = pSataChannel->pCommandPtr->pCommandTable + MAX_COMMAND_QUEUE_CH;
	ci_map = (0x00000001 << MAX_COMMAND_QUEUE_CH);
	//===============================================================================
	pCommandHead->PortMultiplier = 0x0F;
	pCommandHead->CommandFISLen = 0x05;
	pCommandTable->CommandFIS.FIS_Type = 0x27;
	pCommandTable->CommandFIS.PortMultiplier = 0x0F; // Port Multiplier
	pCommandTable->CommandFIS.FIS_Flag = 1;
	pCommandTable->CommandFIS.P1x1 = 0x02;
	pCommandTable->CommandFIS.P1x2 = 0;
	pCommandTable->CommandFIS.P1x3 = 0;
	pCommandTable->CommandFIS.P1x4 = 0;
	pCommandTable->CommandFIS.P1x5 = 0;
	pCommandTable->CommandFIS.P1x6 = 0x0F;
	pCommandTable->CommandFIS.Command = 0xE4; // Read Port Multiplier
	//=========================================================================================
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	while (inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) {
		udelay(100);
		cpu_relax();
	}
	outb(inb(pSataChannel->BaseIoPort + 0x0010),pSataChannel->BaseIoPort + 0x0010);	// clear interrupt
	//=========================================================================================
	PM_FanOut = inb(pSataChannel->BaseIoPort + 0x0024) & 0x0F;
	if (PM_FanOut > MAX_SATA_HDD) {
		PM_FanOut = MAX_SATA_HDD;
	}
	pSataChannel->MaxDeviceCount = PM_FanOut;
	// send reset device command to device
	for (i = 0 ; i < pSataChannel->MaxDeviceCount ; i++) {
		if (PMxReset(pSataChannel,(unsigned char)i)) {
			status = inl(pSataChannel->BaseIoPort + 0x0024);
			if (status == 0xEB140101) {
				// ATAPI Device
				SendIdentifyCommand(pHwDeviceExtension,pDeviceInfo,i,0xA1);
			} else if (status == 0x00000101) {
				SendIdentifyCommand(pHwDeviceExtension,pDeviceInfo,i,0xEC);
			}
			pDeviceInfo->FG_OnSataChannel = 1;
		}
		pDeviceInfo++;
	}
	return;
}
//=======================================================================================
static void SendIdentifyCommand(PHW_DEVICE_EXTENSION DeviceExtension,P_DeviceInfo pDeviceInfo,int PMx,unsigned char IdenCmd)
{
#if __LITTLE_ENDIAN
#else
	unsigned char *pTemp;
#endif	
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_Channel pSataChannel;
	PUIdentify pIdentifyBuff;
	P_CommandHead pCommandHead;
	P_CommandTable pCommandTable;
	P_ReciveFIS pReciveFIS;
	int i;
	unsigned int ci_map;
	unsigned char Ultramode,UltraMap;
	unsigned char Error_status,status74;

	pDeviceInfo->FG_DeviceInstall = 0;
	pSataChannel = pDeviceInfo->pChannel;
	if ((inb(pSataChannel->BaseIoPort + 0x0028) & 0x0F) != 0x03) {
		// physical not ready
		return;
	}
	pIdentifyBuff = (PUIdentify)(pHwDeviceExtension->pIdentifyBuffer);
	status74 = inb(pSataChannel->BaseIoPort + 0x0074);
	pCommandHead  = pSataChannel->pCommandPtr->pCommandHead  + MAX_COMMAND_QUEUE_CH;
	pCommandTable = pSataChannel->pCommandPtr->pCommandTable + MAX_COMMAND_QUEUE_CH;
	pReciveFIS    = ((pSataChannel->pReciveFIS) + PMx);
	ci_map = (0x00000001 << MAX_COMMAND_QUEUE_CH);
	// Device presence detacted and physical ready,building Command Table
	pCommandTable->CommandFIS.FIS_Type = 0x27;
	pCommandTable->CommandFIS.PortMultiplier = (PMx & 0x0F); // Port Multiplier
	pCommandTable->CommandFIS.FIS_Flag = 1;
	pCommandTable->CommandFIS.P1x6 = 0xE0;
	pCommandTable->CommandFIS.Command = IdenCmd;
	// building PTD table
	pCommandTable->MailBox->BufAddressU = 0;
#if __LITTLE_ENDIAN
	pCommandTable->MailBox->BufAddress = pHwDeviceExtension->pDmaIdentifyBuffer;
	pCommandTable->MailBox->DataLength = 0x80000200;		// 512 bytes
#else
	pCommandTable->MailBox->BufAddress = cpu_to_le32(pHwDeviceExtension->pDmaIdentifyBuffer);
	pCommandTable->MailBox->DataLength = cpu_to_le32(0x80000200);		// 512 bytes
#endif
	// building Command List Entry
	pCommandHead->PortMultiplier = (PMx & 0x0F);
	pCommandHead->CommandFISLen = 0x05;
	pCommandHead->PRDTL = 0;
	pCommandHead->PRD_ByteCount = 0;
	// issure command
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	for (i = 0 ; i < 1000 ; i++) {
		// wait CI bit clear
		if ((inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) == 0) {
			// CI bit to be clear
			break;
		}
		udelay(1000);
		cpu_relax();
	}
	if (i >= 1000) {
		// time out
		outb(0x16,pSataChannel->BaseIoPort + 0x0018);
		outb(0x17,pSataChannel->BaseIoPort + 0x0018);
		outb(status74,pSataChannel->BaseIoPort + 0x0074); // restore Reg74
		return;
	}
	for (i = 0 ; i < 1000 ; i++) {
		//wait 1s for D2H/PIO FIS
		if (inb(pSataChannel->BaseIoPort + 0x0010) & 0x03) {
			// got D2H register FIS/ got PIO setup FIS
			break;
		}
		udelay(1000);
		cpu_relax();
	}
	if (i >= 1000) {
		// time out
		outb(0x16,pSataChannel->BaseIoPort + 0x0018);
		outb(0x17,pSataChannel->BaseIoPort + 0x0018);
		outb(status74,pSataChannel->BaseIoPort + 0x0074); // restore Reg74
		return;
	}
	Error_status = pReciveFIS->Device2HostFIS.Status;
	outb(0x03,pSataChannel->BaseIoPort + 0x0010); // clear interrupt
	if (Error_status & 0x01) {
		// Identify command error
		outb(status74,pSataChannel->BaseIoPort + 0x0074); // restore Reg74
		return;
	}
#if __LITTLE_ENDIAN
#else
	for (i = 0 ; i < 256 ; i+=2) {
		Ultramode = (pHwDeviceExtension->pIdentifyBuffer)[i];
		(pHwDeviceExtension->pIdentifyBuffer)[i] = (pHwDeviceExtension->pIdentifyBuffer)[i + 1];
		(pHwDeviceExtension->pIdentifyBuffer)[i+1] = Ultramode;
	}
#endif	
	if ((pIdentifyBuff->FG_ValidMap & 0x0004) == 0) {
		// not support ULTRA device
		outb(status74,pSataChannel->BaseIoPort + 0x0074);	// restore Reg74
		return;
	}
	pDeviceInfo->FG_DeviceInstall	 = 1;
	pDeviceInfo->FG_PMId		 = (unsigned char)PMx;
#if __LITTLE_ENDIAN
#else
	pIdentifyBuff->TotalLBA = ((pIdentifyBuff->TotalLBA >> 16) & 0x0000FFFF) +
							((pIdentifyBuff->TotalLBA << 16) & 0xFFFF0000);
#endif
	((pDeviceInfo->Capacity).U64L).U64L_H = 0;
	((pDeviceInfo->Capacity).U64C).U64C_L_HH = (u8)(pIdentifyBuff->TotalLBA >> 24);
	((pDeviceInfo->Capacity).U64C).U64C_L_HL = (u8)(pIdentifyBuff->TotalLBA >> 16);
	((pDeviceInfo->Capacity).U64C).U64C_L_LH = (u8)(pIdentifyBuff->TotalLBA >> 8);
	((pDeviceInfo->Capacity).U64C).U64C_L_LL = (u8)(pIdentifyBuff->TotalLBA);
#if __LITTLE_ENDIAN
	for (i = 0 ; i < 40 ; i += 2) {
		pDeviceInfo->ModelName[i] = pIdentifyBuff->ModelName[i+1];
		pDeviceInfo->ModelName[i+1] = pIdentifyBuff->ModelName[i];
	}
	for (i = 0 ; i < 20 ; i += 2) {
		pDeviceInfo->SerialNo[i] = pIdentifyBuff->SerialNo[i+1];
		pDeviceInfo->SerialNo[i+1] = pIdentifyBuff->SerialNo[i];
	}	
	for (i = 0 ; i < 8 ; i += 2) {
		pDeviceInfo->FirmWareRev[i] = pIdentifyBuff->FirmWareRev[i+1];
		pDeviceInfo->FirmWareRev[i+1] = pIdentifyBuff->FirmWareRev[i];
	}
#else
	for (i = 0 ; i < 20 ; i++) {
		pDeviceInfo->SerialNo[i] = pIdentifyBuff->SerialNo[i];
	}
	for (i = 0 ; i < 40 ; i++) {
		pDeviceInfo->ModelName[i] = pIdentifyBuff->ModelName[i];
	}
	for (i = 0 ; i < 8 ; i++) {
		pDeviceInfo->FirmWareRev[i] = pIdentifyBuff->FirmWareRev[i];
	}
#endif
	//=====================================================================================
	// check 48 bit LBA capability
	//=====================================================================================
	if (IdenCmd == 0xEC) {
#if __LITTLE_ENDIAN
#else
		pIdentifyBuff->LBA_48bits_High = ((pIdentifyBuff->LBA_48bits_High >> 16) & 0x0000FFFF) +
							((pIdentifyBuff->LBA_48bits_High << 16) & 0xFFFF0000);
		pIdentifyBuff->LBA_48bits_Low = ((pIdentifyBuff->LBA_48bits_Low >> 16) & 0x0000FFFF) +
							((pIdentifyBuff->LBA_48bits_Low << 16) & 0xFFFF0000);
#endif
		if (pIdentifyBuff->Word83 & 0x0400) {
			if ( 	((pIdentifyBuff->LBA_48bits_Low == 0) && (pIdentifyBuff->LBA_48bits_High == 0))||
				((pIdentifyBuff->LBA_48bits_Low == 0xFFFFFFFF) && (pIdentifyBuff->LBA_48bits_High == 0xFFFFFFFF))
			   ) {
				pDeviceInfo->FG_Support48Bits = 0;
			} else {
				pDeviceInfo->FG_Support48Bits = 1;
				((pDeviceInfo->Capacity).U64C).U64C_H_HH = (u8)(pIdentifyBuff->LBA_48bits_High >> 24);
				((pDeviceInfo->Capacity).U64C).U64C_H_HL = (u8)(pIdentifyBuff->LBA_48bits_High >> 16);
				((pDeviceInfo->Capacity).U64C).U64C_H_LH = (u8)(pIdentifyBuff->LBA_48bits_High >> 8);
				((pDeviceInfo->Capacity).U64C).U64C_H_LL = (u8)(pIdentifyBuff->LBA_48bits_High);
				((pDeviceInfo->Capacity).U64C).U64C_L_HH = (u8)(pIdentifyBuff->LBA_48bits_Low >> 24);
				((pDeviceInfo->Capacity).U64C).U64C_L_HL = (u8)(pIdentifyBuff->LBA_48bits_Low >> 16);
				((pDeviceInfo->Capacity).U64C).U64C_L_LH = (u8)(pIdentifyBuff->LBA_48bits_Low >> 8);
				((pDeviceInfo->Capacity).U64C).U64C_L_LL = (u8)(pIdentifyBuff->LBA_48bits_Low);
			}
		} else {
			pDeviceInfo->FG_Support48Bits = 0;
		}
		pDeviceInfo->MaxQueueDepth = (unsigned char)(pIdentifyBuff->QueueDepth & 0x000F) + 1;
#if DISK_NCQ_CHECK
	printk("SATA1 support : %d ,SATA2 support : %d ,NCQ supported : %d ,QueueDepth : %d\n\r",
			pIdentifyBuff->FG_SATA1_support,pIdentifyBuff->FG_SATA2_support,
						pIdentifyBuff->FG_NcqSupport,pIdentifyBuff->QueueDepth);
#endif
#if NCQ_SUPPORT
		if (	(pDeviceInfo->MaxQueueDepth > 1) &&
			(pIdentifyBuff->FG_NcqSupport) &&
			(pIdentifyBuff->FG_SATA2_support)
		   ) {
			pDeviceInfo->FG_QueueSupported = 1;
			if (pDeviceInfo->MaxQueueDepth > MAX_COMMAND_QUEUE_CH) {
				pDeviceInfo->MaxQueueDepth = MAX_COMMAND_QUEUE_CH;
			}
		} else {
			pDeviceInfo->FG_QueueSupported = 0;
			pDeviceInfo->MaxQueueDepth = 1;
		}
#else
		pDeviceInfo->FG_QueueSupported = 0;
		pDeviceInfo->MaxQueueDepth = 1;

#endif
		pDeviceInfo->FG_AtapiDevice = 0;
		pDeviceInfo->FG_DiscDevice  = 0;
		pDeviceInfo->DeviceBlockSize = 512;
	} else {
		pDeviceInfo->FG_Support48Bits	= 0;
		pDeviceInfo->MaxQueueDepth	= 1;
		pDeviceInfo->FG_QueueSupported	= 0;
		((pDeviceInfo->Capacity).U64L).U64L_L = 0;
		((pDeviceInfo->Capacity).U64L).U64L_H = 0;
		pDeviceInfo->FG_AtapiDevice	= 1;
		if ((pIdentifyBuff->ConfigBitInfo & 0x1f00)==0x0500) {
			// CD-ROM device
			pDeviceInfo->FG_DiscDevice  = 1;
			pDeviceInfo->DeviceBlockSize = 2048;
		} else {
			// Direct access device
			pDeviceInfo->FG_DiscDevice  = 0;
			pDeviceInfo->DeviceBlockSize = 512;
		}
	}
	//=====================================================================================
	// set following for IDE device which useing bridge to convert to SATA
	// some NCQ device need following procedure to make NCQ commnad effect (Hitachi HDT7225XX serial HDD)
	//=====================================================================================
	UltraMap = pIdentifyBuff->UltraModeMap;
	pDeviceInfo->DMA_Mode	= 0;
	if (UltraMap & 0x40) {
		// Ultra 133
		pDeviceInfo->DMA_Mode = 7;
	} else if (UltraMap & 0x20) {
		// Ultra 100
		pDeviceInfo->DMA_Mode = 6;
	} else if (UltraMap & 0x10) {
		// Ultra 66
		pDeviceInfo->DMA_Mode = 5;
	} else if (UltraMap & 0x08) {
		// Ultra 44
		pDeviceInfo->DMA_Mode = 4;
	} else if (UltraMap & 0x04) {
		// Ultra 33
		pDeviceInfo->DMA_Mode = 3;
	} else if (UltraMap & 0x02) {
		// Ultra 1 
		pDeviceInfo->DMA_Mode = 2;
	} else if (UltraMap & 0x01) {
		// Ultra 0 
		pDeviceInfo->DMA_Mode = 1;
	}
	Ultramode = pDeviceInfo->DMA_Mode;
	Ultramode--;
	Ultramode |= 0x40;
	pCommandTable->CommandFIS.P1x1 = 0x03;
	pCommandTable->CommandFIS.P1x2 = Ultramode;
	pCommandTable->CommandFIS.P1x6 = 0xE0;
	pCommandTable->CommandFIS.Command = 0xEF;
	// issure command
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	for (i = 0 ; i < 1000 ; i++) {
		// wait CI bit clear
		if ((inl(pSataChannel->BaseIoPort + 0x0038) & ci_map) == 0) {
			// CI bit to be clear
			break;
		}
		udelay(1000);
		cpu_relax();
	}
	if (i >= 1000) {
		// time out
		outb(0x16,pSataChannel->BaseIoPort + 0x0018);
		outb(0x17,pSataChannel->BaseIoPort + 0x0018);
		outb(status74,pSataChannel->BaseIoPort + 0x0074); // restore Reg74
		return;
	}
	for (i = 0 ; i < 1000 ; i++) {
		//wait 1s for D2H/PIO FIS
		if (inb(pSataChannel->BaseIoPort + 0x0010) & 0x03) {
			// got D2H register FIS/ got PIO setup FIS
			break;
		}
		udelay(1000);
		cpu_relax();
	}
	if (i >= 1000) {
		// time out
		outb(0x16,pSataChannel->BaseIoPort + 0x0018);
		outb(0x17,pSataChannel->BaseIoPort + 0x0018);
		outb(status74,pSataChannel->BaseIoPort + 0x0074); // restore Reg74
		return;
	}
	Error_status = pReciveFIS->Device2HostFIS.Status;
	outb(0x03,pSataChannel->BaseIoPort + 0x0010); // clear interrupt
	outb(status74,pSataChannel->BaseIoPort + 0x0074); // restore PM bit
	return;
}
//=======================================================================================
static void ExecuteSataATA(PHW_DEVICE_EXTENSION DeviceExtension,P_WorkingTable pWorkingTable)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_DeviceInfo pDeviceInfo;
	P_Channel pSataChannel;
	P_CommandHead pCommandHead;
	P_CommandTable pCommandTable;
	Scsi_Cmnd *pScsiCmd;
	unsigned int ci_map;
	unsigned char status;

	pDeviceInfo = pWorkingTable->pDevice;
	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	pSataChannel = pDeviceInfo->pChannel;
	//building Command Table
	pCommandHead = (pSataChannel->pCommandPtr->pCommandHead + pWorkingTable->TagID);
	pCommandTable = (pSataChannel->pCommandPtr->pCommandTable + pWorkingTable->TagID);
	pCommandTable->CommandFIS.FIS_Type = 0x27;
	pCommandTable->CommandFIS.FIS_Flag = 1;
	pCommandTable->CommandFIS.PortMultiplier = pDeviceInfo->FG_PMId;
	if (pDeviceInfo->FG_QueueSupported) {
		pCommandTable->CommandFIS.P1x1 = (((pWorkingTable->TransferLen).U64C).U64C_L_LL);
		pCommandTable->CommandFIS.P1x1Exp = (((pWorkingTable->TransferLen).U64C).U64C_L_LH);
		pCommandTable->CommandFIS.P1x2 = (((pWorkingTable->TagID) << 3) & 0xF8);
		pCommandTable->CommandFIS.P1x2Exp = 0;
		pCommandTable->CommandFIS.P1x3 = (((pWorkingTable->StartLBA).U64C).U64C_L_LL);
		pCommandTable->CommandFIS.P1x4 = (((pWorkingTable->StartLBA).U64C).U64C_L_LH);
		pCommandTable->CommandFIS.P1x5 = (((pWorkingTable->StartLBA).U64C).U64C_L_HL);
		pCommandTable->CommandFIS.P1x3Exp = (((pWorkingTable->StartLBA).U64C).U64C_L_HH);
		pCommandTable->CommandFIS.P1x4Exp = (((pWorkingTable->StartLBA).U64C).U64C_H_LL);
		pCommandTable->CommandFIS.P1x5Exp = (((pWorkingTable->StartLBA).U64C).U64C_H_LH);
		pCommandTable->CommandFIS.P1x6 = 0x40;
		if (	(pScsiCmd->cmnd[0] == READ_6)||
			(pScsiCmd->cmnd[0] == READ_10)||
			(pScsiCmd->cmnd[0] == READ_12)||
			(pScsiCmd->cmnd[0] == READ_16)
		   ) {
			pWorkingTable->XferCmd = IDE_NCQ_READ;
			pCommandHead->ReadWrite = 0;
		} else {
			pWorkingTable->XferCmd = IDE_NCQ_WRITE;
			pCommandHead->ReadWrite = 1;
		}
	} else {
		if (	(pWorkingTable->XferCmd == IDE_READ_DMA_EXT)||
			(pWorkingTable->XferCmd == IDE_WRITE_DMA_EXT)
		   ) {
			pCommandTable->CommandFIS.P1x1 = 0;
			pCommandTable->CommandFIS.P1x1Exp = 0;
			pCommandTable->CommandFIS.P1x2 = (((pWorkingTable->TransferLen).U64C).U64C_L_LL);
			pCommandTable->CommandFIS.P1x2Exp = (((pWorkingTable->TransferLen).U64C).U64C_L_LH);
			pCommandTable->CommandFIS.P1x3 = (((pWorkingTable->StartLBA).U64C).U64C_L_LL);
			pCommandTable->CommandFIS.P1x4 = (((pWorkingTable->StartLBA).U64C).U64C_L_LH);
			pCommandTable->CommandFIS.P1x5 = (((pWorkingTable->StartLBA).U64C).U64C_L_HL);
			pCommandTable->CommandFIS.P1x3Exp = (((pWorkingTable->StartLBA).U64C).U64C_L_HH);
			pCommandTable->CommandFIS.P1x4Exp = (((pWorkingTable->StartLBA).U64C).U64C_H_LL);
			pCommandTable->CommandFIS.P1x5Exp = (((pWorkingTable->StartLBA).U64C).U64C_H_LH);
			pCommandTable->CommandFIS.P1x6 = 0xE0;
			if (	(pScsiCmd->cmnd[0] == READ_6)||
				(pScsiCmd->cmnd[0] == READ_10)||
				(pScsiCmd->cmnd[0] == READ_12)||
				(pScsiCmd->cmnd[0] == READ_16)
			   ) {
				pCommandHead->ReadWrite = 0;
			} else {
				pCommandHead->ReadWrite = 1;
			}
		} else {
			pCommandTable->CommandFIS.P1x1 = 0;
			pCommandTable->CommandFIS.P1x1Exp = 0;
			pCommandTable->CommandFIS.P1x2 = (((pWorkingTable->TransferLen).U64C).U64C_L_LL);
			pCommandTable->CommandFIS.P1x2Exp = (((pWorkingTable->TransferLen).U64C).U64C_L_LH);
			pCommandTable->CommandFIS.P1x3 = (((pWorkingTable->StartLBA).U64C).U64C_L_LL);
			pCommandTable->CommandFIS.P1x4 = (((pWorkingTable->StartLBA).U64C).U64C_L_LH);
			pCommandTable->CommandFIS.P1x5 = (((pWorkingTable->StartLBA).U64C).U64C_L_HL);
			pCommandTable->CommandFIS.P1x3Exp = 0;
			pCommandTable->CommandFIS.P1x4Exp = 0;
			pCommandTable->CommandFIS.P1x5Exp = 0;
			pCommandTable->CommandFIS.P1x6 = (0xE0 | ((((pWorkingTable->StartLBA).U64C).U64C_L_HH) & 0x0F));
			if (	(pScsiCmd->cmnd[0] == READ_6)||
				(pScsiCmd->cmnd[0] == READ_10)||
				(pScsiCmd->cmnd[0] == READ_12)||
				(pScsiCmd->cmnd[0] == READ_16)
			   ) {
				pCommandHead->ReadWrite = 0;
			} else {
				pCommandHead->ReadWrite = 1;
			}
		}
	}
	pCommandTable->CommandFIS.Command = pWorkingTable->XferCmd;
	// building PTD table
	BuildPrdTable(pHwDeviceExtension,pWorkingTable,&(pCommandTable->MailBox[0]));
	// building Command List Entry
	pCommandHead->CommandFISLen = 5;
	pCommandHead->AtapiPioFIS = 0;
	pCommandHead->PreFetchAble = 0;
	pCommandHead->Reset = 0;
	pCommandHead->Bist = 0;
	pCommandHead->ClearBusy = 0;
	pCommandHead->Reserved0 = 0;
	pCommandHead->PortMultiplier = pDeviceInfo->FG_PMId;
	pCommandHead->PRDTL = 0;
	pCommandHead->PRD_ByteCount = 0;
	// issure command
	status = inb(pSataChannel->BaseIoPort + 0x0074) & 0xFE;
	ci_map = 0x00000001;
	ci_map <<= pWorkingTable->TagID;
	if (pDeviceInfo->FG_QueueSupported) {
		status |= 0x01; // enable NCQ bit
		outb(status,pSataChannel->BaseIoPort + 0x0074);
		outl(ci_map,pSataChannel->BaseIoPort + 0x0034);
	} else {
		outb(status,pSataChannel->BaseIoPort + 0x0074);
	}
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	return;
}
static void ExecuteUpdata(PHW_DEVICE_EXTENSION DeviceExtension,P_WorkingTable pWorkingTable)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_DeviceInfo pDeviceInfo;
	P_Channel pSataChannel;
	P_CommandHead pCommandHead;
	P_CommandTable pCommandTable;
	Scsi_Cmnd *pScsiCmd;
	unsigned int ci_map;
	unsigned char status;

	pDeviceInfo = pWorkingTable->pDevice;
	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	pSataChannel = pDeviceInfo->pChannel;
	//building Command Table
	pCommandHead = (pSataChannel->pCommandPtr->pCommandHead + pWorkingTable->TagID);
	pCommandTable = (pSataChannel->pCommandPtr->pCommandTable + pWorkingTable->TagID);
	pCommandTable->CommandFIS.FIS_Type = 0x27;
	pCommandTable->CommandFIS.FIS_Flag = 1;
	pCommandTable->CommandFIS.PortMultiplier = pDeviceInfo->FG_PMId;			
	pCommandTable->CommandFIS.P1x1Exp = 0;
	pCommandTable->CommandFIS.P1x2	  = pScsiCmd->cmnd[8];
	pCommandTable->CommandFIS.P1x2Exp = 0;
	pCommandTable->CommandFIS.P1x3 = pScsiCmd->cmnd[3];
	pCommandTable->CommandFIS.P1x4 = pScsiCmd->cmnd[4];
	pCommandTable->CommandFIS.P1x5 = pScsiCmd->cmnd[5];
	pCommandTable->CommandFIS.P1x3Exp = 0;
	pCommandTable->CommandFIS.P1x4Exp = 0;
	pCommandTable->CommandFIS.P1x5Exp = 0;
	pCommandTable->CommandFIS.P1x6 = (0xE0 | ((((pWorkingTable->StartLBA).U64C).U64C_L_HH) & 0x0F));
	if (pScsiCmd->cmnd[0] == Updata_NonData	) {
		pCommandTable->CommandFIS.P1x1 = pScsiCmd->cmnd[2];
		pCommandHead->ReadWrite = 0;
		pCommandTable->CommandFIS.Command = 0xFC;
		pWorkingTable->FG_NonData = 1;
	} else if(pScsiCmd->cmnd[0] == Fireware_ReadBuffer){
		pCommandTable->CommandFIS.P1x1 = 0x3C;
		pCommandHead->ReadWrite = 0;
		pCommandTable->CommandFIS.Command = 0xFD;
	}else {
		pCommandTable->CommandFIS.P1x1 = 0x3B;
		pCommandHead->ReadWrite = 1;
		pCommandTable->CommandFIS.Command = 0xFD;
	}	
	
	// building PTD table
	if (pWorkingTable->FG_NonData == 0)
	{
		BuildPrdTable(pHwDeviceExtension,pWorkingTable,&(pCommandTable->MailBox[0]));
	}
	// building Command List Entry
	pCommandHead->CommandFISLen = 5;
	pCommandHead->AtapiPioFIS = 0;
	pCommandHead->PreFetchAble = 0;
	pCommandHead->Reset = 0;
	pCommandHead->Bist = 0;
	pCommandHead->ClearBusy = 0;
	pCommandHead->Reserved0 = 0;
	pCommandHead->PortMultiplier = pDeviceInfo->FG_PMId;
	pCommandHead->PRDTL = 0;
	pCommandHead->PRD_ByteCount = 0;
	// issure command
	status = inb(pSataChannel->BaseIoPort + 0x0074) & 0xFE;
	ci_map = 0x00000001;
	ci_map <<= pWorkingTable->TagID;	
	outb(status,pSataChannel->BaseIoPort + 0x0074);	
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	return;
}
//=======================================================================================
static void ExecuteSataATAPI(PHW_DEVICE_EXTENSION DeviceExtension,P_WorkingTable pWorkingTable)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_DeviceInfo pDeviceInfo;
	P_Channel pSataChannel;
	P_CommandHead pCommandHead;
	P_CommandTable pCommandTable;
	Scsi_Cmnd *pScsiCmd;
	unsigned int ci_map;
	unsigned char status;
	int i;

	pDeviceInfo = pWorkingTable->pDevice;
	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	pSataChannel = pDeviceInfo->pChannel;
	//building Command Table
	pCommandHead = (pSataChannel->pCommandPtr->pCommandHead + pWorkingTable->TagID);
	pCommandTable = (pSataChannel->pCommandPtr->pCommandTable + pWorkingTable->TagID);
	pCommandTable->CommandFIS.FIS_Type = 0x27;
	pCommandTable->CommandFIS.FIS_Flag = 1;
	pCommandTable->CommandFIS.PortMultiplier = pDeviceInfo->FG_PMId;
	ConvertCmdATAPI(pWorkingTable);
	//building Command Table
	if (pWorkingTable->request_bufflen) { 
		// Do DMA
		pCommandTable->CommandFIS.P1x1 = 1;
		pWorkingTable->FG_DoIoByDMA = 1;
	} else {
		// Do PIO
		pCommandTable->CommandFIS.P1x1 = 0;
		pWorkingTable->FG_DoIoByDMA = 0;
	}
	pCommandTable->CommandFIS.P1x1Exp = 0;
	pCommandTable->CommandFIS.P1x2 = 0;
	pCommandTable->CommandFIS.P1x2Exp = 0;
	pCommandTable->CommandFIS.P1x3 = 0;
	pCommandTable->CommandFIS.P1x4 = 0xFF;
	pCommandTable->CommandFIS.P1x5 = 0xFF;
	pCommandTable->CommandFIS.P1x3Exp = 0;
	pCommandTable->CommandFIS.P1x4Exp = 0;
	pCommandTable->CommandFIS.P1x5Exp = 0;
	pCommandTable->CommandFIS.P1x6 = 0x40;
	pCommandTable->CommandFIS.Command = 0xA0;
	for (i = 0 ; i < 16 ; i++) {
		pCommandTable->AtapiCommnad[i] = pScsiCmd->cmnd[i];
	}
	if (pScsiCmd->sc_data_direction == DMA_TO_DEVICE) {
		pCommandHead->ReadWrite	= 1;
	} else {
		pCommandHead->ReadWrite	= 0;
	}
	if (pWorkingTable->request_bufflen) {
		// building PTD table
		BuildPrdTable(pHwDeviceExtension,pWorkingTable,&(pCommandTable->MailBox[0]));
	}
	// building Command List Entry
	pCommandHead->CommandFISLen = 5;
	pCommandHead->AtapiPioFIS = 1;
	pCommandHead->PreFetchAble = 0;
	pCommandHead->Reset = 0;
	pCommandHead->Bist = 0;
	pCommandHead->ClearBusy = 0;
	pCommandHead->Reserved0 = 0;
	pCommandHead->PortMultiplier = pDeviceInfo->FG_PMId;
	pCommandHead->PRDTL = 0;
	pCommandHead->PRD_ByteCount = 0;
	// issure command
	status = inb(pSataChannel->BaseIoPort + 0x0074) & 0xFE;
	outb(status,pSataChannel->BaseIoPort + 0x0074);
	ci_map = 0x00000001;
	ci_map <<= pWorkingTable->TagID;
	outl(ci_map,pSataChannel->BaseIoPort + 0x0038);
	return;
}
//=======================================================================================
static void BuildPrdTable(PHW_DEVICE_EXTENSION DeviceExtension,P_WorkingTable pWorkingTable,P_MailBox pMailBox)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	Scsi_Cmnd *pScsiCmd;
	struct scatterlist *sg_list,*sg;
	P_MailBox pmailbox;
	P_DeviceInfo pDeviceInfo;
	int sg_num,i,j;
	u32 sg_addr,sg_len;

	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	pDeviceInfo = pWorkingTable->pDevice;
	pmailbox = pMailBox;
	if (pWorkingTable->use_sg) {
		sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);

//		sg_num = scsi_dma_map(pScsiCmd);
//		target_id = scmd_id(pScsiCmd);
//		scsi_for_each_sg(pScsiCmd, sg_list, sg_num, target_id) {

		/* Add the below block for async cpu cache and memory,
		   k3b project found.
		   LiWengang <liwengang@loongson.cn>
		   Jason Luan <luanjianhai@ccoss.com.cn>
		 */
		int index = 0;
		for (index = 0; index < pWorkingTable->use_sg; index++){
			unsigned long  addr = (unsigned long) sg_virt(sg_list);
			dma_cache_wback_inv(addr, sg_list->length);
		}


		sg_num = pci_map_sg(pHwDeviceExtension->pPciDev,
					sg_list,
					pWorkingTable->use_sg,
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
					scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
# else
					pScsiCmd->sc_data_direction
# endif
				   );
#ifdef KERNEL_V2625
		for_each_sg(sg_list, sg, sg_num, i) {
#else
		for (j = 0 ; j < sg_num ; j++) {
#endif
			sg_addr	=  sg_dma_address(sg_list);
			sg_len	=  sg_dma_len(sg_list);
			while (sg_len > 0x00010000) {
				pmailbox->BufAddressU =  0;
#if __LITTLE_ENDIAN
				pmailbox->BufAddress = sg_addr;
				pmailbox->DataLength = 0x00010000;
#else
				pmailbox->BufAddress = cpu_to_le32(sg_addr);
				pmailbox->DataLength = cpu_to_le32(0x00010000);
#endif
				sg_addr += 0x00010000;
				sg_len -= 0x00010000;
				pmailbox++;
			} 
			pmailbox->BufAddressU =  0;
#if __LITTLE_ENDIAN
			pmailbox->BufAddress = sg_addr;
			pmailbox->DataLength = sg_len;
#else
			pmailbox->BufAddress = cpu_to_le32(sg_addr);
			pmailbox->DataLength = cpu_to_le32(sg_len);
#endif
			pmailbox++;

/*			sg_list = sg_next(sg_list);*/

#ifdef KERNEL_V2625
			sg_list = sg_next(sg_list);
#else
			sg_list++;
#endif
		}
		pmailbox--;
#if __LITTLE_ENDIAN
		pmailbox->DataLength |= 0x80000000;
#else
		pmailbox->DataLength |= 0x00000080;
#endif
	} else {
		if (  (pWorkingTable->pDmaSingleMapAddr = 
				pci_map_single(pHwDeviceExtension->pPciDev,
						pWorkingTable->request_buffer,
						pWorkingTable->request_bufflen,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
						scsi_to_pci_dma_dir(pScsiCmd->sc_data_direction)
#else
						pScsiCmd->sc_data_direction
#endif
					     )
		      ) == 0

		   ) {
			pDeviceInfo->SenseKey = 0x03;
			pDeviceInfo->SenseAsc = 0x55;
			pDeviceInfo->SenseAscQ = 0x01;
			pScsiCmd->result = SAM_STAT_CHECK_CONDITION;
			return;
		}
		sg_addr = pWorkingTable->pDmaSingleMapAddr;
		sg_len = pWorkingTable->request_bufflen;
		while (sg_len > 0x00010000) {
			pmailbox->BufAddressU = 0;
#if __LITTLE_ENDIAN
			pmailbox->BufAddress = sg_addr;
			pmailbox->DataLength = 0x00010000;
#else
			pmailbox->BufAddress = cpu_to_le32(sg_addr);
			pmailbox->DataLength = cpu_to_le32(0x00010000);
#endif
			sg_addr += 0x00010000;
			sg_len -= 0x00010000;
			pmailbox++;
		} 
		sg_len |= 0x80000000;
		pmailbox->BufAddressU = 0;
#if __LITTLE_ENDIAN
		pmailbox->BufAddress = sg_addr;
		pmailbox->DataLength = sg_len;
#else
		pmailbox->BufAddress = cpu_to_le32(sg_addr);
		pmailbox->DataLength = cpu_to_le32(sg_len);
#endif
	}
	return;
}
//=======================================================================================
static P_WorkingTable AllocateWorkingTable(PHW_DEVICE_EXTENSION DeviceExtension)
{
	PHW_DEVICE_EXTENSION pHwDeviceExtension = DeviceExtension;
	P_WorkingTable pWorkingTable;
	unsigned int i;

	pWorkingTable = (P_WorkingTable)(pHwDeviceExtension->pWorkingTablePool);
	for (i = 0 ; i < MAX_QUEUE_SUPPORT ; i++) {
		if (pWorkingTable->FG_TableActive == 0) {
			// found a empty TaskTable
			memset(pWorkingTable,0,sizeof(T_WorkingTable));
			pWorkingTable->FG_TableActive = 1;

			pWorkingTable->pMailBox = pHwDeviceExtension->pIdeMailBox;
			pWorkingTable->LmailBox = pHwDeviceExtension->pDmaIdeMailBox;
			return pWorkingTable;
		}
		pWorkingTable++;
	}
	printk("---!!! Warnning,WorkingTable pool empty !!!---\n");
	return 0;
}
//=======================================================================================
static void ReleaseWorkingTable(P_WorkingTable pWorkingTable)
{
	pWorkingTable->FG_TableActive = 0;
	return;
}
//=======================================================================================
static void ChannelIoHook(P_WorkingTable pWorkingTable)
{
	P_Channel pChannel;
	P_WorkingTable pWorkingTable1;

	if (pWorkingTable->pDevice == 0) {
		printk("Channel Io Hook : Device of WorkingTable lost,IoTagID:%ld \n\r",pWorkingTable->IoTagID);
		return;
	}
	pChannel = ((P_DeviceInfo)(pWorkingTable->pDevice))->pChannel;
	if (pChannel->pPendingIO == 0) {
		pChannel->pPendingIO	= pWorkingTable;
		pWorkingTable->pPrevTable	= 0;
	} else {
		pWorkingTable1 = pChannel->pPendingIO;
		while (pWorkingTable1->pNextTable != 0) {
			pWorkingTable1 = pWorkingTable1->pNextTable;
		}
		pWorkingTable1->pNextTable = pWorkingTable;
		pWorkingTable->pPrevTable = pWorkingTable1;
		pWorkingTable->pNextTable = 0;
	}
	return;
}
//=======================================================================================
static void ChannelIoRemove(P_WorkingTable pWorkingTable)
{
	P_WorkingTable pWorkingTable2;
	P_Channel pChannel;

	if (pWorkingTable->pDevice == 0) {
		printk("ChannelIoRemove : Device of WorkingTable lost,IoTagID:%ld \n\r",pWorkingTable->IoTagID);
		return;
	}
	pChannel = ((P_DeviceInfo)(pWorkingTable->pDevice))->pChannel;
	if (pWorkingTable->pPrevTable == 0) {
		// this is first task table in task queue
		pChannel->pPendingIO = pWorkingTable->pNextTable;
		if (pWorkingTable->pNextTable != 0) {
			((P_WorkingTable)(pWorkingTable->pNextTable))->pPrevTable = 0;
		}
	} else {
		pWorkingTable2 = pWorkingTable->pPrevTable;
		pWorkingTable2->pNextTable = pWorkingTable->pNextTable;
		if (pWorkingTable->pNextTable != 0) {
			((P_WorkingTable)(pWorkingTable->pNextTable))->pPrevTable = pWorkingTable2;
		}
	}
	return;
}
//========================================================================
static void ConvertCommandATA(P_WorkingTable pWorkingTable)
{
	T_U64 EndLBA;
	Scsi_Cmnd *pScsiCmd;
	P_DeviceInfo pHardDiskInfo;
	unsigned char IdeCommand;

	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	pHardDiskInfo = pWorkingTable->pDevice;
	AddU64(&(pWorkingTable->StartLBAOrg),&(pWorkingTable->TransferLenOrg),&EndLBA);
	switch(pScsiCmd->cmnd[0]) {
	case READ_6:
		IdeCommand = IDE_READ_DMA;
		break;
	case READ_10:
	case READ_12:
	case READ_16:
		if (  (((EndLBA.U64L).U64L_H) != 0) || ((EndLBA.U64L).U64L_L & 0xF0000000)  ) {
			IdeCommand = IDE_READ_DMA_EXT;
		} else {
			IdeCommand = IDE_READ_DMA;
		}
		break;
	case WRITE_6:
		IdeCommand = IDE_WRITE_DMA;
		break;
	case WRITE_10:
	case WRITE_12:
	case WRITE_16:
		if (  (((EndLBA.U64L).U64L_H) != 0) || ((EndLBA.U64L).U64L_L & 0xF0000000)  ) {
			IdeCommand = IDE_WRITE_DMA_EXT;
		} else {
			IdeCommand = IDE_WRITE_DMA;
		}
		break;
	case SEEK_10:
	case SEEK_6:
		IdeCommand = IDE_SEEK;
		break;
	case VERIFY:
		if (  (((EndLBA.U64L).U64L_H) != 0) || ((EndLBA.U64L).U64L_L & 0xF0000000)  ) {
			IdeCommand = IDE_VERIFY_EXT;
		} else {
			IdeCommand = IDE_VERIFY;
		}
		break;
	case SYNCHRONIZE_CACHE:
		if (pHardDiskInfo->FG_Support48Bits) {
			IdeCommand = IDE_FLUSH_CACHE_EXT;
		} else {
			IdeCommand = IDE_FLUSH_CACHE;
		}
		break;
	default:
		IdeCommand = 0;
		break;
	}
	pWorkingTable->XferCmd = IdeCommand;
	return;
}
//========================================================================
static void ConvertCmdATAPI(P_WorkingTable pWorkingTable)
{
	P_Channel pIdeChannel;
	P_DeviceInfo pCdRomInfo;
	struct scatterlist *sg_list;
	Scsi_Cmnd *pScsiCmd;
	unsigned long km_flags;
	unsigned char *km_buffer;
	unsigned char *SrbExtDataBuffer,*SrbDataBuffer;
	int i,km_sg_used;
	unsigned short blklen,pagelen,BlockLen;

	km_buffer = NULL;
	km_sg_used = 0;
	km_flags = 0;
	pScsiCmd = pWorkingTable->pWorkingScsiCmd;
	pCdRomInfo = pWorkingTable->pDevice;
	pIdeChannel = pCdRomInfo->pChannel; 
	for (i = pScsiCmd->cmd_len ; i < 12; i++) {
		pScsiCmd->cmnd[i] = 0;
	}
	// clear remain CDB
	switch(pScsiCmd->cmnd[0]) {
	case READ_6:
	case READ_10:
		BlockLen = pCdRomInfo->DeviceBlockSize;
		if ((BlockLen != 2048) && (BlockLen != 512)) {
			// backup original srb
			for (i = 0 ; i < 12 ; i++) {
				pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
			}
			pScsiCmd->cmnd[0] = 0xBE;
			pScsiCmd->cmnd[1] = 0;
			pScsiCmd->cmnd[6] = 0;
			switch(BlockLen) {
			case 0x0920:
				pScsiCmd->cmnd[9] = 0x58;
				pScsiCmd->cmnd[10] = 0;
				pScsiCmd->cmnd[11] = 0;
				break;	
			case 0x0930:
				pScsiCmd->cmnd[9] = 0xF8;
				pScsiCmd->cmnd[10] = 0;
				pScsiCmd->cmnd[11] = 0;
				break;
			case 0x0940:
				pScsiCmd->cmnd[9] = 0xF8;
				pScsiCmd->cmnd[10] = 0x02;
				pScsiCmd->cmnd[11] = 0;
				break;
			case 0x0990:
				pScsiCmd->cmnd[9] = 0xF8;
				pScsiCmd->cmnd[10] = 0x04;
				pScsiCmd->cmnd[11] = 0;
				break;
			default:
				pScsiCmd->cmnd[9] = 0x10;
				pScsiCmd->cmnd[10] = 0;
				pScsiCmd->cmnd[11] = 0;
				break;
			}
		}
		break;
/*
	case 0x46:
		for (i = 0 ; i < 12 ; i++) {
			pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
		//	pScsiCmd->cmnd[i] = 0;
		}
		pWorkingTable->pOrgRequestBuffLen = pWorkingTable->request_bufflen;
		pWorkingTable->pOrgRequesrBuff = pWorkingTable->request_buffer;
		pWorkingTable->old_use_sg = pWorkingTable->use_sg;
		pWorkingTable->use_sg = 0;
		pWorkingTable->request_buffer = pWorkingTable->DataBuffer;
		pWorkingTable->pDataBufPtr = pWorkingTable->DataBuffer;
		break;
*/
	case MODE_SENSE:
		for (i = 0 ; i < 12 ; i++) {
			pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
			pScsiCmd->cmnd[i]	  = 0;
		}
		pWorkingTable->pOrgRequestBuffLen = pWorkingTable->request_bufflen;
		pWorkingTable->pOrgRequesrBuff = pWorkingTable->request_buffer;
		pWorkingTable->old_use_sg = pWorkingTable->use_sg;
		// rebuild CDB
		pScsiCmd->cmnd[0] = MODE_SENSE_10;			// 1A -> 5A
		pScsiCmd->cmnd[2] = pWorkingTable->OrgCmnd[2];	// PC flag & Page Code
		pScsiCmd->cmnd[8] = pWorkingTable->OrgCmnd[4];	// Allocation length
		pWorkingTable->use_sg = 0;
		pWorkingTable->request_buffer = pWorkingTable->DataBuffer;
		pWorkingTable->pDataBufPtr = pWorkingTable->DataBuffer;
		break;
	case MODE_SENSE_10:
		for (i = 0 ; i < 12 ; i++) {
			pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
			pScsiCmd->cmnd[i]	  = 0;
		}
		pWorkingTable->pOrgRequestBuffLen = pWorkingTable->request_bufflen;
		pWorkingTable->pOrgRequesrBuff = pWorkingTable->request_buffer;
		pWorkingTable->old_use_sg = pWorkingTable->use_sg;
		// rebuild CDB
		
		pScsiCmd->cmnd[0] = MODE_SENSE_10;		   // 1A -> 5A
	/*	pScsiCmd->cmnd[1] = pWorkingTable->OrgCmnd[1]; */
		pScsiCmd->cmnd[2] = pWorkingTable->OrgCmnd[2]; // PC flag & Page Code
		pScsiCmd->cmnd[7] = pWorkingTable->OrgCmnd[7]; // Allocation length
		pScsiCmd->cmnd[8] = pWorkingTable->OrgCmnd[8]; // Allocation length

		pWorkingTable->use_sg = 0;
		pWorkingTable->request_buffer = pWorkingTable->DataBuffer;
		pWorkingTable->pDataBufPtr = pWorkingTable->DataBuffer;
		break;
	case MODE_SELECT:
		for (i = 0 ; i < 12 ; i++) {
			pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
			pScsiCmd->cmnd[i] = 0;
		}
		pWorkingTable->pOrgRequestBuffLen = pWorkingTable->request_bufflen;
		pWorkingTable->pOrgRequesrBuff = pWorkingTable->request_buffer;
		pWorkingTable->old_use_sg = pWorkingTable->use_sg;
		km_sg_used = 0;
		sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
		if (pWorkingTable->use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
			SrbDataBuffer = (unsigned char *)(sg_list->address);
#else
			local_irq_save(km_flags);
			km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//			km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
			SrbDataBuffer = km_buffer;
			km_sg_used = 1;
#endif
		} else {
			SrbDataBuffer =  (unsigned char *)(pWorkingTable->request_buffer);
		}		
		SrbExtDataBuffer = (unsigned char *)pWorkingTable->DataBuffer;
		blklen = SrbDataBuffer[3];		// get block descriptor length
		pagelen = SrbDataBuffer[blklen+5];	// get page len
		// rebuild mode parameter header
		for (i = 0 ; i < 8 ; i++) {
			SrbExtDataBuffer[i] = 0;	// all field is reserved when mode select
		}
		// get read/write block length
		
		if (blklen >= 8) {
			BlockLen =  ((((unsigned short)(SrbDataBuffer[9]))<<16)&0x00FF0000)+
				    ((((unsigned short)(SrbDataBuffer[10]))<<8)&0x0000FF00)+
				     (((unsigned short)(SrbDataBuffer[11]))&0x000000FF);
			if (BlockLen != 0) {
				pCdRomInfo->DeviceBlockSize = (unsigned short)BlockLen;
			}
		}
		// rebuild page list
		SrbDataBuffer += (4+blklen);// skip header & block descriptor
		SrbExtDataBuffer += 8;	// skip header
		pagelen += 2;		// total page list length
		for (i = 0 ; i < pagelen ; i++) {
			SrbExtDataBuffer[i] = SrbDataBuffer[i];
		}
		pagelen += 8;		// total transfer length
		pWorkingTable->use_sg = 0;
		pWorkingTable->request_bufflen = pagelen;
		pWorkingTable->request_buffer = pWorkingTable->DataBuffer;
		pWorkingTable->pDataBufPtr= pWorkingTable->DataBuffer;
		// rebuild CDB
		pScsiCmd->cmnd[0] = MODE_SELECT_10;
		pScsiCmd->cmnd[1] = ((pWorkingTable->OrgCmnd[1] & 0x01)|0x10);	// keep SP & set PF=1
		pScsiCmd->cmnd[7] = (unsigned char)(pagelen >> 8);
		pScsiCmd->cmnd[8] = (unsigned char)pagelen;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#else
		if (km_sg_used) {
			kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
			local_irq_restore(km_flags);
		}
#endif
		break;
	case MODE_SELECT_10:
		for (i = 0 ; i < 12 ; i++) {
			pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
			pScsiCmd->cmnd[i] = 0;
		}
		pWorkingTable->pOrgRequestBuffLen = pWorkingTable->request_bufflen;
		pWorkingTable->pOrgRequesrBuff = pWorkingTable->request_buffer;
		pWorkingTable->old_use_sg = pWorkingTable->use_sg;		
		sg_list = (struct scatterlist *)(pWorkingTable->request_buffer);
		if (pWorkingTable->use_sg) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
			SrbDataBuffer = (unsigned char *)(sg_list->address);
#else
			local_irq_save(km_flags);
			km_buffer = kmap_atomic(sg_page(sg_list),KM_IRQ0) + sg_list->offset;
//			km_buffer = kmap_atomic(sg_list->page,KM_IRQ0) + sg_list->offset;
			SrbDataBuffer = km_buffer;
			km_sg_used = 1;
#endif
		} else {
			SrbDataBuffer =  (unsigned char *)(pWorkingTable->request_buffer);
		}		
		SrbExtDataBuffer = (unsigned char *)pWorkingTable->DataBuffer;
		blklen = (((unsigned short)SrbDataBuffer[6])<<8)+SrbDataBuffer[7]; // get block descriptor length
		pagelen = SrbDataBuffer[blklen+9];			     // get page len
		// rebuild mode parameter header
		for (i = 0 ; i < 8 ; i++) {
			SrbExtDataBuffer[i] = 0; // all filed are reserved when mode select
		}
		// get read/write block length
		if (blklen>0) {
			BlockLen =  ((((unsigned short)SrbDataBuffer[13])<<16)&0x00FF0000)+
				    ((((unsigned short)SrbDataBuffer[14])<<8)&0x0000FF00)+
				     (((unsigned short)SrbDataBuffer[15])&0x000000FF);
			if ((BlockLen != 0) && (SrbExtDataBuffer[0] != 5)) {
				pCdRomInfo->DeviceBlockSize = (unsigned short)BlockLen;
			}
		}
		// rebuild page list
		SrbDataBuffer += (8+blklen);// skip header & block descriptor
		SrbExtDataBuffer += 8;	// skip header
		pagelen += 2;		// total page list length
		for (i = 0 ; i < pagelen ; i++) {
			SrbExtDataBuffer[i] = SrbDataBuffer[i];
		}
		pagelen += 8;		// total transfer length
		if (pagelen % 2) {
			pagelen++;
		}		
		pWorkingTable->use_sg = 0;
		pWorkingTable->request_bufflen = pagelen;
		pWorkingTable->request_buffer = pWorkingTable->DataBuffer;
		pWorkingTable->pDataBufPtr= pWorkingTable->DataBuffer;
		// rebuild CDB		
		pScsiCmd->cmnd[0] = MODE_SELECT_10;
		pScsiCmd->cmnd[1] = ((pWorkingTable->OrgCmnd[1] & 0x01)|0x10);	// keep SP & set PF=1
		pScsiCmd->cmnd[7] = (unsigned char)(pagelen >>8);
		pScsiCmd->cmnd[8] = (unsigned char)pagelen;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) 
#else
		if (km_sg_used) {
			kunmap_atomic(km_buffer - sg_list->offset,KM_IRQ0);
			local_irq_restore(km_flags);
		}
#endif		
		break;
	case 0xCD:
		// backup original srb
		for (i = 0 ; i < 12 ; i++) {
			pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
		}
		pScsiCmd->cmnd[0] = 0xBA;
		break;
	case 0xD8:
		for (i = 0 ; i < 12 ; i++) {
			pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
		}
		pScsiCmd->cmnd[0] = ATAPI_READ_CD;
		pScsiCmd->cmnd[6] = pScsiCmd->cmnd[7];
		pScsiCmd->cmnd[7] = pScsiCmd->cmnd[8];
		pScsiCmd->cmnd[8] = pScsiCmd->cmnd[9];
		switch(pScsiCmd->cmnd[10]) {
		case 0:
			pScsiCmd->cmnd[9]  = 0xF8;
			pScsiCmd->cmnd[10] = 0;
			break;
		case 1:
			pScsiCmd->cmnd[9]  = 0xF8;
			pScsiCmd->cmnd[10] = 0x02;
			break;
		case 2:
			pScsiCmd->cmnd[9]  = 0xF8;
			pScsiCmd->cmnd[10] = 0x04;
			break;
		case 3:
			pScsiCmd->cmnd[9]  = 0x40;
			pScsiCmd->cmnd[10] = 0x04;
			break;
		}
		pScsiCmd->cmnd[11] = 0;
		break;
	case 0xD9:
		for (i = 0 ; i < 12 ; i++) {
			pWorkingTable->OrgCmnd[i] = pScsiCmd->cmnd[i];
		}
		pScsiCmd->cmnd[0] = 0xB9;
		pScsiCmd->cmnd[6] = pScsiCmd->cmnd[7];
		pScsiCmd->cmnd[7] = pScsiCmd->cmnd[8];
		pScsiCmd->cmnd[8] = pScsiCmd->cmnd[9];
		switch(pScsiCmd->cmnd[10]) {
		case 0:
			pScsiCmd->cmnd[9]  = 0xF8;
			pScsiCmd->cmnd[10] = 0;
			break;
		case 1:
			pScsiCmd->cmnd[9]  = 0xF8;
			pScsiCmd->cmnd[10] = 0x02;
			break;
		case 2:
			pScsiCmd->cmnd[9]  = 0xF8;
			pScsiCmd->cmnd[10] = 0x04;
			break;
		case 3:
			pScsiCmd->cmnd[9]  = 0x40;
			pScsiCmd->cmnd[10] = 0x04;
			break;
		}
		pScsiCmd->cmnd[11] = 0;
		break;
	}
	return;
}
//=======================================================================================
static void ConvertSenseKey(P_Channel pChannel,P_WorkingTable pWorkingTable,unsigned char status)
{
	P_DeviceInfo pDeviceInfo = pWorkingTable->pDevice;
	T_U64 end_lba64;

	if (status & 0x80) {
		pDeviceInfo->SenseKey = 0x0B;
		pDeviceInfo->SenseAsc = 0x47;
		pDeviceInfo->SenseAscQ = 0x01;
	} else if (status & 0x40) {
		pDeviceInfo->SenseKey = 0x03;
		pDeviceInfo->SenseAsc = 0x11;
		if (	(pWorkingTable->XferCmd == IDE_READ_DMA) ||
			(pWorkingTable->XferCmd == IDE_READ_DMA) ||
			(pWorkingTable->XferCmd == 0xC4) ||
			(pWorkingTable->XferCmd == 0x29) ||
			(pWorkingTable->XferCmd == IDE_READ_PIO) ||
			(pWorkingTable->XferCmd == IDE_READ_PIO_EXT) ||
			(pWorkingTable->XferCmd == IDE_NCQ_READ)
		   ) {
			pDeviceInfo->SenseAscQ = 0x00;
		} else {
			pDeviceInfo->SenseAscQ = 0x0C;
		}
	} else if (status & 0x20) {
		pDeviceInfo->SenseKey = 0x06;
		pDeviceInfo->SenseAsc = 0x28;
		pDeviceInfo->SenseAscQ = 0x00;
	} else if (status & 0x10) {
		AddU64(&(pWorkingTable->StartLBAOrg),&(pWorkingTable->TransferLenOrg),&end_lba64);
		if (CmpU64(&end_lba64,&(pDeviceInfo->Capacity)) > 0) {
			pDeviceInfo->SenseKey = 0x05;
			pDeviceInfo->SenseAsc = 0x21;
			pDeviceInfo->SenseAscQ = 0x00;
		} else {
			pDeviceInfo->SenseKey = 0x03;
			pDeviceInfo->SenseAsc = 0x11;
			if (	(pWorkingTable->XferCmd == IDE_READ_DMA) ||
				(pWorkingTable->XferCmd == IDE_READ_DMA) ||
				(pWorkingTable->XferCmd == 0xC4) ||
				(pWorkingTable->XferCmd == 0x29) ||
				(pWorkingTable->XferCmd == IDE_READ_PIO) ||
				(pWorkingTable->XferCmd == IDE_READ_PIO_EXT) ||
				(pWorkingTable->XferCmd == IDE_NCQ_READ)
			   ) 		{
				pDeviceInfo->SenseAscQ = 0x00;
			} else {
				pDeviceInfo->SenseAscQ = 0x0c;
			}
		}
	} else if (status & 0x0A) {
		pDeviceInfo->SenseKey = 0x02;
		pDeviceInfo->SenseAsc = 0x3a;
		pDeviceInfo->SenseAscQ = 0x00;
	} else if (status & 0x04) {
		// internal target failure
		pDeviceInfo->SenseKey = 0x0b;
		pDeviceInfo->SenseAsc = 0x44;
		pDeviceInfo->SenseAscQ = 0x00;
	} else {
		pDeviceInfo->SenseKey = 0x0b;
		pDeviceInfo->SenseAsc = 0x00;
		pDeviceInfo->SenseAscQ = 0x00;
	}
	return;
}
//=======================================================================================
static void AddU64(P_U64 pSrc64_1,P_U64 pSrc64_2,P_U64 pDest64)
{
	 (pDest64->U64L).U64L_L = (pSrc64_1->U64L).U64L_L + (pSrc64_2->U64L).U64L_L;
	if (	  ((pDest64->U64L).U64L_L < (pSrc64_1->U64L).U64L_L)
		&&((pDest64->U64L).U64L_L < (pSrc64_2->U64L).U64L_L)
	   ) {
		 (pDest64->U64L).U64L_H = (pSrc64_1->U64L).U64L_H + (pSrc64_2->U64L).U64L_H + 1;
	} else {
		 (pDest64->U64L).U64L_H = (pSrc64_1->U64L).U64L_H + (pSrc64_2->U64L).U64L_H;
	}
	return;
}
//=======================================================================================
/*
static void SubU64(P_U64 pSrc64_1,P_U64 pSrc64_2,P_U64 pDest64)
{
	 (pDest64->U64L).U64L_L = (pSrc64_1->U64L).U64L_L - (pSrc64_2->U64L).U64L_L;
	if ((pDest64->U64L).U64L_L > (pSrc64_1->U64L).U64L_L) {
		 (pDest64->U64L).U64L_H = (pSrc64_1->U64L).U64L_H - (pSrc64_2->U64L).U64L_H - 1;
	} else {
		 (pDest64->U64L).U64L_H = (pSrc64_1->U64L).U64L_H - (pSrc64_2->U64L).U64L_H;
	}
	return;
}
*/
//=======================================================================================
static int CmpU64(P_U64 pSrc64_1,P_U64 pSrc64_2)
{
	if (((pSrc64_1->U64L).U64L_H) == ((pSrc64_2->U64L).U64L_H)) {
		if (((pSrc64_1->U64L).U64L_L) == ((pSrc64_2->U64L).U64L_L)) {
			return 0;
		} else if (((pSrc64_1->U64L).U64L_L) > ((pSrc64_2->U64L).U64L_L)) {
			return 1;
		} else {
			return -1;
		}

	} else if (((pSrc64_1->U64L).U64L_H) > ((pSrc64_2->U64L).U64L_H)) {
		return 1;
	} else {
		return -1;
	}
}
//=======================================================================================
static void mDelay(int interval)
{
	int i;
	
	for (i = 0 ; i < interval ; i++) {
		udelay(1000);
		cpu_relax();
	}
	return;
}
//=======================================================================================
static void Debug80Port(unsigned char code,int delaytime)
{
#if DEBUG_80_PORT
	outb(code,0x80);
	udelay(delaytime);
#endif
	return;
}
//=======================================================================================
//=======================================================================================
#include <linux/ide.h>

static int __devinit atp8620_init_one(struct pci_dev *dev)
{
	unsigned long base , ctl;
    
	base = dev->resource[0].start+0x80;
	ctl = dev->resource[0].start+0x8e;
	hw_regs_t hw, *hws[] = { &hw, NULL, NULL, NULL };


	memset(&hw, 0, sizeof(hw));
	ide_std_init_ports(&hw, base, ctl);
	hw.irq = dev->irq;
	hw.chipset = ide_pci;

	return ide_host_add(NULL, hws, NULL);
}


static int __init atp8620_setup(char *options)
{
    if (!options || !*options)
        return 0;
    if(options[0]=='0')oldide=0;
    else oldide=1;
    return 1;
}

__setup("oldide=", atp8620_setup);

