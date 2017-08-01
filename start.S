#define pWTCON		0x53000000
#define CLKDIVN 	0x4C000014
#define MEM_CTL_BASE    0x48000000
#define S3C2440_MPLL_400MHZ     ((0x5c<<12)|(0x01<<4)|(0x01))
//#define MYUBOOT
.text
.global _start
_start:
/*step 1:disable the wwdog*/
	ldr r0, =pWTCON          
	mov r1, #0x0
	str r1, [r0]


/*step 2:initial the clk*/
	ldr r0,=CLKDIVN
	mov r1, #0x03;            // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1
	str r1, [r0]
	
	/*这里是数据手册上要求的: 如果HDIVN非0，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode” */
	mrc	p15, 0, r1, c1, c0, 0		/* 读出控制寄存器 */ 
	orr	r1, r1, #0xc0000000			/* 设置为“asynchronous bus mode” */
	mcr	p15, 0, r1, c1, c0, 0		/* 写入控制寄存器 */
	
	/* MPLLCON = S3C2440_MPLL_200MHZ */
	ldr r0, =0x4c000004
	ldr r1, =S3C2440_MPLL_400MHZ
	str r1, [r0]
	
	
/*step 3:initial sdram*/
	ldr r0, =MEM_CTL_BASE
	adr r1, sdram_config
	add r2, r0, #(4*13)
1:
	ldr r3, [r1], #0x04
	str r3, [r0], #0x04
	cmp r0, r2
	bne 1b
	
	
/*step 4:relocate code*/
    /*stack set*/
	ldr sp, =0x34000000 /*我们让栈指向内存最高地址,栈是向下增长的,我们的内存是64M,基地址是0x30000000*/
	bl init_flash
	mov r0, #0x0
	ldr r1, _start
	ldr r2, __bss_start
	sub r2,r2,r1
	bl copy_to_sdram
	bl clear_bss
	
	
	
	
	/*step 5:execute c function*/
	ldr lr, =halt   /*这里不能用bl main ,因为bl是相对跳转,我们已经完成了重定位,那么肯定不能跳转到sdram里面去执行main*/
	ldr pc, =main   /*bl 跳转指令只能在+/- 32MB内跳转,所以我们要用位置有关码来实现*/
halt:
	b halt	
	        
	
	
	
	
sdram_config:
	.long 0x22011110	 //BWSCON
	.long 0x00000700	 //BANKCON0
	.long 0x00000700	 //BANKCON1
	.long 0x00000700	 //BANKCON2
	.long 0x00000700	 //BANKCON3  
	.long 0x00000700	 //BANKCON4
	.long 0x00000700	 //BANKCON5
	.long 0x00018005	 //BANKCON6
	.long 0x00018005	 //BANKCON7
	.long 0x008C04F4	 //REFRESH
	.long 0x000000B1	 //BANKSIZE
	.long 0x00000030	 //MRSRB6
	.long 0x00000030	 //MRSRB7