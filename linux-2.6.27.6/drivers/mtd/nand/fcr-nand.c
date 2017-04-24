/*
 *  drivers/mtd/nand/fcr_soc.c
 */

//#includ <asm/mach-types.h>
//#include <machine/types.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include "fcr-nand.h"
/*
 * MTD structure for fcr_soc board
 */
static struct mtd_info *fcr_soc_mtd = NULL;
/*
 * Define partitions for flash device
 */
static const struct mtd_partition partition_info[] = {
/*	{
	 .name = "boot",
	 .offset = 0x0,
	 .size = 1 * 1024 * 1024}
	,*/{
	 .name = "kernel",
	 //.offset = 1*1024*1024,
	 .offset = 0x0,
	 .size = 32 * 1024 * 1024}
	,{
	 .name = "ramdisk",
	 .offset = 32*1024*1024,
	 .size = 32 * 1024 * 1024}
#if 0
	,
		{
	 .name = "jffs2",
	 .offset = 64*1024*1024,
	 .size = 32 * 1024 * 1024}
	,{
	 .name = "yaffs2",
	 .offset = 96*1024*1024,
	 .size = 32 * 1024 * 1024}
#endif
	,{
	 .name = "test",
	 .offset = 64*1024*1024,
	 .size = 420 * 1024 * 1024}
	 //.size = 900 * 1024 * 1024}
};

#define NUM_PARTITIONS 3

static void fcr_soc_hwcontrol(struct mtd_info *mtd, int dat,unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;

//printk("--------%s--------------\n",__func__);
if((ctrl & NAND_CTRL_ALE)==NAND_CTRL_ALE)
		*(volatile unsigned char *)(0xffffffffbe000082) = dat;
if ((ctrl & NAND_CTRL_CLE)==NAND_CTRL_CLE)
		*(volatile unsigned char *)(0xffffffffbe000081) = dat;
}


//Main initialization for yaffs ----- by zhly
int fcr_soc_foryaffs_init(struct mtd_info *mtd)
{
	struct nand_chip *this;
	
	if(!mtd)
	{
		if(!(mtd = kmalloc(sizeof(struct mtd_info),GFP_KERNEL)))
		{
			printk("unable to allocate mtd_info structure!\n");
			return -ENOMEM;
		}
		memset(mtd, 0, sizeof(struct mtd_info));
	}

	this = kmalloc(sizeof(struct nand_chip),GFP_KERNEL);
	if(!this)
	{
		printk("Unable to allocate nand_chip structure!\n");
		return -ENOMEM;
	}
	memset(this,0,sizeof(struct nand_chip));
		
	fcr_soc_mtd=mtd;
	fcr_soc_mtd->priv = this;

	this->IO_ADDR_R = (void *)0xffffffffbf000040;
	this->IO_ADDR_W = (void *)0xffffffffbf000040;
	this->cmd_ctrl = fcr_soc_hwcontrol;
	this->ecc.mode = NAND_ECC_SOFT;
	
	if(nand_scan(fcr_soc_mtd,1))
	{
		kfree(fcr_soc_mtd);
		printk("nand_scan failed!\n");
		return -1;		
	}
	fcr_soc_mtd->size=0x04000000;

	return 0;
}

/*
 * Main initialization routine
 */
int fcr_soc_nand_init(void)
{
	struct nand_chip *this;

	/* Allocate memory for MTD device structure and private data */
//printk("--------%s--------------\n",__func__);
	fcr_soc_mtd = kmalloc(sizeof(struct mtd_info) + sizeof(struct nand_chip), GFP_KERNEL);
	if (!fcr_soc_mtd) {
		printk("Unable to allocate fcr_soc NAND MTD device structure.\n");
		return -ENOMEM;
	}

	/* Get pointer to private data */
	this = (struct nand_chip *)(&fcr_soc_mtd[1]);

	/* Initialize structures */
	memset(fcr_soc_mtd, 0, sizeof(struct mtd_info));
	memset(this, 0, sizeof(struct nand_chip));

	/* Link the private data with the MTD structure */
	fcr_soc_mtd->priv = this;


	/* Set address of NAND IO lines */
	this->IO_ADDR_R = (void  *)0xffffffffbe000080;
	this->IO_ADDR_W = (void  *)0xffffffffbe000080;
	/* Set address of hardware control function */
	this->cmd_ctrl = fcr_soc_hwcontrol;
	/* 15 us command delay time */
	this->chip_delay = 35;  //mj
	this->ecc.mode = NAND_ECC_SOFT;
	//this->ecc.mode = NAND_ECC_NONE;

//	this->ecc.layout = 32;

	/* Scan to find existence of the device */
	if (nand_scan(fcr_soc_mtd, 1)) {
		kfree(fcr_soc_mtd);
		return -ENXIO;
	}

	/* Register the partitions */
	add_mtd_partitions(fcr_soc_mtd, partition_info, NUM_PARTITIONS);
//	add_mtd_device(fcr_soc_mtd,0,0,"total flash");
//	add_mtd_device(fcr_soc_mtd,0,0x02000000,"kernel");
//	add_mtd_device(fcr_soc_mtd,0x02000000,0x04000000,"squashfs");
//	add_mtd_device(fcr_soc_mtd,0x02000000+0x04000000,0x04000000,"jffs2");
//	add_mtd_device(fcr_soc_mtd,0x02000000+0x04000000+0x04000000,0x04000000,"yaffs2");
//	add_mtd_device(fcr_soc_mtd,0x02000000+0x04000000+0x04000000+0x04000000,0x02000000,"cramfs");

	/* Return happy */
	return 0;
}
module_init(fcr_soc_nand_init);


