INCLUDE util/syms-boot.ld

MEMORY
{
  /* these values are STM32L4-dependent */
  FLASH (rx)  : ORIGIN = _rom_end_, LENGTH = 0x08000000+256K-_rom_end_
  RAM (xrw)   : ORIGIN = _ram_end_, LENGTH = 0x20000000+48K-_ram_end_
}

INCLUDE util/common.ld
