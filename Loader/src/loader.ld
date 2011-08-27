OUTPUT_FORMAT("elf32-i386")
ENTRY(_loader)

Offset  = 0x00100000;

SECTIONS
{
    . = Offset;
    
    _start = .;

    .text :
    {
        *(.text)
        . = ALIGN(0x1000);
    }
    
    .data :
    {
        *(.data*)        
        *(.gnu.linkonce.d*)
        *(.rodata*)
		*(.gnu.linkonce.r*)
        
        . = ALIGN(0x1000);
    }
    
    .bss :
    {
        *(COMMON*)
        *(.bss*)
        . = ALIGN(0x1000);
    }

    . = ALIGN(0x1000);
    
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }
    
    _end = .;
}

