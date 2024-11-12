/* -----------------------------------------------------------------------------
 * Copyright (c) 2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        12. November 2023
 * $Revision:    V0.1.0
 *
 * Project:      Flash Device Description for ST STM32WL3x Flash
 * --------------------------------------------------------------------------- */

/* History:
 *  Version 0.1.0
 *    Initial release
 */

#include "..\FlashOS.h"        // FlashOS Structures
#include <stdio.h>


typedef volatile unsigned char    vu8;
typedef          unsigned char     u8;
typedef volatile unsigned short   vu16;
typedef          unsigned short    u16;
typedef volatile unsigned long    vu32;
typedef          unsigned long     u32;

#define FLASH_SIZE_MASK (0x1FFFF)
#define M8(adr)  (*((vu8  *) (adr)))
#define M16(adr) (*((vu16 *) (adr)))
#define M32(adr) (*((vu32 *) (adr)))

#define NVM_BASE_ADDRESS ((u32)0x40001000)

// Registers_STM32WB09xx
#define NVM_REG_COMMAND         (NVM_BASE_ADDRESS + 0x00)    // 0x00
#define NVM_REG_CONFIG          (NVM_BASE_ADDRESS + 0x04)    // 0x04
#define NVM_REG_IRQSTAT         (NVM_BASE_ADDRESS + 0x08)    // 0x08
#define NVM_REG_IRQMASK         (NVM_BASE_ADDRESS + 0x0C)    // 0x0C
#define NVM_REG_IRQRAW          (NVM_BASE_ADDRESS + 0x10)    // 0x10
#define NVM_REG_ADDRESS         (NVM_BASE_ADDRESS + 0x18)    // 0x18
#define NVM_REG_DATA            (NVM_BASE_ADDRESS + 0x40)    // 0x40
#define NVM_REG_DATA1           (NVM_BASE_ADDRESS + 0x44)    // 0x44
#define NVM_REG_DATA2           (NVM_BASE_ADDRESS + 0x48)    // 0x48
#define NVM_REG_DATA3           (NVM_BASE_ADDRESS + 0x4C)    // 0x4C


// Bit fields_STM32WB09xx

#define NVM_IRQ_CMDDONE         0x01
#define NVM_IRQ_CMDSTART        0x02
#define NVM_IRQ_CMDERR          0x04
#define NVM_IRQ_ILLCMD          0x08
#define NVM_IRQ_READOK          0x10
#define NVM_IRQ_FLNREADY        0x20

#define NVM_CMD_ERASE           0x11
#define NVM_CMD_MASSERASE       0x22
#define NVM_CMD_WRITE           0x33
#define NVM_CMD_BURSTWRITE      0xCC

/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_MEM
int Init (u32 adr, u32 clk, u32 fnc) { 
	
  /* Clear status */
	*((u32 *)NVM_REG_IRQMASK) = 0x00000000;
  *((u32 *)NVM_REG_IRQSTAT) = 0xFFFFFFFF;

  return (0);
}
#endif

/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_MEM
int UnInit (u32 fnc) {
  return (0);
}
#endif

/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_MEM
int EraseChip (void) {
	vu32 stat = 0;
	
	/* Clear status */
  *((vu32*)NVM_REG_IRQSTAT) = 0xFFFFFFFF;
   
  /* Mass erase command */
  *((vu32*)NVM_REG_COMMAND) = NVM_CMD_MASSERASE;
 
	do {
    stat = *((vu32*)NVM_REG_IRQSTAT);
  } while((stat & NVM_IRQ_CMDDONE) != NVM_IRQ_CMDDONE);

  stat = *((u32*)NVM_REG_IRQSTAT);
  if( stat & (NVM_IRQ_CMDERR | NVM_IRQ_ILLCMD) )  // error
    return (1);
 
  return (0);                                    
}
#endif

/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

#ifdef FLASH_MEM

int EraseSector (u32 adr) {
	
	vu32 stat = 0;
	
	*((vu32*)NVM_REG_IRQSTAT) = 0xFFFFFFFF;
	
  *((vu32*)NVM_REG_ADDRESS) = (adr - 0x10040000)/4;
  *((vu32*)NVM_REG_COMMAND) =  NVM_CMD_ERASE;

	do {
    stat = *((vu32*)NVM_REG_IRQSTAT);
  } while((stat & NVM_IRQ_CMDDONE) != NVM_IRQ_CMDDONE);
  
		
  stat = *((u32*)NVM_REG_IRQSTAT);
  if( stat & (NVM_IRQ_CMDERR | NVM_IRQ_ILLCMD) ) // error
    return (1);
  
	return (0); // Done
}
#endif


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */



#ifdef FLASH_MEM
int ProgramPage (u32 Address, u32 Size, unsigned char *buffer) {

  int Addr = ((((u32)Address) - 0x10040000)/4) & FLASH_SIZE_MASK;;
	vu32 stat = 0;
  u32* src = (u32 *)buffer;

	
	if (Size % 16) // for BURST WRITING: if needed, count is rounded to the next
    // multiple of 16
   Size = Size - (Size % 16) + 16 ;
  
	for( ; Size; Size-=16)
  { 
		
		*((vu32*)NVM_REG_IRQSTAT) = 0xFFFFFFFF;
    
		/* Write Address offset */
    *((vu32*)NVM_REG_ADDRESS) = Addr;

    /* Write Data */
		*((vu32*)NVM_REG_DATA)  = *src++;
		*((vu32*)NVM_REG_DATA1) = *src++;
		*((vu32*)NVM_REG_DATA2) = *src++;
		*((vu32*)NVM_REG_DATA3) = *src++;

    /* Write command */
    *((vu32*)NVM_REG_COMMAND) = NVM_CMD_BURSTWRITE;

    /* wait for complete operation */    
		do {
			stat = *((vu32*)NVM_REG_IRQSTAT);
		} while((stat & NVM_IRQ_CMDDONE) != NVM_IRQ_CMDDONE);
		
		Addr += 4;
  }

	return (0); // Done
}

#endif
