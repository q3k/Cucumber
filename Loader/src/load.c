
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char s8;

u8 stdio_current_line = 0;
u8 stdio_cur_x = 0, stdio_cur_y = 0;

void outb(u16 Port, u8 Data)
{
    __asm__ volatile("outb %1, %0" :: "dN" (Port), "a" (Data));
}

void *memcpy(void* Destination, const void *Source, u32 Count)
{
    u8* Destination8 = (u8*)Destination;
    u8* Source8 = (u8*)Source;

    while (Count--)
    {
        *Destination8++ = *Source8++;
    }
    return Destination;
}

void *memset(void *Destination, u8 Value, u32 Count)
{
    u8 *us = (u8 *)Destination;
    while (Count-- != 0)
        *us++ = Value;
    return Destination;
}

void *memsetw(void *Destination, u16 Value, u32 Count)
{
    u16 *us = (u16 *)Destination;
    while (Count-- != 0)
        *us++ = Value;
    return Destination;
}


void scroll_up(void)
{
   //semaphore_acquire(&ScreenWriteLock);
   u16 Blank = 0x20 | (0x0F << 8);
   u16 Temp;

   if (stdio_cur_y >= 25)
   {
        Temp = stdio_cur_y - 25 + 1;
        memcpy((void*)0xB8000, (void*)(0xB8000 + Temp * 80 * 2), (25 - Temp) * 80 * 2);

        memsetw((void*)(0xB8000 + (25 - Temp) * 160), Blank, 160);
        stdio_cur_y = 25 - 1;
   }
   //semaphore_release(&ScreenWriteLock);
}

void move_cursor(u8 X, u8 Y)
{
    stdio_cur_x = X;
    stdio_cur_y = Y;

    //wraparound
    if (stdio_cur_x >= 80)
    {
        stdio_cur_y += stdio_cur_y / 80;
        stdio_cur_x = 0;
    }

    //wrapup
    scroll_up();

    if (Y > 24)
        Y = 24;

    u16 Position = Y * 80 + X;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(Position & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)(Position >> 8 & 0xFF));
}

void putch(s8 Character)
{
    volatile u8 *VideoMemory = (u8 *)0xB8000;
    u16 Offset = (stdio_cur_y * 80 + stdio_cur_x) << 1;

    if (Character == '\n')
        move_cursor(0, stdio_cur_y + 1);
    else
    {
        VideoMemory[Offset] = Character;
        VideoMemory[Offset+1] = 0x0F;
        if (stdio_cur_x + 1 >= 80)
            move_cursor(0, stdio_cur_y + 1);
        else
            move_cursor(stdio_cur_x + 1, stdio_cur_y);
    }
}

void puts(const s8 *szString)
{
    while (*szString != 0)
    {
        putch(*szString);
        szString++;
    }
}

void clear(void)
{
    volatile u8 *VideoMemory = (u8 *)0xB8000;
    u32 Size = (80 * 25 ) << 1;
    for (u32 i = 0; i < Size; i += 2)
    {
        VideoMemory[i] = 0;
        VideoMemory[i+1] = 0xF;
    }
    move_cursor(0, 0);
}

void dump_nibble(u8 Nibble)
{
    if (Nibble < 10)
        putch(Nibble + 48);
    else
        putch(Nibble + 55);
}

void print_hex(u64 Number)
{
    for (s8 i = 7; i >= 0; i--)
    {
        u8 Byte = (Number >> (i << 3)) & 0xFF; //switch i bytes to the right and mask as byte
        dump_nibble((Byte >> 4)  & 0x0F); //high nibble
        dump_nibble(Byte & 0x0F); //low nibble
    }
}

struct elf_ident {
	u32 Magic; // \x7FELF
	u8 Class;
	u8 Data;
	u8 Version;
	u8 Padding[9];
};

struct elf_header {
	struct elf_ident Identification;
	u16 Type;
	u16 Machine;
	u32 Version;
	u64 Entry;
	u64 ProgramHeaderOffset;
	u64 SectionHeaderOffset;
	u32 Flags;
	u16 HeaderSize;
	u16 ProgramHeaderEntrySize;
	u16 NumProgramHeaderEntries;
	u16 SectionHeaderEntrySize;
	u16 NumSectionHeaderEntries;
	u16 SectionEntryStrings;
};

struct elf_section_header {
	u32 Name;
	u32 Type;
	u64 Flags;
	u64 Address;
	u64 Offset;
	u64 Size;
	u32 Link;
	u32 Info;
	u64 Alignment;
	u64 FixedSize;
};

static inline void cpuid(u32 code, u32 *a, u32 *d) {
	__asm__ volatile("cpuid":"=a"(*a),"=d"(*d):"0"(code):"ecx","ebx");
}


