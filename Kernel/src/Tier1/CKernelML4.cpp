#include "Tier1/CKernelML4.h"
using namespace cb;

extern "C"
{
    #include "Tier0/paging.h"
    #include "Tier0/heap.h"
    #include "Tier0/system.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
}

#define ASSERT_ALIGNED(m) ASSERT(!(m & 0xFFF))
#define POPULATE_PAGING_ENTRY(Entry, address) do { Entry.Present = 0;\
    Entry.RW = 0; \
    Entry.User = 0; \
    Entry.Misc = 0; \
    Entry.Zero = 0; \
    Entry.Physical = address >> 12; } while(0)

void CKernelML4::PopulateCommonPointers(void)
{
    T_PAGING_ML4 *ML4 = (T_PAGING_ML4*)kmalloc_aligned(sizeof(T_PAGING_ML4));
    for (u16 i = 0; i < 256; i++)
        ML4->Entries[i].Present = 0;
    // start with TEXT, this is the easiest (16 directory entries)
    // TEXT: ml4 entry 511, dpt entry 510, dir entries 0 - 15
    T_PAGING_DPT *TextDPT = (T_PAGING_DPT*)kmalloc_aligned(sizeof(T_PAGING_DPT));
    for (u16 i = 0; i < 256; i++)
        TextDPT->Entries[i].Present = 0;

    POPULATE_PAGING_ENTRY(ML4->Entries[511], paging_scratch_get_physical(TextDPT));
    ASSERT_ALIGNED(paging_scratch_get_physical(TextDPT));

    T_PAGING_DIR *TextDirectory = (T_PAGING_DIR*)kmalloc_aligned(sizeof(T_PAGING_DIR));
    for (u16 i = 0; i < 256; i++)
        TextDirectory->Entries[i].Present = 0;

    u64 KernelStart = system_get_kernel_physical_start();
    for (u16 i = 0; i < 16; i++)
    {
        T_PAGING_TAB *Table = (T_PAGING_TAB*)kmalloc_aligned(sizeof(T_PAGING_TAB));
        for (u16 j = 0; j < 256; i++)
        {
            POPULATE_PAGING_ENTRY(Table->Entries[i], KernelStart);
            KernelStart += 4096;
        }
        u64 TablePhysical = paging_scratch_get_physical(Table);
        ASSERT_ALIGNED(TablePhysical);
        POPULATE_PAGING_ENTRY(TextDirectory->Entries[i], TablePhysical);
    }
    m_TEXT = TextDirectory;
    m_TEXT_Physical = paging_scratch_get_physical(TextDirectory);
    POPULATE_PAGING_ENTRY(TextDPT->Entries[510], paging_scratch_get_physical(TextDirectory));
    ASSERT_ALIGNED(paging_scratch_get_physical(TextDirectory));


    // next let's populate LOWMEM (1/2 of a Table)
    // LOWMEM: ml4 entry 0, dpt entry 0, dir entry 0, table entries 0-127
    T_PAGING_DPT *LowmemDPT = (T_PAGING_DPT*)kmalloc_aligned(sizeof(T_PAGING_DPT));
    for (u16 i = 0; i < 256; i++)
        LowmemDPT->Entries[i].Present = 0;

    POPULATE_PAGING_ENTRY(ML4->Entries[0], paging_scratch_get_physical(LowmemDPT));
    ASSERT_ALIGNED(paging_scratch_get_physical(LowmemDPT));

    T_PAGING_DIR *LowmemDirectory = (T_PAGING_DIR*)kmalloc_aligned(sizeof(T_PAGING_DIR));
    for (u16 i = 0; i < 256; i++)
        LowmemDirectory->Entries[i].Present = 0;

    POPULATE_PAGING_ENTRY(LowmemDPT->Entries[0], paging_scratch_get_physical(LowmemDirectory));
    ASSERT_ALIGNED(paging_scratch_get_physical(LowmemDirectory));

    T_PAGING_TAB *LowmemTable = (T_PAGING_TAB*)kmalloc_aligned(sizeof(T_PAGING_TAB));
    for (u16 i = 0; i < 128; i++)
        POPULATE_PAGING_ENTRY(LowmemTable->Entries[i], 4096 * i);

    for (u16 i = 128; i < 256; i++)
        LowmemTable->Entries[i].Present = 0;

    POPULATE_PAGING_ENTRY(LowmemDirectory->Entries[0], paging_scratch_get_physical(LowmemTable));
    ASSERT_ALIGNED(paging_scratch_get_physical(LowmemTable));

    m_LOWMEM = LowmemTable;
    m_LOWMEM_Physical = paging_scratch_get_physical(LowmemTable);

    // aaand do the SCRATCH (one whole dirctory of tables)
    // SCRATCH: ml4 entry 511, dpt entry 509, dir entries 0 - 255
    // T_PAGING_DIR *ScratchDirectory = (T_PAGING_DIR*)kmalloc_aligned(sizeof(T_PAGING_DIR));

    // u64 ScratchStart = 0xFFFFFFFF40000000;
    // for (u16 i = 0; i < 256; i++)
    // {
    //     T_PAGING_TAB *Table = (T_PAGING_TAB*)kmalloc_aligned(sizeof(T_PAGING_TAB));
    //     for (u16 j = 0; j < 256; i++)
    //     {
    //         Table->Entries[j].Present = 1;
    //         Table->Entries[j].RW = 0;
    //         Table->Entries[j].User = 0;
    //         Table->Entries[j].Misc = 0;
    //         Table->Entries[j].Zero = 0;
            
    //         POPULATE_PAGING_ENTRY(Table->Entries[j], ScratchStart);
    //         ScratchStart += 4096;
    //     }
    //     u64 TablePhysical = paging_scratch_get_physical(Table);
    //     POPULATE_PAGING_ENTRY(ScratchDirectory->Entries[i], TablePhysical);
    //     ASSERT_ALIGNED(TablePhysical);
    // }

    // // SCRATCH and TEXT share the same DPT
    // TextDPT->Entries[509].Present = 0;
    // TextDPT->Entries[509].RW = 0;
    // TextDPT->Entries[509].User = 0;
    // TextDPT->Entries[509].Misc = 0;
    // TextDPT->Entries[509].Zero = 0;
    // TextDPT->Entries[509].Physical = paging_scratch_get_physical(ScratchDirectory) >> 12;
    // ASSERT_ALIGNED(paging_scratch_get_physical(ScratchDirectory));
}