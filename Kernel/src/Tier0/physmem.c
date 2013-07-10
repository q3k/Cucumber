// This is a frame allocator that uses bitmaps, and allocates space for these bitmaps via itself.
//
// The funny / tragic part is that we will access the bitmaps via a temp page at first. It will be slow,
// but it'll save us some mind-twisting if we would try to integrate a directory structure into it.

// TODO: actually implement being able to used chained metadata - right now we are limited to ~128M of RAM.

#include "Tier0/physmem.h"
#include "Tier0/system.h"
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"
#include "Tier0/paging.h"

struct {
    // The amount of memory in the system, or the top usable pointer.
    u64 MemorySize;
    // The first metadata structure
    u64 FirstMetadata;
    // for fast access
    u64 MemoryFree ;
} g_PhysicalMemory;

#define PHYSMEM_METADATA_BITS (64 * 511)
#define PHYSMEM_METADATA_COVERS_BYTES (PHYSMEM_METADATA_BITS * 4096)
#define PHYSMEM_METADATA_COVERS_BITS (PHYSMEM_METADATA_BITS)
#define PHYSMEM_ADDRESS_TO_BIT_NUMBER(address) ((address) / 4096)
#define PHYSMEM_BIT_NUMBER_TO_METADATA_NUMBER(bit) ((bit) / PHYSMEM_METADATA_COVERS_BITS)
#define PHYSMEM_ADDRESS_TO_METADATA_NUMBER(address) ((address) / PHYSMEM_METADATA_COVERS_BYTES)
#define PHYSMEM_ADDRESS_TO_BIT_IN_METADATA(address) ((address) % PHYSMEM_METADATA_COVERS_BYTES)
#define PHYSMEM_BIT_NUMBER_TO_BIT_IN_METADATA(bit) ((bit) % PHYSMEM_METADATA_COVERS_BITS)
#define PHYSMEM_METADATA_SET_BIT(mdp, bit) do { (mdp)->Bitmap[(bit)/64] |= (1 << ((bit)%64)); } while(0)
#define PHYSMEM_METADATA_CLEAR_BIT(mdp, bit) do { (mdp)->Bitmap[(bit)/64] &= ~(1 << ((bit)%64)); } while(0)

typedef struct {
    u64 Bitmap[511]; // covers 511*64 frames, or 130816KiB
    u64 NextMetadata; // physical address of next metadata
} __attribute__((packed)) T_PHYSMEM_METADATA; // exactly 4k in size, to fit in a frame

u64 __physmem_allocate_first_page(void)
{
    u64 NextPageStart = 0;
    
    while (!system_memory_available(NextPageStart, PHYSMEM_PAGE_SIZE))
    {
        NextPageStart += PHYSMEM_PAGE_SIZE;
        
        if (NextPageStart > g_PhysicalMemory.MemorySize)
            PANIC("Out of memory!");
    }
    kprintf("%x\n", NextPageStart);
    return NextPageStart;
}

void physmem_init(void)
{
    g_PhysicalMemory.MemorySize = system_get_memory_top();

    // allocate the first frame, for metadata
    u64 MetadataFrame = __physmem_allocate_first_page();
    kprintf("[i] Physmem: First frame @%x\n", MetadataFrame);
    kprintf("[i] Physmem: First bit: %i\n", PHYSMEM_ADDRESS_TO_BIT_NUMBER(MetadataFrame));
    if (PHYSMEM_ADDRESS_TO_METADATA_NUMBER(MetadataFrame) > 0)
        PANIC("Physmem: First allocated address > metadata covering!");

    // Let's make sure that frame is mapped into our memory...
    if (MetadataFrame >= 0xEFFFFF)
        PANIC("Physmem: first allocated address > memory mapped by loader!");
    T_PHYSMEM_METADATA *Metadata = (T_PHYSMEM_METADATA *)MetadataFrame;
    
    // zero the metadata (the 512th bit overflows into the nextmatadata field)
    for (u64 i = 0; i < 512; i++)
        Metadata->Bitmap[i] = 0;
    // mask all the bits up to and including our metadata frame as used
    kprintf("[i] Marking physical memory up to 0x%x (bit %i) as used.\n", MetadataFrame, PHYSMEM_ADDRESS_TO_BIT_NUMBER(MetadataFrame));
    for (u32 i = 0; i <= PHYSMEM_ADDRESS_TO_BIT_NUMBER(MetadataFrame); i++)
    {
        u32 Bit = PHYSMEM_BIT_NUMBER_TO_BIT_IN_METADATA(i);
        PHYSMEM_METADATA_SET_BIT(Metadata, Bit);
        g_PhysicalMemory.MemoryFree -= 4096;
    }

    // mask all the bits that are reserved accoriding to system
    for (u32 i = PHYSMEM_ADDRESS_TO_BIT_NUMBER(MetadataFrame) + 1; i < PHYSMEM_METADATA_COVERS_BITS; i ++)
    {
        u64 Address = i * 4096;
        if (!system_memory_available(Address, 4096))
        {
            PHYSMEM_METADATA_SET_BIT(Metadata, i);
            g_PhysicalMemory.MemoryFree -= 4096;
        }
    }

    g_PhysicalMemory.FirstMetadata = MetadataFrame;
}


u64 physmem_allocate_page(void)
{
    ASSERT(g_PhysicalMemory.FirstMetadata <= 0xEFFFFF);
    T_PHYSMEM_METADATA *Metadata = (T_PHYSMEM_METADATA *)g_PhysicalMemory.FirstMetadata;
    for (u32 i = 0; i < PHYSMEM_METADATA_COVERS_BITS; i++)
    {
        if (Metadata->Bitmap[i] != 0xFFFFFFFFFFFFFFFF)
        {
            // scan the subbitmap
            for (u8 j = 0; j < 64; j++)
                if (((Metadata->Bitmap[i] >> j)&1) == 0)
                {
                    PHYSMEM_METADATA_SET_BIT(Metadata, i * 64 + j);
                    g_PhysicalMemory.MemoryFree -= 4096;
                    return i * 64 + j;
                }
        }
    }
    PANIC("physmem metadata addition not yet implemented - OOM!");
    return 0;
}

void physmem_free_page(u64 Page)
{
    if (Page > PHYSMEM_METADATA_COVERS_BITS)
        PANIC("...and where did you get that page index?");

    ASSERT(g_PhysicalMemory.FirstMetadata <= 0xEFFFFF);
    T_PHYSMEM_METADATA *Metadata = (T_PHYSMEM_METADATA *)g_PhysicalMemory.FirstMetadata;

    // todo: check for double frees
    u32 Bit = PHYSMEM_BIT_NUMBER_TO_BIT_IN_METADATA(Page);
    PHYSMEM_METADATA_CLEAR_BIT(Metadata, Bit);
    g_PhysicalMemory.MemoryFree += 4096;
}

u64 physmem_page_to_physical(u64 Page)
{
    return Page * PHYSMEM_PAGE_SIZE;
}

u64 physmem_physical_to_page(u64 Physical)
{
    return Physical / PHYSMEM_PAGE_SIZE;
}

void physmem_read(u64 Base, u64 Size, void *Destination)
{
    if ((u64)Destination <= 0xEFFFFF)
    {
        for (u64 i = 0; i < Size; i++)
            ((u8 *)Destination)[i] = ((u8 *)Base)[i];
    }
    else
    {
        PANIC("physmem_read > extmem not implemented!");
    }
}

u64 physmem_get_free(void)
{
    return g_PhysicalMemory.MemoryFree;
}