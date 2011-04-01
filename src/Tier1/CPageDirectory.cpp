#include "Tier1/CPageDirectory.h"
using namespace cb;

extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/kstdlib.h"
};

CPageDirectory::CPageDirectory(void)
{
    m_OwnerBitmap = 0;
    
    u32 PhysicalDirectory;
    m_Directory = (T_PAGING_DIRECTORY *)kmalloc_p(sizeof(T_PAGING_DIRECTORY),
        1, &PhysicalDirectory);
    
    m_Directory->PhysicalAddress = PhysicalDirectory;
    
    
}

CPageDirectory::~CPageDirectory(void)
{
    for (u16 Table = 0; Table < 1024; Table++)
    {
        u8 Index = Table / 32;
        u8 Offset = Table % 32;
        
        u8 Owned = (m_OwnerBitmap[Index] & (1 << Ofset)) > 0;
        if (Owned)
        {
            u8 *TableMemory = (u8*)m_Directory->Tables[Table];
            kfree(TableMemory);
        }
    }
    
    kfree(m_Directory);
}

void CPageDirectory::CreateTable(u32 Virtual, u8 User = 1, u8 RW = 1)
{
    u16 TableIndex = Virtual / (4 * 1024 * 1024);
    u8 Index = TableIndex / 32;
    u8 Offset = TableIndex % 32;
    
    m_OwnerBitmap[Index] |= (1 << Offset);
    
    u32 TablePhysical;
    T_PAGING_TABLE *TableVirtual = (T_PAGING_TABLE*)kmalloc_p(
        sizeof(T_PAGING_TABLE), 1, &TablePhysical);
    m_Directory->Tables[TableIndex] = TableVirtual;
    
    kmemsetw((void*)TableVirtual, 0, 1024);
    
    // Prepare the entry
    u32 Entry = TablePhysical & (0xFFC00000);
    Entry |= 1; //Present
    Entry |= (RW << 1);
    Entry |= (User << 1);
    
    m_Directory->Entries[TableIndex] = Entry;
}

void CPageDirectory::MapTable(u32 Virtual, u32 Physical, u8 User, u8 RW)
{
    for (u16 i = 0; i < 1024; i++)
        MapPage(Virtual + i * 4096; Physical + i * 4096; User, RW);
}

void CPageDirectory::MapPage(u32 Virtual, u32 Physical, u8 User, u8 RW)
{
    u16 DirectoryIndex = (Virtual >> 22) & 0x3FF;
    u32 Entry = m_Directory->Entries[DirectoryIndex];
    
    u8 TablePresent = (Entry & 0x01) > 0;
    
    if (!TablePresent)
        CreateTable(Virtual & 0xFFC00000);
    
    T_PAGING_TABLE *Table = Directory->Tables[DirectoryIndex];
    
    u16 TableIndex = (Virtual >> 12) & 0x3FF;
    
    T_PAGING_PAGE *Page = &Table->Pages[TableIndex];
    
    *((u32*)Page) = 1;
    Page->User = User;
    Page->RW = RW;
    Page->Physical = (Physical & 0xFFFFF000) >> 12;
    
    // Flush the TLB
    __asm__ volatile("invlpg %0" :: "m" (Virtual));
}
