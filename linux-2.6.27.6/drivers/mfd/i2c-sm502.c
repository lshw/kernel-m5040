#include <linux/io.h>
#include <asm/types.h>
#include <linux/init.h>
#include <linux/types.h>
#include <asm/serial.h>		/* For the serial port location and base baud */
#include <linux/pci.h>


#define DEVBD2F_SM502 
extern struct pci_ops * loongson2f_pci_pci_ops;

static unsigned long read_mmio(int renew)
{
	struct pci_bus bus;
	struct pci_dev dev;
	u32 tmp;
	unsigned long mmio;
	static unsigned long maped_mmio=0;

	bus.ops=&loongson2f_pci_pci_ops;
	dev.bus=&bus;
	bus.number=0x0;
	dev.devfn=0x70;
	pci_read_config_dword(&dev,0x14,&tmp);
	mmio=(tmp&~0xf);
	if(mmio<0x20000000){
	if(mmio<0x10000000)mmio |=0x10000000;
	//return (mmio+0x30000)|0xffffffffa0000000;
	return (mmio)|0xffffffffa0000000;
	}
	//else if(!maped_mmio) maped_mmio = ioremap_nocache(mmio,1*1024*1024)+0x30000;
	else if(!maped_mmio) maped_mmio = ioremap_nocache(mmio,1*1024*1024);

	return maped_mmio;
}

#define I2C_SINGLE 0
#define I2C_BLOCK 1
#define I2C_SMB_BLOCK 2


#define GPIO_DIR_REG 		(volatile unsigned int *)(read_mmio(0)+ 0x1000c)
#define GPIO_DATA_REG		(volatile unsigned int *)(read_mmio(0) + 0x10004)
#define G_OUTPUT			1
#define G_INPUT				0
#define GPIO_SDA_DIR_SHIFT	15
#define	GPIO_SCL_DIR_SHIFT	14
#define GPIO_SDA_DATA_SHIFT	15
#define GPIO_SCL_DATA_SHIFT	14


static void i2c_sleep(int ntime)
{
	int i,j = 0;
	for(i = 0; i < 300 * ntime; i++)
	{
		j = i;
		j += i;
	}
}

void sda_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_REG;
	if(ivalue == 1)
		*GPIO_DIR_REG = tmp | (0x1 << GPIO_SDA_DIR_SHIFT);
	else
		*GPIO_DIR_REG = tmp & (~(0x1 << GPIO_SDA_DIR_SHIFT));
}
void scl_dir(int ivalue)
{
	int tmp;
	tmp = *GPIO_DIR_REG;
	if(ivalue == 1)
		*GPIO_DIR_REG = tmp | (0x1 << GPIO_SCL_DIR_SHIFT);
	else
		*GPIO_DIR_REG = tmp & (~(0x1 << GPIO_SCL_DIR_SHIFT));
}

void sda_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_REG;
	if(ivalue == 1)
		*GPIO_DATA_REG = tmp | (0x1 << GPIO_SDA_DATA_SHIFT);
	else
		*GPIO_DATA_REG = tmp & (~(0x1 << GPIO_SDA_DATA_SHIFT));
}

void scl_bit(int ivalue)
{
	int tmp;
	tmp = *GPIO_DATA_REG;
	if(ivalue == 1)
		*GPIO_DATA_REG = tmp | (0x1 << GPIO_SCL_DATA_SHIFT);
	else
		*GPIO_DATA_REG = tmp & (~(0x1 << GPIO_SCL_DATA_SHIFT));
}

static void i2c_start(void)
{
	sda_dir(G_OUTPUT);
	scl_dir(G_OUTPUT);
	scl_bit(0);
	i2c_sleep(1);
	sda_bit(1);
	i2c_sleep(1);
	scl_bit(1);
	i2c_sleep(5);
	sda_bit(0);
	i2c_sleep(5);
	scl_bit(0);
	i2c_sleep(2);
}
static void i2c_stop(void)
{
	sda_dir(G_OUTPUT);
	scl_dir(G_OUTPUT);
	scl_bit(0);
	i2c_sleep(1);
	sda_bit(0);
	i2c_sleep(1);
	scl_bit(1);
	i2c_sleep(5);
	sda_bit(1);
	i2c_sleep(5);
	scl_bit(0);
	i2c_sleep(2);
}

