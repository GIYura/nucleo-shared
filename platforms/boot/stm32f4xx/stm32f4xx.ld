/*-----------------------------------------------------------------------------------------------------------------------
* Linker script for STM32F4xx
*
* Board: STM32F411-Nucleo
-------------------------------------------------------------------------------------------------------------------------*/

/* Entry point of the program */
ENTRY(Reset_Handler)

/* RAM Top for initial SP */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Memory regions 
Memories (datasheet DS10314 Rev 8)
– Up to 512 Kbytes of flash memory
– 128 Kbytes of SRAM
*/
MEMORY
{
  /* ORIGIN, LENGTH are standard-key words of the linker script  
  (attribute list)
  r - read-only
  w - read and write
  x - exacutable code
  a - allocated section
  i - initialized section
  ! - inverts meaning of the the following attributes
  ORIGIN = start address

  (RM0383) Table 3. Memory mapping vs. Boot mode/physical remap in STM32F411xC/E
  0x2000 0000 - 0x2002 0000   SRAM1 (128 KB)
  0x0800 0000 - 0x0807 FFFF   Flash memory
  */
  RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  ROM (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
}

/* --- VECTOR TABLE --- */
SECTIONS
{
  .isr_vector : /* label of the output section */
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >ROM /* in this case VMA and LMA are the same */

/* --- CODE --- */
  .text : /* create output sectioon of ELF named ".text" */
  {
    . = ALIGN(4);
    *(.text*)   /* fill output section with sections from object files which names starts from ".text" */
    . = ALIGN(4);
    _etext = .;
  } >ROM

/* --- CONST --- */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata*)
    . = ALIGN(4);
  } >ROM

/* --- INIT-DATA (ROM -> RAM copy) --- */
  _sidata = LOADADDR(.data);    /* Used by the startup to initialize data (data in ROM) = LMA */
  
  .data :
  {
    . = ALIGN(4);
    _sdata = .;     /* start addres of RAM (VMA) */
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } >RAM AT> ROM

/* --- BSS (zero-fill) --- */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } >RAM

/* --- HEAP and STACK reservation --- */
  ._heap_stack :
  {
    . = ALIGN(8);
    /* HEAP disabled */
    . = . + 0x1000;  /* 4 KB stack */
    . = ALIGN(8);
  } >RAM
}

/*-----------------------------------------------------------------------------------------------------------------------
* EOF
-------------------------------------------------------------------------------------------------------------------------*/
