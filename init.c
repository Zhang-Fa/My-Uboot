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

void copy_to_sdram(unsigned char *src,unsigned char *dst,unsigned int len)
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

void nand_init(void)
{
}



