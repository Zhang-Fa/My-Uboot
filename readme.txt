自己写bootloader:
一:uboot需要哪些基本功能:
通过分析bootloader的作用来知道我们在写bootloader的时候应该要写什么?
bootloader的作用:目标是启动内核
细细分析这句话:
uboot要完成这个操作要做两步:
1:把内核从flash上复制到内存
  1.1:内核在哪里?内核在flash上
  1.2:bootloader需具备有读内核的能力,而内核存在与flash上,那么就必须具备读nandflash或者norflash的能力
      若为nandflash:需要一些命令等等
      若为norflash:norflash的读就和读内存一样
      要读到内存SDRAM中,就要先初始化sdram
  1.3:我们的2440一上电的时候的时钟是12M,我们想让他运行的更快,我们要初始化时钟,还有关闭看门狗等等硬件操作
2:跳转执行内核
  2.1:跳转执行之前uboot需要给内核传递一些参数
      我们就需要在特定位置设置参数
  2.2:跳转到内核去执行

总结:
最简单的bootloader的编写步骤(这里是内核在nandflash的情况)：
1. 初始化硬件：关看门狗(对于2440来说看门狗一上电是打开的)、设置时钟、设置SDRAM、初始化NAND FLASH
2. 如果bootloader比较大，要把它重定位到SDRAM
3. 把内核从NAND FLASH读到SDRAM
4. 设置"要传给内核的参数"
5. 跳转执行内核
改进：
1. 提高CPU频率, 200MHZ ==> 400MHZ
2. 启动ICACHE


内核在flash上
我们就要从flash把内核读到

NOR flash:就像读和内存一样
NAND flash:
初始化内存SDRAM
Q1:c语言清除BSS段中的语法:
void clear_bss(void)
{
	extern int __bss_start, __bss_end;
	int *p = &__bss_start;
	
	for (; p < &__bss_end; p++)
		*p = 0;
}

Q1.1:如何调用链接脚本中的变量:__bss_end和__bss_start
A1.1:extern int __bss_start, __bss_end;

Q1.2:链接脚本中的__bss_end这个标记的地址代表什么意思,值又是什么意思,为什么要 int *p = &__bss_start;而不是int *p = __bss_start;
A1.2:百度

Q1:内核为何要被复制到内存上去执行?


Q2:start.s中的bne 1b是什么意思
1:
	ldr r3, [r1], #0x04
	str r3, [r0], #0x04
	cmp r0, r2
	bne 1b

这里的如果不想等就跳回后面的1标号 就是1b,如果想跳到前面的1标号,就是1f(要注意这里的前面和后面的方向)


Q3:我们在重定位完代码之后想跳转到main函数去执行,我们用的是:ldr pc, =main,为什么不能用bl main呢
A:BL main 为相对跳转指令，与位置无关代码。
  LDR PC,=main 为绝对跳转指令，与位置有关代码。
  相对跳转指令跳转范围+/- 32MB,但是由main在SDRAM中，而从片内4K SRAM调到SDRAM显然超过了+/- 32M  

Q4:我们在重定位代码的时候要区分是NOR启动还是NAND启动
   而我们的内核


注意事项:
1:在判断是NOR或者NAND启动的时候代码,我们是通过给0地址写个值,然后读取看是否改变来判断的,要注意的是,写完之后要还原本来的值
2:在进行代码重定位的C代码时候
void copy_code_to_sdram(unsigned char *src, unsigned char *dest, unsigned int len)
{	
	int i = 0;
	
	/* 如果是NOR启动 */
	if (isBootFromNorFlash())
	{
		while (i < len)
		{
			dest[i] = src[i];
			i++;
		}
	}
	else
	{
		//nand_init();
		nand_read((unsigned int)src, dest, len);
	}
}
之所以我们这里屏蔽了nand_init();是因为即使是NOR启动的话,最终的内核也是被下载到了NAND flash,而uboot要读内核的话,最终还是要读nandflash,
所以我们的nand_init函数不管是NOR启动还是NAND启动都是要做的,我们放在了start.S的一起做了