#include "nandflash.h"


static S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;
static S3C2440_NAND * s3c2440nand = (S3C2440_NAND *)0x4e000000;

static t_nand_chip nand_chip;

/* 供外部调用的函数 */
void nand_init(void);
void nand_read(unsigned char *buf, unsigned long start_addr, int size);

/* NAND Flash操作的总入口, 它们将调用S3C2410或S3C2440的相应函数 */
static void nand_reset(void);
static void wait_idle(void);
static void nand_select_chip(void);
static void nand_deselect_chip(void);
static void write_cmd(int cmd);
static void write_addr(unsigned int addr);
static unsigned char read_data(void);

/* S3C2440的NAND Flash处理函数 */
static void s3c2440_nand_reset(void);
static void s3c2440_wait_idle(void);
static void s3c2440_nand_select_chip(void);
static void s3c2440_nand_deselect_chip(void);
static void s3c2440_write_cmd(int cmd);
static void s3c2440_write_addr_lp(unsigned int addr);
static unsigned char s3c2440_read_data(void);

/* 复位 */
static void s3c2440_nand_reset(void)
{
    s3c2440_nand_select_chip();
    s3c2440_write_cmd(0xff);  // 复位命令
    s3c2440_wait_idle();
    s3c2440_nand_deselect_chip();
}

/* 等待NAND Flash就绪 */
static void s3c2440_wait_idle(void)
{
    int i;
    volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFSTAT;
    while(!(*p & BUSY))
        for(i=0; i<10; i++);
}

/* 发出片选信号 */
static void s3c2440_nand_select_chip(void)
{
    int i;
    s3c2440nand->NFCONT &= ~(1<<1);
    for(i=0; i<10; i++);    
}

/* 取消片选信号 */
static void s3c2440_nand_deselect_chip(void)
{
    s3c2440nand->NFCONT |= (1<<1);
}

/* 发出命令 */
static void s3c2440_write_cmd(int cmd)
{
	*((volatile unsigned char*)&s3c2440nand->NFCMD) = cmd;
}

/* 发出地址 */
static void s3c2440_write_addr_lp(unsigned int addr)
{
	int i;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;
	int col, page;

	col = addr & NAND_BLOCK_MASK_LP;
	page = addr / NAND_SECTOR_SIZE_LP;
	
	*p = col % 0xff;			/* Column Address A0~A7  在某页中的序号*/
	for(i=0; i<10; i++);		
	*p = (col >> 8) & 0x0f; 	/* Column Address A8~A11 页序号*/
	for(i=0; i<10; i++);
	*p = page & 0xff;			/* Row Address A12~A19 */
	for(i=0; i<10; i++);
	*p = (page >> 8) & 0xff;	/* Row Address A20~A27 */
	for(i=0; i<10; i++);
	*p = (page >> 16) & 0x03;	/* Row Address A28~A29 */
	for(i=0; i<10; i++);
}

/* 读取数据 */
static unsigned char s3c2440_read_data(void)
{
	return *((volatile unsigned char*)&(s3c2410nand->NFDATA));	
}

/* 在第一次使用NAND Flash前，复位一下NAND Flash */
static void nand_reset(void)
{
    nand_chip.nand_reset();
}

static void wait_idle(void)
{
    nand_chip.wait_idle();
}

static void nand_select_chip(void)
{
    int i;
    nand_chip.nand_select_chip();
    for(i=0; i<10; i++);
}

static void nand_deselect_chip(void)
{
    nand_chip.nand_deselect_chip();
}

static void write_cmd(int cmd)
{
    nand_chip.write_cmd(cmd);
}
static void write_addr(unsigned int addr)
{
    nand_chip.write_addr(addr);
}

static unsigned char read_data(void)
{
    return nand_chip.read_data();
}

/*NandFlash initial*/
void nand_init(void)
{
	if(GSTATUS1 == 0x32410001){      /*the chip id of s3c2440 is 0x32410001*/
		nand_chip.nand_reset 		 = s3c2440_nand_reset;   /*initial nandflash operation functions*/
		nand_chip.wait_idle          = s3c2440_wait_idle;
        nand_chip.nand_select_chip   = s3c2440_nand_select_chip;
        nand_chip.nand_deselect_chip = s3c2440_nand_deselect_chip;
        nand_chip.write_cmd          = s3c2440_write_cmd;
#ifdef LARGER_NAND_PAGE
        nand_chip.write_addr         = s3c2440_write_addr_lp;
#else
		nand_chip.write_addr		 = s3c2440_write_addr;
#endif
		nand_chip.read_data          = s3c2440_read_data;
		}	
        s3c2440nand->NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);	/*set clock*/
	    nand_reset();                                          /*reset nandflash*/		
}

/* 读函数 */
void nand_read(unsigned char *buf, unsigned long start_addr, int size)
{
    int i, j;

#ifdef LARGER_NAND_PAGE
    if ((start_addr & NAND_BLOCK_MASK_LP) || (size & NAND_BLOCK_MASK_LP)) {
        return ;    /* 地址或长度不对齐 */
    }
#else
    if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK)) {
        return ;    /* 地址或长度不对齐 */
    }
#endif	

    /* 选中芯片 */
    nand_select_chip();

    for(i=start_addr; i < (start_addr + size);) {
      /* 发出READ0命令 */
      write_cmd(0);

      /* Write Address */
      write_addr(i);
#ifdef LARGER_NAND_PAGE
      write_cmd(0x30);		
#endif
      wait_idle();

#ifdef LARGER_NAND_PAGE
      for(j=0; j < NAND_SECTOR_SIZE_LP; j++, i++) {
#else
	  for(j=0; j < NAND_SECTOR_SIZE; j++, i++) {
#endif
          *buf = read_data();
          buf++;
      }
    }

    /* 取消片选信号 */
    nand_deselect_chip();
    
    return ;
}


int Nor_or_Nand_Startup(void)
{
	volatile int *p=0;
	int val=0;
	val = *p;
	*p = 0x12345678;
	if(*p == 0x12345678){
		/* NAND startup*/
		*p = val;
		return 0;
	}else{
		/* NOR startup*/
		return 1;
	}
}

void copy_code_to_sdram(unsigned char *src,unsigned char *dst,unsigned int len)
{
	unsigned int i=0;
	/*judge the mode of startup :NOR or NAND*/
	if(Nor_or_Nand_Startup() == 1){
		/*NOR startup*/
		while(i < len){
			i++;
			dst[i] = src[i];
		}
	}else{
		/*NAND startup*/
		
	}
}

void clear_bss(void)
{
	extern int __bss_start, __bss_end;
	int *p = &__bss_start;
	
	for (; p < &__bss_end; p++)
		*p = 0;
}