// A simple PAE paging structure so we can jump into 64-bit.
// This will be replaced later on by the kernel code.

u64 pml4[512] __attribute__((aligned(0x1000)));

u64 page_dir_ptr_tab_low[512] __attribute__((aligned(0x1000)));
u64 page_dir_low[512] __attribute__((aligned(0x1000)));
u64 page_tab_low[512] __attribute__((aligned(0x1000)));

u64 page_dir_ptr_tab_high[512] __attribute__((aligned(0x1000)));
u64 page_dir_high[512] __attribute__((aligned(0x1000)));
u64 page_tab_high[512] __attribute__((aligned(0x1000)));

#define GET_PML4_ENTRY(x) (((u64)x >> 39) & 0x1FF)
#define GET_PDP_ENTRY(x) (((u64)x >> 30) & 0x1FF)
#define GET_DIR_ENTRY(x) (((u64)x >> 21) & 0x1FF)
#define GET_TAB_ENTRY(x) (((u64)x >> 12) & 0x1FF)
#define GET_OFFSET(x) (x & 0xFFF)

u32 create_ia32e_paging(u64 KernelPhysicalStart, u64 KernelVirtualStart, u64 KernelSize)
{
	puts("Clearing paging structures...\n");

	for (u16 i = 0; i < 512; i++)
	{
		pml4[i] = 0;
		page_dir_ptr_tab_low[i] = 0;
		page_dir_low[i] = 0;
		page_tab_low[i] = 0;
		page_dir_ptr_tab_high[i] = 0;
		page_dir_high[i] = 0;
		page_tab_high[i] = 0;
	}

	puts("Setting up identity paging for first 2MiB...\n");
	pml4[GET_PML4_ENTRY(0)] = (u32)page_dir_ptr_tab_low | 3;
	page_dir_ptr_tab_low[GET_PDP_ENTRY(0)] = (u32)page_dir_low | 3;
	page_dir_low[GET_DIR_ENTRY(0)] = (u32)page_tab_low | 3;

	u64 Address = 0;
	for (u16 i = 0; i < 512; i++)
	{
		page_tab_low[i] = Address | 3;
		Address += 0x1000;
	}

	puts("Setting up paging for the kernel...\n");
	u16 NumPages = KernelSize / 0x1000;
	puts("  (0x");
	print_hex(NumPages);
	puts(" pages)\n");

	if (NumPages > 512)
	{
		puts("Error: Kernel size > 2MiB not implemented!");
		return 1;
	}

	if (GET_PML4_ENTRY(KernelVirtualStart) != 0)
	{
		// We're NOT mapping the same PML4 entry as for identity mapping...
		puts("Different PML4...\n");
		pml4[GET_PML4_ENTRY(KernelVirtualStart)] = (u32)page_dir_ptr_tab_high | 3;
		page_dir_ptr_tab_high[GET_PDP_ENTRY(KernelVirtualStart)] = (u32)page_dir_high | 3;
		page_dir_high[GET_PDP_ENTRY(KernelVirtualStart)] = (u32)page_tab_high | 3;
	}
	else if (GET_PDP_ENTRY(KernelVirtualStart) != 0)
	{
		// We're NOT mapping the same page directory pointer table entry as for identity paging...
		puts("Different PDPT... (");
		print_hex(GET_PDP_ENTRY(KernelVirtualStart));
		puts(")\n");
		page_dir_ptr_tab_low[GET_PDP_ENTRY(KernelVirtualStart)] = (u32)page_dir_high | 3;
		page_dir_high[GET_DIR_ENTRY(KernelVirtualStart)] = (u32)page_tab_high | 3;
	}
	else if (GET_DIR_ENTRY(KernelVirtualStart) != 0)
	{
		// We're NOT mapping the same page directory entry as for identity paging...
		puts("Different DIR...\n");
		page_dir_low[GET_DIR_ENTRY(KernelVirtualStart)] = (u32)page_tab_high | 3;
	}
	else
	{
		puts("Error: kernel overlaps 2MiB identity paging!\n");
		return 1;
	}

	Address = KernelPhysicalStart;
	for (u16 i = GET_TAB_ENTRY(KernelVirtualStart); i < GET_TAB_ENTRY(KernelVirtualStart) + NumPages; i++)
	{
		page_tab_high[i] = Address | 3;
		/*print_hex(KernelVirtualStart + i * 0x1000);
		puts(" -> ");
		print_hex(Address);
		puts("\n");*/
		Address += 0x1000;
	}

	return 0;
}

u32 g_multiboot_header;

