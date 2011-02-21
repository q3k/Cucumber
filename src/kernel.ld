OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

Offset  = 0x00100000;
LMA     = 0x00000000;
VMA     = 0xC0000000;

SECTIONS
{
    . = Offset;

    g_setup_gdt_ptr = g_gdt_ptr - VMA;
    g_setup_gdt_entries = g_gdt_entries - VMA;

    .setup LMA + Offset : AT(LMA + Offset)
    {
        *(.setup)
        . = ALIGN(0x1000);
    }

    . += VMA;

    .text : AT(g_section_code)
    {
        g_section_code = . - VMA;
        *(.text)
        . = ALIGN(0x1000);
    }
    
    .data : AT(g_section_data)
    {
        g_section_data = . - VMA;
        *(.data)
        *(.rodata*)
        . = ALIGN(0x1000);
    }
    
    .bss : AT(g_section_bss)
    {
        g_section_bss = . - VMA;
        *(COMMON*)
        *(.bss*)
        . = ALIGN(0x1000);
    }

    . = ALIGN(0x1000);
}