static void i2c_send_ack(int ack)
{
	sda_dir(G_OUTPUT);
	sda_bit(ack);
	i2c_sleep(3);
	scl_bit(1);
	i2c_sleep(5);
	scl_bit(0);
	i2c_sleep(2);
}

static char i2c_rec_ack()
{
    char res = 1;
    int num = 10;
	int tmp;
    sda_dir(G_INPUT);
    i2c_sleep(3);
    scl_bit(1);
    i2c_sleep(5);
	tmp = ((*GPIO_DATA_REG) & (0x1 << GPIO_SDA_DATA_SHIFT));
        //wait for a ack signal from slave

     while(tmp)
     {
		i2c_sleep(1);
        num--;
        if(!num)
        {
			res = 0;
            break;
        }
		tmp = ((*GPIO_DATA_REG) & (0x1 << GPIO_SDA_DATA_SHIFT));
        }
    scl_bit(0);
    i2c_sleep(3);
    return res;
}

static unsigned char i2c_rec()
{
	int i;
	int tmp;
	unsigned char or_char;
	unsigned char value = 0x00;
	sda_dir(G_INPUT);
	for(i = 7;i >= 0;i--)
	{
		i2c_sleep(5);
		scl_bit(1);
		i2c_sleep(3);
		tmp = ((*GPIO_DATA_REG) & (0x1 << GPIO_SDA_DATA_SHIFT));
		if(tmp)
			or_char = 0x1;
		else
			or_char = 0x0;
		or_char <<= i;
		value |= or_char;
		i2c_sleep(3);
		scl_bit(0);
	}
	return value;
}

static unsigned char i2c_send(unsigned char value)
{
	//we assume that now scl is 0
	int i;
	unsigned char and_char;
	sda_dir(G_OUTPUT);
	for(i = 7;i >= 0;i--)
	{
		and_char = value;
		and_char >>= i;
		and_char &= 0x1;
		if(and_char)
			sda_bit(1);
		else
			sda_bit(0);
		i2c_sleep(1);
		scl_bit(1);
		i2c_sleep(5);
		scl_bit(0);
		i2c_sleep(1);
	}
	sda_bit(1);	
	return 1;
}

unsigned char i2c_rec_s(unsigned char *addr,int addrlen,unsigned char reg,unsigned char* buf ,int count)
{
	int i;
	int j;
	unsigned char value;
	//start signal
	for(i = 0;i < count;i++)
	{
		i2c_start();
		for(j = 0;j < addrlen;j++)
		{
		//write slave_addr
		i2c_send(addr[j]);
		if(!i2c_rec_ack())
			return 0;
		}

		i2c_send(reg);
		if(!i2c_rec_ack())
			return 0;

		//repeat start
		i2c_start();
		//write slave_addr+1
		i2c_send(addr[0] | 0x1);
		if(!i2c_rec_ack())
			return 0;
		//read data
		buf[i] = i2c_rec();	
//		i2c_send_ack(1);//***add in***//
		i2c_stop();
		reg++;
	}
	return count;
}

unsigned char i2c_send_s(unsigned char *addr,int addrlen,unsigned char reg,unsigned char * buf ,int count)
{
	int i;
	int j;
	for(i = 0;i < count;i++)
	{	
		i2c_start();	
		for(j = 0;j < addrlen;j++)
		{
		//write slave_addr
		i2c_send(addr[j]);
		if(!i2c_rec_ack())
			return 0;
		}

		i2c_send(reg);
		if(!i2c_rec_ack())
			return 0;

		i2c_send(buf[i]);
		if(!i2c_rec_ack())
			return 0;
		i2c_stop();
		reg++;
	}
	return 1;
}

