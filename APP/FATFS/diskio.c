/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_WQ256		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (								//读取SD卡状态用的函数，此处可不用管
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
	case DEV_WQ256 :
		
	if(WQ256_Flag)	return RES_OK;		//若WQ256处于已初始化状态，回复OK
		else	return STA_NOINIT;

	case DEV_MMC :
		return STA_NOINIT;

	case DEV_USB :
		return STA_NOINIT;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	int result;

	switch (pdrv) {
	case DEV_WQ256 :	
		result = W25QXX_Init(FF_Num);							//需填入一个初始化参数，暂时填一个6
		if(result==1)	return RES_OK;							//WQ256初始化成功
		else	return STA_NOINIT;								//初始化失败
	case DEV_MMC :
		return STA_NODISK;

	case DEV_USB :
		return STA_NODISK;
	default :
		return STA_NODISK;
	}
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;
	
	if(!count) return RES_PARERR; 									// count不能等于0，否则返回参数错误 
	switch (pdrv) {													//选择设备
	case DEV_WQ256 :
		result = W25QXX_Read_By_Sector(buff,sector,count);
		if(result==1)	res=RES_OK;
		else	res=RES_ERROR;
		return res;

	case DEV_MMC :
		res=RES_PARERR;
		return res;

	case DEV_USB :
		res=RES_PARERR;
		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_WQ256 :
		// translate the arguments here

		result = W25QXX_Write_By_Sector((INT8U*)&buff[0], sector, count);
		
		if(result==1)	
		{
			OSTimeDly(1);	//注意事项：在写完之后，有一个延时20ms的动作。这是因为：FatFs写页表和目录表时，如果写完SPIFlash不加延时，就会写入不成功。这导致的后果是：对页表的修改无法生效。也就是说，对已有文件的数据修改（不改变文件大小）会生效，但新增文件、删除文件不会生效。
			res=RES_OK;
		}
		else			res=RES_ERROR;
		return res;
	
	case DEV_MMC :
		res=RES_PARERR;
		return res;
	
	case DEV_USB :
		res=RES_PARERR;
		return res;
	}

	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DWORD *pdword = NULL;
    WORD  *pword = NULL;
	
	switch(cmd) {
	 case CTRL_SYNC:										//确保写入操作已完成
				return RES_OK;

	case GET_SECTOR_COUNT:									//获取扇区数量
				pdword = (DWORD *)buff;
				*pdword = Sector_Max + 1;
				return RES_OK;

	case GET_SECTOR_SIZE:									//获取单个扇区大小
				pword = (WORD *)buff;
				*pword = Sector_Size;
				return RES_OK;
	
	 case GET_BLOCK_SIZE:									//获取擦除块大小（以扇区为单位）
				pdword = (DWORD *)buff;
				*pdword = 1;
				return RES_OK;
     
     case CTRL_TRIM:										//强制擦除
				return RES_PARERR;
	
	
	}

	return RES_PARERR;
}

/*******************************************************************************
添加文件时间戳功能，创建/修改文件时记录操作时间：
*当FF_FS_NORTC设为0：开启文件时间戳功能
*返回值是32位无符号数：31-25位为当前年份与1980的差值；
					  24-21位为月；20-16为日；
					  15-11为时；
					  10-5为分；
					  4-0为秒除以2
*******************************************************************************/
DWORD get_fattime (void)
{
	struct NW_TIME		time = {0};											//年月日时分秒（年为年份-2000）
	DWORD				fat_time = 0;
	
	NW_GetTime(&time);
	fat_time += time.year << 25;
	fat_time += time.mon << 21;
	fat_time += time.mday << 16;
	fat_time += time.hour << 11;
	fat_time += time.min << 5;
	fat_time += time.sec / 2;
	return fat_time;
}








/************************************TTTTTTTTTTTTTTTTTTTTTTTTTTTEEEEEEEEEEEEEEEEEEEEESSSSSSSSSSSSSSSSSSSSSSSSTTTTTTTTTTTTTTTTTTTT***********************/
#if 0
static
DWORD pn (		/* Pseudo random number generator */
    DWORD pns	/* 0:Initialize, !0:Read */
)
{
    static DWORD lfsr;
    UINT n;


    if (pns) {
        lfsr = pns;
        for (n = 0; n < 32; n++) pn(0);
    }
    if (lfsr & 1) {
        lfsr >>= 1;
        lfsr ^= 0x80200003;
    } else {
        lfsr >>= 1;
    }
    return lfsr;
}


