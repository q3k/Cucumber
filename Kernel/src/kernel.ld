OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

KERNEL_VMA = 0x0000800000000000;

SECTIONS
{
    . = KERNEL_VMA;
    
    _start = .;

    .text : AT(ADDR(.text) - KERNEL_VMA)
    {
        _code = .;
        *(.text)
        *(.rodata*)
        
        . = ALIGN(4096);
    }

   .data : AT(ADDR(.data) - KERNEL_VMA)
   {
        _data = .;
        *(.data)
        . = ALIGN(4096);
   }

   .ehframe : AT(ADDR(.ehframe) - KERNEL_VMA)
   {
       _ehframe = .;
       *(.ehframe)
        . = ALIGN(4096);
   }

   .bss : AT(ADDR(.bss) - KERNEL_VMA)
   {
       _bss = .;
       *(.bss)
        *(COMMON)
       . = ALIGN(4096);
   }

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}