u32 load(void *Multiboot, unsigned int Magic)
{
    clear();
    puts("Cucumber x86-64 loader...\n");

    g_multiboot_header = (u32)Multiboot;


    if (Magic != 0x2BADB002)
    {
	    puts("Error: not booted via Multiboot!\n");
 	    return 0;
    }

    u32 CPUID_A, CPUID_D;
    cpuid(0x80000001, &CPUID_A, &CPUID_D);

    u8 SupportFor64 = (CPUID_D & (1 << 29)) > 0;

    if (!SupportFor64)
    {
    	puts("Error: You CPU does not support long mode!\n");
    	return 0;
    }

    u32 Flags = *((u32*)Multiboot);

    u8 ModulesPresent = (Flags & (1 << 3)) > 0;

    if (!ModulesPresent)
    {
	    puts("Error: no 64-bit kernel loaded!\n");
	    puts("  (did you forget the module line in GRUB?)\n");
	    return 0;
    }

    u32 ModulesCount = *((u32*)Multiboot + 5);
    u32 ModulesAddress = *((u32*)Multiboot + 6);

    if (ModulesCount != 1)
    {
	    puts("Error: just one module is enough. Don't load a ton of them.\n");
	    return 0;
    }

    puts("Kernel is @");
    print_hex(ModulesAddress);
    puts(".\n");

    struct elf_header *Header = *((struct elf_header **)ModulesAddress);
    if (Header->Identification.Magic != 0x464C457F)
    {
	    puts("Error: Module is not an ELF file!\n");
	    return 0;
    }

    if (Header->Identification.Class != 2)
    {
 	   puts("Error: Module is not a 64-bit ELF file!\n");
 	   return 0;
    }

    if (!Header->Entry)
    {
 	   puts("Error: Kernel does not have entry point!\n");
 	   return 0;
    }

    puts("Entry point @");
    print_hex(Header->Entry);
    puts(".\n");

    if (Header->SectionHeaderEntrySize != sizeof(struct elf_section_header))
    {
    	puts("Error: Weird section header entry size!\n");
	    return 0;
    }

    struct elf_section_header *Sections = (struct elf_section_header *)((u32)Header + (u32)Header->SectionHeaderOffset);
    struct elf_section_header *StringSection = &Sections[Header->SectionEntryStrings];

    //u32 *Strings = (u32*)(ModulesAddress + (u32)StringSection->Offset);

    puts("0x");
    print_hex(Header->NumSectionHeaderEntries);
    puts(" ELF sections.\n");

    u64 ContinuityTest = 0;
    u64 StartPhysical = 0;
    u64 StartVirtual = 0;
    u64 Size = 0;

    for (u16 i = 0; i < Header->NumSectionHeaderEntries; i++)
    {
	    s8* Name = (s8*)((u32)Header + (u32)StringSection->Offset + (u32)Sections[i].Name);
	    u64 PhysicalAddress = (u32)Header + Sections[i].Offset;
	    u64 VirtualAddress = Sections[i].Address;

	    if (VirtualAddress)
	    {
	    	if (!StartVirtual)
	    		StartVirtual = VirtualAddress;

	    	if (!StartPhysical)
	    		StartPhysical = PhysicalAddress;

	    	if (ContinuityTest && VirtualAddress != ContinuityTest)
			{
				puts("Error: kernel is not continuous!\n");
				return 0;
			}

			ContinuityTest = VirtualAddress + Sections[i].Size;
			Size += Sections[i].Size;

		    puts("-> Section ");
		    puts(Name);
		    puts(", 0x");
		    print_hex(PhysicalAddress);
		    puts(" will be located at 0x");
		    print_hex(VirtualAddress);
		    puts(".\n");
	    }
    }

    puts("\nPaging setup:\n 0x");
    print_hex(StartVirtual);
    puts(" => 0x");
    print_hex(StartPhysical);
    puts("\n  (0x");
    print_hex(Size);
    puts(" bytes)\n");

    if (create_ia32e_paging(StartPhysical, StartVirtual, Size))
    	return 0;
    __asm__ volatile ("movl %cr4, %eax; bts $5, %eax; movl %eax, %cr4");
    __asm__ volatile ("movl %%eax, %%cr3" :: "a" (pml4));
    puts("CR3 is now pointing to PML4 (0x");
    print_hex((u32)pml4);
    puts(")\n");

    puts("Here it goes, enabling long mode...\n");

    __asm__ volatile(	"movl $0xc0000080, %%ecx;\n"
    					"rdmsr;\n"
    					"orl $0x100, %%eax;\n"
    					"wrmsr;\n"
    					"movl %%cr0, %%ebx;\n"
    					"bts $31, %%ebx;\n"
    					"movl %%ebx, %%cr0;":::"eax","ebx","ecx");

    puts("Now in 32-bit compability mode, jumping to the kernel...\n");

    return 1;
}