int test_diskio (
    BYTE pdrv,      /* Physical drive number to be checked (all data on the drive will be lost) */
    UINT ncyc,      /* Number of test cycles */
    INT32U* buff,    /* Pointer to the working buffer */
    UINT sz_buff    /* Size of the working buffer in unit of byte */
)
{
    UINT n, cc, ns;
    DWORD sz_drv, lba, lba2, sz_eblk, pns = 1;
    WORD sz_sect;
    BYTE *pbuff = (BYTE*)buff;
    DSTATUS ds;
    DRESULT dr;
	INT8U Temp[4]={0};


   BspUartWrite(2,(INT8U *)"test_diskio\r\n",sizeof("test_diskio\r\n")-1);

    if (sz_buff < 4) {
        BspUartWrite(2,(INT8U *)"Insufficient work area to run program.\r\n",sizeof("Insufficient work area to run program.\r\n")-1);
        return 1;
    }

    for (cc = 1; cc <= ncyc; cc++) {
		if(cc==1) BspUartWrite(2,SIZE_OF("***TEST First Round***\r\n"));
		if(cc==2) BspUartWrite(2,SIZE_OF("***TEST Sencond Round***\r\n"));
		if(cc==3) BspUartWrite(2,SIZE_OF("***TEST Third Round***\r\n"));
		
		
       BspUartWrite(2,SIZE_OF(" disk_initalize"));
        ds = disk_initialize(pdrv);
        if (ds & STA_NOINIT) {  
			 BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 2;
        } else {
			 BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        }

		 BspUartWrite(2,SIZE_OF("**** Get drive size ****\r\n"));
        sz_drv = 0;
        dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_drv);
        if (dr == RES_OK) {
           BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
            BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 3;
        }
        if (sz_drv < 128) {
			 BspUartWrite(2,SIZE_OF("Failed: Insufficient drive size to test.\r\n"));
            return 4;
        }
		Temp[0]=sz_drv/1000+0x30;
		Temp[1]=sz_drv%1000/100+0x30;
		Temp[2]=sz_drv%100/10+0x30;
		Temp[3]=sz_drv%10+0x30;
		BspUartWrite(2,SIZE_OF("Number of sectors on the drive is :"));
		BspUartWrite(2,Temp,4);
		BspUartWrite(2,SIZE_OF("\r\n"));

        sz_sect = FF_MAX_SS;

		BspUartWrite(2,SIZE_OF("**** Get block size ****\r\n"));
        sz_eblk = 0;
        dr = disk_ioctl(pdrv, GET_BLOCK_SIZE, &sz_eblk);
        if (dr == RES_OK) {
           BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
            BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
        }
        if (dr == RES_OK || sz_eblk >= 2) {
			Temp[0]=sz_eblk/1000+0x30;
			Temp[1]=sz_eblk%1000/100+0x30;
			Temp[2]=sz_eblk%100/10+0x30;
			Temp[3]=sz_eblk%10+0x30;
			BspUartWrite(2,SIZE_OF(" Size of the erase block is :"));
			BspUartWrite(2,Temp,4);
			BspUartWrite(2,SIZE_OF("\r\n"));
        } else {

			 BspUartWrite(2,SIZE_OF(" Size of the erase block is unknown.\r\n"));
        }

		BspUartWrite(2,SIZE_OF("**** Single sector write test 1 ****\r\n"));
        lba = 0;
		pns=1;
        for (n = 0, pn(pns); n < sz_sect; n++) pbuff[n] = (BYTE)pn(0);
		BspUartWrite(2,SIZE_OF("disk_write\r\n"));
        dr = disk_write(pdrv, pbuff, lba, 1);
        if (dr == RES_OK) {
            BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
            BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 6;
        }
		BspUartWrite(2,SIZE_OF(" disk_ioctl\r\n"));
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
        if (dr == RES_OK) {
            BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
            BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 7;
        }
        memset(pbuff, 0, sz_sect);
		BspUartWrite(2,SIZE_OF("disk_read\r\n"));
        dr = disk_read(pdrv, pbuff, lba, 1);
        if (dr == RES_OK) {
           BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
             BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 8;
        }
		pns=1;
		for (n = 0, pn(pns); n < sz_sect; n++) buff1[n] = (BYTE)pn(0);
		pns=1;
        for (n = 0, pn(pns); n < sz_sect && pbuff[n] == (BYTE)pn(0); n++) ;
        if (n == sz_sect) {

			 BspUartWrite(2,SIZE_OF("Data matched.\r\n"));
        } else {
			  BspUartWrite(2,SIZE_OF("Failed: Read data differs from the data written.\r\n"));
            return 10;
        }
        pns++;

		 BspUartWrite(2,SIZE_OF("**** Multiple sector write test ****\r\n"));
        lba = 1; ns = sz_buff / sz_sect;
        if (ns > 4) ns = 4;
        for (n = 0, pn(pns); n < (UINT)(sz_sect * ns); n++) pbuff[n] = (BYTE)pn(0);
		
		 BspUartWrite(2,SIZE_OF("disk_write\r\n"));
        dr = disk_write(pdrv, pbuff, lba, ns);
        if (dr == RES_OK) {
             BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
           BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 11;
        }
		
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
		BspUartWrite(2,SIZE_OF(" disk_ioctl\r\n"));
        if (dr == RES_OK) {
           BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
           BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 12;
        }
        memset(pbuff, 0, sz_sect * ns);
		BspUartWrite(2,SIZE_OF(" disk_read.\r\n"));
        dr = disk_read(pdrv, pbuff, lba, ns);
        if (dr == RES_OK) {
             BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
           BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 13;
        }
        for (n = 0, pn(pns); n < (UINT)(sz_sect * ns) && pbuff[n] == (BYTE)pn(0); n++) ;
        if (n == (UINT)(sz_sect * ns)) {
             BspUartWrite(2,SIZE_OF("Data matched.\r\n"));
        } else {
			  BspUartWrite(2,SIZE_OF("Failed: Read data differs from the data written.\r\n"));
            return 14;
        }
        pns++;


		BspUartWrite(2,SIZE_OF("**** Single sector write test (misaligned address) ****\r\n"));
        lba = 5;
        for (n = 0, pn(pns); n < sz_sect; n++) pbuff[n+3] = (BYTE)pn(0);
		BspUartWrite(2,SIZE_OF("disk_write\r\n"));
        dr = disk_write(pdrv, pbuff+3, lba, 1);
        if (dr == RES_OK) {
            BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
           BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 15;
        }
        dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
		BspUartWrite(2,SIZE_OF(" disk_ioctl\r\n"));
        if (dr == RES_OK) {
           BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
           BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 16;
        }
        memset(pbuff+5, 0, sz_sect);
		 BspUartWrite(2,SIZE_OF(" disk_read\r\n"));
        dr = disk_read(pdrv, pbuff+5, lba, 1);
        if (dr == RES_OK) {
            BspUartWrite(2,SIZE_OF("-ok.\r\n"));
        } else {
            BspUartWrite(2,SIZE_OF(" - failed.\r\n"));
            return 17;
        }
        for (n = 0, pn(pns); n < sz_sect && pbuff[n+5] == (BYTE)pn(0); n++) ;
        if (n == sz_sect) {
			 BspUartWrite(2,SIZE_OF("Data matched.\r\n"));
        } else {
			 BspUartWrite(2,SIZE_OF("Failed: Read data differs from the data written.\r\n"));
            return 18;
        }
        pns++;

//        printf("**** 4GB barrier test ****\r\n");
//		
//        if (sz_drv >= 128 + 0x80000000 / (sz_sect / 2)) {
//            lba = 6; lba2 = lba + 0x80000000 / (sz_sect / 2);
//            for (n = 0, pn(pns); n < (UINT)(sz_sect * 2); n++) pbuff[n] = (BYTE)pn(0);
//            printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT)pbuff, lba);
//            dr = disk_write(pdrv, pbuff, lba, 1);
//            if (dr == RES_OK) {
//                printf(" - ok.\r\n");
//            } else {
//                printf(" - failed.\r\n");
//                return 19;
//            }
//            printf(" disk_write(%u, 0x%X, %lu, 1)", pdrv, (UINT)(pbuff+sz_sect), lba2);
//            dr = disk_write(pdrv, pbuff+sz_sect, lba2, 1);
//            if (dr == RES_OK) {
//                printf(" - ok.\r\n");
//            } else {
//                printf(" - failed.\r\n");
//                return 20;
//            }
//            printf(" disk_ioctl(%u, CTRL_SYNC, NULL)", pdrv);
//            dr = disk_ioctl(pdrv, CTRL_SYNC, 0);
//            if (dr == RES_OK) {
//            printf(" - ok.\r\n");
//            } else {
//                printf(" - failed.\r\n");
//                return 21;
//            }
//            memset(pbuff, 0, sz_sect * 2);
//            printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT)pbuff, lba);
//            dr = disk_read(pdrv, pbuff, lba, 1);
//            if (dr == RES_OK) {
//                printf(" - ok.\r\n");
//            } else {
//                printf(" - failed.\r\n");
//                return 22;
//            }
//            printf(" disk_read(%u, 0x%X, %lu, 1)", pdrv, (UINT)(pbuff+sz_sect), lba2);
//            dr = disk_read(pdrv, pbuff+sz_sect, lba2, 1);
//            if (dr == RES_OK) {
//                printf(" - ok.\r\n");
//            } else {
//                printf(" - failed.\r\n");
//                return 23;
//            }
//            for (n = 0, pn(pns); pbuff[n] == (BYTE)pn(0) && n < (UINT)(sz_sect * 2); n++) ;
//            if (n == (UINT)(sz_sect * 2)) {
//                printf(" Data matched.\r\n");
//            } else {
//                printf("Failed: Read data differs from the data written.\r\n");
//                return 24;
//            }
//        } else {
//            printf(" Test skipped.\r\n");
//        }
        pns++;

//        printf("**** Test cycle %u of %u completed ****\r\n\r\n", cc, ncyc);
    }

    return 0;
}

#endif
