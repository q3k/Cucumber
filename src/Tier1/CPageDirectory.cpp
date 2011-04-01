#include "Tier1/CPageDirectory.h"
using namespace cb;

extern "C" {
    #include "Tier0/heap.h"
    #include "Tier0/kstdlib.h"
    #include "Tier0/panic.h"
};

CPageDirectory::CPageDirectory(void)
{
    for (u8 i = 0; i < 32; i++)
        m_OwnerBitmap[i] = 0;
    
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
        
        u8 Owned = (m_OwnerBitmap[Index] & (1 << Offset)) > 0;
        if (Owned)
        {
            u8 *TableMemory = (u8*)m_Directory->Tables[Table];
            kfree(TableMemory);
        }
    }
    
    kfree(m_Directory);
}

void CPageDirectory::CreateTable(u32 Virtual, u8 User, u8 RW)
{
    u16 TableIndex = Virtual / 0x400000;
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

bool CPageDirectory::IsTableOurs(u32 Virtual)
{
    u16 TableIndex = Virtual / 0x400000;
    u8 Index = TableIndex / 32;
    u8 Offset = TableIndex % 32;
    
    return (m_OwnerBitmap[Index] & (1 << Offset)) > 0;
}

void CPageDirectory::MapTable(u32 Virtual, u32 Physical, u8 User, u8 RW)
{
    for (u16 i = 0; i < 1024; i++)
        MapPage(Virtual + i * 0x1000, Physical + i * 0x1000, User, RW);
}

void CPageDirectory::MapPage(u32 Virtual, u32 Physical, u8 User, u8 RW)
{
    u16 DirectoryIndex = (Virtual >> 22) & 0x3FF;
    u32 Entry = m_Directory->Entries[DirectoryIndex];
    
    u8 TablePresent = (Entry & 0x01) > 0;
    
    if (!TablePresent)
        CreateTable(Virtual & 0xFFC00000, User, RW);
    else if (!IsTableOurs(Virtual & 0xFFC00000))
        CopyTable(Virtual & 0xFFC00000, this, false, User, RW);
    
    T_PAGING_TABLE *Table = m_Directory->Tables[DirectoryIndex];
    
    u16 TableIndex = (Virtual >> 12) & 0x3FF;
    
    T_PAGING_PAGE *Page = &Table->Pages[TableIndex];
    
    *((u32*)Page) = 1;
    Page->User = User;
    Page->RW = RW;
    Page->Physical = (Physical & 0xFFFFF000) >> 12;
    
    // Flush the TLB
    __asm__ volatile("invlpg %0" :: "m" (Virtual));
}

void CPageDirectory::MapRange(u32 Virtual, u32 Physical, u32 Size, u8 User,
    u8 RW)
{
    // Align start address if needed
    if ((Virtual | 0xFFFFF000) > 0)
    {
        Size += (Virtual - (Virtual | 0xFFFFF000));
        Virtual |= 0xFFFFF000;
    }
    
    // Align size if needed
    if ((Size % 0x1000) > 0)
    {
        Size |= 0x1000;
        Size += 0x1000;
    }
    
    // Map 'em!
    u16 NumPages = Size / 0x1000;
    for (u32 i = 0; i < NumPages; i++)
        MapTable(Virtual + i * 0x1000, Physical + i * 0x1000, User, RW);
}

void CPageDirectory::LinkTable(u32 Virtual, CPageDirectory *Source)
{
    u16 TableIndex = Virtual / 0x400000;
    T_PAGING_TABLE *Table = Source->m_Directory->Tables[TableIndex];
    m_Directory->Tables[TableIndex] = Table;
}

void CPageDirectory::CopyTable(u32 Virtual, CPageDirectory *Source, bool Deep,
    u8 User, u8 RW)
{
    // Align start address if needed
    if ((Virtual | 0xFFFFF000) > 0)
        Virtual |= 0xFFFFF000;
    
    if (Deep)
    {
        for (u16 i = 0; i < 1024; i++)
            CopyPage(Virtual + i * 0x1000, Source);
    }
    else
    {
        CreateTable(Virtual, User, RW);
        u16 TableIndex = (Virtual >> 22) & 0x3FF;
        
        u8 *SourceTable = (u8*)Source->m_Directory->Tables[TableIndex];
        u8 *DestinationTable = (u8*)m_Directory->Tables[TableIndex];
        
        kmemcpy(DestinationTable, SourceTable, sizeof(T_PAGING_TABLE));
    }
}

void CPageDirectory::CopyPage(u32 Virtual, CPageDirectory *Source, u8 User,
                              u8 RW)
{
    // Our directory
    u16 DirectoryIndex = (Virtual >> 22) & 0x3FF;
    u32 Entry = m_Directory->Entries[DirectoryIndex];
    u8 TablePresent = (Entry & 0x01) > 0;
    
    if (!TablePresent)
        CreateTable(Virtual & 0xFFC00000, User, RW);
    else if (!IsTableOurs(Virtual & 0xFFC00000))
        CopyTable(Virtual & 0xFFC00000, this, false, User, RW);
    
    // Foreign directory
    Entry = Source->m_Directory->Entries[DirectoryIndex];
    TablePresent = (Entry & 0x01) > 0;
    if (!TablePresent)
        PANIC("Copying unexisting table!");
        
    T_PAGING_TABLE *Table = Source->m_Directory->Tables[DirectoryIndex];
    u16 TableIndex = (Virtual >> 12) & 0x3FF;
    
    if (!Table->Pages[TableIndex].Present)
        PANIC("Copying unexisting page!");
    
    u32 Page = *((u32*)&Table->Pages[TableIndex]);
    
    // Our directory again
    Table = m_Directory->Tables[DirectoryIndex];
    *((u32*)&Table->Pages[TableIndex]) = Page;
}

u32 CPageDirectory::Translate(u32 Virtual)
{
    u32 Physical;
    u8 Result =  paging_get_physical_ex(Virtual, &Physical, m_Directory);
    ASSERT(Result);
    
    return Physical;
}
