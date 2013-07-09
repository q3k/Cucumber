#include "paging.h"
#include "context.h"
#include "io.h"

u32 g_FramePointer;
u64 *g_PML4;

void _zero_paging_structure(u64 *Structure)
{
    for (unsigned i = 0; i < 512; i++)
        Structure[i] = 0;
}

u32 _allocate_frame_4k(void)
{
    u32 Frame = g_FramePointer;
    if (Frame > 0xEFFFFF)
    {
        puts("BUG: Out of extended memory.\n");
        for (;;) {}
    }
    g_FramePointer += 0x1000;
    return Frame;
}

void paging_setup(u32 AllocateFramesFrom)
{
    g_FramePointer = AllocateFramesFrom;

    // set up PML4
    g_PML4 = (u64*)_allocate_frame_4k();
    _zero_paging_structure(g_PML4);
}

u8 dbg = 0;
void paging_map_page(u64 Virtual, u64 Physical)
{
    if (Virtual % 0x1000 || Physical % 0x1000)
    {
        puts("BUG: Requsted allocation of unaligned address.\n");
        print_hex(Virtual);
        for (;;) {}
    }
    // printf("Mapping %x\n", Virtual);
    
    u64 PML4I = GET_PML4_ENTRY(Virtual);
    u64 PDPI = GET_PDP_ENTRY(Virtual);
    u64 DIRI = GET_DIR_ENTRY(Virtual);
    u64 TABI = GET_TAB_ENTRY(Virtual);
    // printf("%x %i/%i/%i/%i\n", Virtual, PML4I, PDPI, DIRI, TABI);

    u64 *PDP = (u64 *)(g_PML4[PML4I] & 0xFFFFF000);
    if (PDP == 0)
    {
        // printf("-> Creating PDP @%i\n", PML4I);
        PDP = (u64 *)_allocate_frame_4k();
        _zero_paging_structure(PDP);
        g_PML4[PML4I] = ((u64)PDP) | 3;
    }
    u64 *Directory = (u64 *)(PDP[PDPI] & 0xFFFFF000);
    if (Directory == 0)
    {
        // printf("-> Creating Dirctory @%i/%i\n", PML4I, PDPI);
        Directory = (u64 *)_allocate_frame_4k();
        _zero_paging_structure(Directory);
        PDP[PDPI] = ((u64)Directory) | 3;
        printf("P%x[%i] < d%x\n", (u64)PDP, PDPI, (u64)Directory);
        dbg = 0;
    }
    u64 *Table = (u64 *)(Directory[DIRI] & 0xFFFFF000);
    if (Table == 0)
    {
        if (dbg < 3)
        {
            printf("P%x[%i] == D%x\n", (u64)PDP, PDPI, (u64)Directory);
            dbg += 1;
        }
        // printf("P@ %x D@ %x, De %x\n", (u64)PDP, (u64)Directory, Directory[DIRI]);
        // printf("-> Creating Table @%i/%i/%i\n", PML4I, PDPI, DIRI);
        Table = (u64 *)_allocate_frame_4k();
        // printf("aT %x\n", Table);
        _zero_paging_structure(Table);
        Directory[DIRI] = ((u64)Table) | 3;
        // printf("De <- %x\n", Directory[DIRI]);
    }
    Table[TABI] = ((u64)Physical) | 3;
}

void paging_map_address(u64 PhysicalStart, u64 VirtualStart, u64 Size)
{
    if (VirtualStart % 0x1000 || PhysicalStart % 0x1000)
    {
        puts("BUG: Requsted allocation of unaligned address.\n");
        for (;;) {}
    }
    u64 AlignedSize = Size;
    if (AlignedSize % 0x1000)
        AlignedSize = (AlignedSize + 0x1000) & 0xFFFFF000;
    for (u64 i = 0; i < AlignedSize; i += 0x1000)
    {
        paging_map_page(VirtualStart + i, PhysicalStart + i);
    }
}

void *paging_allocate(u64 VirtualStart, u64 Size)
{
    // let's first allocate all the pages for the actual data
    u32 Start = 0;
    for (u64 i = 0; i < Size; i += 0x1000)
    {
        u64 Frame = _allocate_frame_4k();
        if (!Start)
            Start = Frame;
    }

    // and now let's map the data (and allocate frames for mapping structures)
    u32 Frame = Start;
    for (u64 Virtual = VirtualStart; (Virtual - VirtualStart) < Size; Virtual += 0x1000)
    {
        paging_map_page(Virtual, Frame);
        Frame += 0x1000;
    }
    return (void *)Start;
}

u32 paging_get_last_frame(void)
{
    return g_FramePointer;
}

void *paging_get_pml4(void)
{
    return g_PML4;
}