unsigned char i2c_rec_b(unsigned char *addr,int addrlen,unsigned char reg,unsigned char* buf ,int count)
{
	int i;
	int j;
	unsigned char value;
	//start signal
	i2c_start();
	for(j = 0;j < addrlen;j++)
	{
		//write slave_addr
		i2c_send(addr[j]);
		if(!i2c_rec_ack())
			return 0;
	}

	i2c_send(reg);
	if(!i2c_rec_ack())
		return 0;

	//repeat start
	i2c_start();
	//write slave_addr+1
	i2c_send(addr[0] | 0x1);
	if(!i2c_rec_ack())
		return 0;

	for(i = 0;i < count;i++)
	{
		//read data
		buf[i] = i2c_rec();	
//		i2c_send_ack(1);//***add in***//
		
	}
	i2c_stop();
	return count;
}
unsigned char i2c_send_b(unsigned char *addr,int addrlen,unsigned char reg,unsigned char * buf ,int count)
{
	int i;
	int j;
	i2c_start();	
	for(j = 0;j < addrlen;j++)
	{
		//write slave_addr
		i2c_send(addr[j]);
		if(!i2c_rec_ack())
			return 0;
	}

	i2c_send(reg);
	if(!i2c_rec_ack())
		return 0;

	for(i = 0;i < count;i++)
	{
		i2c_send(buf[i]);
		if(!i2c_rec_ack())
			return 0;
	}
	i2c_stop();
	return count;
}
//----------------------
/*
 * 0 single:
 * 1 smb block
 */
 static void * my_memset(void * s,int c, size_t count)
	  {
		char *xs = (char *) s;
	
		while (count--)
			*xs++ = c;
	
				return s;
			 }
#if 1
int tgt_i2cinit1()
{
	int tmp;
	unsigned long mmio=read_mmio(0);
    // mmio = sm50x_base_reg;
		tmp = *(volatile long *)(mmio + 0x40);
		*(volatile long *)(mmio + 0x40) = tmp | 0x40;
		return 0;
//		tgt_printf("clock enable bit 40 = %x\n", *(volatile int *)(mmio + 0x40));
}
void prom_printf(char *fmt, ...);
int tgt_i2cinit()
{
#if 1
	static int inited = 0;
	unsigned long tmp;
	unsigned long mmio=read_mmio(0);
    
	if(inited == 0 && (mmio & 0x80000000) )
	{
		//mmio = mmio | (0xb0000000);
		tmp = *(volatile long *)(mmio + 0x40);
		*(volatile long *)(mmio + 0x40) = tmp | 0x40;
//		tgt_printf("clock enable bit 40 = %x\n", *(volatile int *)(mmio + 0x40));
//	prom_printf("------1--inited:%d---\n",inited);
		inited += 1;
	}
	else if (inited == 1 && !(mmio & 0x80000000) )
	{
		//mmio = mmio | (0xb0000000);
		tmp = *(volatile long *)(mmio + 0x40);
		*(volatile long *)(mmio + 0x40) = tmp | 0x40;
//		tgt_printf("clock enable bit 40 = %x\n", *(volatile int *)(mmio + 0x40));
//	prom_printf("------2--inited:%d---\n",inited);
		inited += 1;

	}
#endif
	return 0;
}
#endif
int tgt_i2cread(int type,unsigned char *addr,int addrlen,unsigned char reg,unsigned char *buf,int count)
{
	int i;
	tgt_i2cinit();
//	prom_printf("------1--read_mmio:%08x---\n",read_mmio(0));
	my_memset(buf,-1,count);
//	prom_printf("------2-----\n");
	switch(type)
	{
		case I2C_SINGLE:
			return i2c_rec_s(addr,addrlen,reg,buf,count);
		break;
		case I2C_BLOCK:
			return i2c_rec_b(addr,addrlen,reg,buf,count);
		break;

		default: return 0;break;
	}
	return 0;
}

int tgt_i2cwrite(int type,unsigned char *addr,int addrlen,unsigned char reg,unsigned char *buf,int count)
{
	tgt_i2cinit();
	switch(type & 0xff)
	{
		case I2C_SINGLE:
			i2c_send_s(addr,addrlen,reg,buf,count);
		break;
		case I2C_BLOCK:
			return i2c_send_b(addr,addrlen,reg,buf,count);
		break;
		case I2C_SMB_BLOCK:
		break;
		default:return -1;break;
	}
	return -1;
}

