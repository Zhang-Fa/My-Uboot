#ifndef __NANDFLASH_H__
#define __NANDFLASH_H__

typedef unsigned int S3C24X0_REG32;
/****************************NANDFLASH REGISTER DEFINED**************************/

/* NAND FLASH (see S3C2410 manual chapter 6) */
typedef struct {
    S3C24X0_REG32   NFCONF; //0x4E000000
    S3C24X0_REG32   NFCMD;
    S3C24X0_REG32   NFADDR;
    S3C24X0_REG32   NFDATA;
    S3C24X0_REG32   NFSTAT;
    S3C24X0_REG32   NFECC;
} S3C2410_NAND;

/* NAND FLASH (see S3C2440 manual chapter 6) */
typedef struct {
    S3C24X0_REG32   NFCONF; //0x4E000000
    S3C24X0_REG32   NFCONT;
    S3C24X0_REG32   NFCMD;
    S3C24X0_REG32   NFADDR;
    S3C24X0_REG32   NFDATA;
    S3C24X0_REG32   NFMECCD0;
    S3C24X0_REG32   NFMECCD1;
    S3C24X0_REG32   NFSECCD;
    S3C24X0_REG32   NFSTAT;
    S3C24X0_REG32   NFESTAT0;
    S3C24X0_REG32   NFESTAT1;
    S3C24X0_REG32   NFMECC0;
    S3C24X0_REG32   NFMECC1;
    S3C24X0_REG32   NFSECC;
    S3C24X0_REG32   NFSBLK;
    S3C24X0_REG32   NFEBLK;
} S3C2440_NAND;

#define GSTATUS1        (*(volatile unsigned int *)0x560000B0) /*GSTATUS1 register is Chip ID*/

/****************************NANDFALSH OPERATION FUNCTION********************************/
typedef struct {
    void (*nand_reset)(void);
    void (*wait_idle)(void);
    void (*nand_select_chip)(void);         //nandflash selected
    void (*nand_deselect_chip)(void);       
    void (*write_cmd)(int cmd);             //send cmd
    void (*write_addr)(unsigned int addr);  //send addr
    unsigned char (*read_data)(void);       //read data
}t_nand_chip;

/****************************GLOBAL MICRO DEFINED********************************/
#define LARGER_NAND_PAGE  //our nandflash is largepage(2048 byte/page), small page is 512 byte per page

#define NAND_SECTOR_SIZE    512
#define NAND_BLOCK_MASK     (NAND_SECTOR_SIZE - 1)

#define NAND_SECTOR_SIZE_LP    2048
#define NAND_BLOCK_MASK_LP     (NAND_SECTOR_SIZE_LP - 1)

#define TACLS   0    //set clock of nandflash
#define TWRPH0  3
#define TWRPH1  0

#define BUSY            1
#endif
