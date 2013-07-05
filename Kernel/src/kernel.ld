OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

KERNEL_VMA = 0xffffffff80000000;

SECTIONS
{
    . = KERNEL_VMA;
    
    _start = .;

    .text :
    {
        _code = .;
        *(.text)
        *(.text.*)
        *(.rodata*)

        . = ALIGN(8);

        _start_ctors = .;
            *(.ctors)
        _end_ctors = .;
    }

    . = ALIGN(4096);

   .data :
   {
        _data = .;
        *(.data)
   }

   . = ALIGN(4096);

   .bss :
   {
       _bss = .;
       *(.bss)
       *(COMMON)
   }

   _end = .;

   /DISCARD/ :
   {
        *(.eh_frame*)
        *(.ehframe*)
        *(.debug*)

   }
}
