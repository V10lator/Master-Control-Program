/*
   minimal_gpio.c
   2019-07-03
   Public Domain
*/

/*
   gcc -o minimal_gpio minimal_gpio.c
   sudo ./minimal_gpio
*/

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "minimal_gpio.h"

#define PI_PERIPH_BASE 0x3F000000

#define GPIO_BASE  (PI_PERIPH_BASE + 0x200000)

#define GPIO_LEN  0xF4

#define GPSET0 7
#define GPSET1 8

#define GPCLR0 10
#define GPCLR1 11

#define GPLEV0 13
#define GPLEV1 14

#define GPPUD     37
#define GPPUDCLK0 38

#define SYST_CS  0
#define SYST_CLO 1
#define SYST_CHI 2

static volatile uint32_t  *gpioReg = MAP_FAILED;

#define PI_BANK (gpio>>5)
#define PI_BIT  (1<<(gpio&0x1F))

void gpioSetMode(unsigned gpio, GPIO_MODE mode)
{
   int reg, shift;

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   gpioReg[reg] = (gpioReg[reg] & ~(7<<shift)) | (mode<<shift);
}

GPIO_MODE gpioGetMode(unsigned gpio)
{
   int reg, shift;

   reg   =  gpio/10;
   shift = (gpio%10) * 3;

   return (*(gpioReg + reg) >> shift) & 7;
}

bool gpioRead(unsigned gpio)
{
   return *(gpioReg + GPLEV0 + PI_BANK) & PI_BIT;
}

void gpioWrite(unsigned gpio, bool level)
{
   *(gpioReg + (level ? GPSET0 : GPCLR0) + PI_BANK) = PI_BIT;
}

void gpioTrigger(unsigned gpio, unsigned pulseLen, bool level)
{
   gpioWrite(gpio, level);
   usleep(pulseLen);
   gpioWrite(gpio, !level);
}

/* Bit (1<<x) will be set if gpio x is high. */

uint32_t gpioReadBank1(void) { return (*(gpioReg + GPLEV0)); }
uint32_t gpioReadBank2(void) { return (*(gpioReg + GPLEV1)); }

/* To clear gpio x bit or in (1<<x). */

void gpioClearBank1(uint32_t bits) { *(gpioReg + GPCLR0) = bits; }
void gpioClearBank2(uint32_t bits) { *(gpioReg + GPCLR1) = bits; }

/* To set gpio x bit or in (1<<x). */

void gpioSetBank1(uint32_t bits) { *(gpioReg + GPSET0) = bits; }
void gpioSetBank2(uint32_t bits) { *(gpioReg + GPSET1) = bits; }

static inline uint32_t * initMapMem(int fd, uint32_t addr, uint32_t len)
{
    return (uint32_t *) mmap(0, len,
       PROT_READ|PROT_WRITE|PROT_EXEC,
       MAP_SHARED|MAP_LOCKED,
       fd, addr);
}

bool gpioInitialise(void)
{
   int fd = open("/dev/gpiomem", O_RDWR | O_SYNC) ;

   if (fd < 0)
   {
      fprintf(stderr, "[GPIO DRIVER] Error opening /dev/gpiomem\n");
      return false;
   }

   gpioReg  = initMapMem(fd, GPIO_BASE,  GPIO_LEN);

   close(fd);

   if (gpioReg == MAP_FAILED)
   {
      fprintf(stderr,
         "[GPIO DRIVER] Bad, mmap failed\n");
      return false;
   }
   return true;
}

void shutdownGpio()
{
	if(gpioReg != MAP_FAILED)
	{
		munmap((void *)gpioReg, GPIO_LEN);
		gpioReg = MAP_FAILED;
	}
}
