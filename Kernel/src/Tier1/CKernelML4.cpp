#include "Tier1/CKernelML4.h"
using namespace cb;

extern "C"
{
    #include "Tier0/paging.h"
    #include "Tier0/heap.h"
    #include "Tier0/system.h"
    #include "Tier0/panic.h"
    #include "Tier0/kstdio.h"
    #include "Tier0/kstdlib.h"
}

#define ASSERT_ALIGNED(m) ASSERT(!(((u64)m) & 0xFFF))
#define POPULATE_PAGING_ENTRY(Entry, address) do { Entry.Present = 0;\
    Entry.RW = 0; \
    Entry.User = 0; \
    Entry.Misc = 0; \
    Entry.Zero = 0; \
    Entry.Physical = address >> 12; } while(0)

#define align_new(Memory, Type, ...) do { Memory = (Type *)kmalloc_aligned_physical(sizeof(Type)); new(Memory) Type(__VA_ARGS__); } while(0)

CKernelML4::CKernelML4(bool AllocateStack)
{
    m_Running = false;
    // Allocate directory
    align_new(m_ML4, CPagingStructure<T_PAGING_ML4_ENTRY>);

    // Map LOWMEM (TODO: share this, and actually limit this to RAM size)
    Map(AREA_LOWMEM_START, 0x0, AREA_LOWMEM_SIZE);

    // Map the SCRATCH directory from Tier0
    u64 ScratchDirectory = paging_get_scratch_directory();
    u64 ScratchVirtual = 0xFFFFFFFF00000000;
    u64 ScratchPML4I = PAGING_GET_PML4_INDEX(ScratchVirtual);
    u64 ScratchPDPI = PAGING_GET_PDP_INDEX(ScratchVirtual);
    CPagingStructure<T_PAGING_PDP_ENTRY> *ScratchPDP;
    m_ML4->GetOrNewEntry(ScratchPML4I, &ScratchPDP);
    kprintf("[i] Setting SCRATCH directory to %X\n", ScratchDirectory);
    ScratchPDP->SetEntry(ScratchPDPI, ScratchDirectory);

    if (AllocateStack) {
        // Allocate STACK
        //TODO: unhardcode this
        u64 StackSize = 1 * 1024 * 1024;
        u64 StackStart = (u64)kmalloc_aligned_physical(StackSize);
        Map(AREA_STACK_START, StackStart, StackSize);
    }

    // Map the TEXT directory from Tier0
    u64 TextDirectory = paging_get_text_directory();
    u64 TextVirtual = 0xFFFFFFFF80000000;
    u64 TextPML4I = PAGING_GET_PML4_INDEX(TextVirtual);
    u64 TextPDPI = PAGING_GET_PDP_INDEX(TextVirtual);
    CPagingStructure<T_PAGING_PDP_ENTRY> *TextPDP;
    m_ML4->GetEntry(TextPML4I, &TextPDP);
    kprintf("[i] Setting TEXT directory to %X\n", TextDirectory);
    TextPDP->SetEntry(TextPDPI, TextDirectory);
}

u64 CKernelML4::Resolve(u64 Virtual)
{
    u64 PML4I = PAGING_GET_PML4_INDEX(Virtual);
    u64 PDPI = PAGING_GET_PDP_INDEX(Virtual);
    u64 DIRI = PAGING_GET_DIR_INDEX(Virtual);
    u64 TABI = PAGING_GET_TAB_INDEX(Virtual);
    
    CPagingStructure<T_PAGING_PDP_ENTRY> *PDP;
    CPagingStructure<T_PAGING_DIR_ENTRY> *Dir;
    CPagingStructure<T_PAGING_TAB_ENTRY> *Tab;

    if (!m_ML4->IsEntryPresent(PML4I))
        PANIC("No such PML4I");
    m_ML4->GetEntry(PML4I, &PDP);
    if (!PDP->IsEntryPresent(PDPI))
        PANIC("No such PDPI");
    PDP->GetEntry(PDPI, &Dir);
    if (!Dir->IsEntryPresent(DIRI))
        PANIC("No such DIRI");
    Dir->GetEntry(DIRI, &Tab);
    if (!Tab->IsEntryPresent(TABI))
        PANIC("No such TABI");
    return Tab->GetEntryPhysical(TABI);
}

void CKernelML4::Apply(void)
{
    u64 ML4 = (u64)m_ML4;
    __asm__ volatile("movq %%rax, %%cr3" :: "a" (ML4));
}

void CKernelML4::Dump(void)
{
    T_PAGING_ML4 *ML4 = (T_PAGING_ML4 *)m_ML4->m_Entries;
    // We actually do this as bare structures to simulate processor behaviour as close as possible
    for (u64 ML4I = 0; ML4I < 512; ML4I++) {
        if (!ML4->Entries[ML4I].Present) {
            continue;
        }
        kprintf("ML4: %X -> %X\n", ML4I << 39, ML4->Entries[ML4I].Physical<<12);
        T_PAGING_PDP *PDP = (T_PAGING_PDP *)(ML4->Entries[ML4I].Physical<<12);
        for (u64 PDPI = 0; PDPI < 512; PDPI++) {
            if (!PDP->Entries[PDPI].Present) {
                continue;
            }
            kprintf(" PDP: %X -> %X\n", (ML4I<<39)|(PDPI<<30), PDP->Entries[PDPI].Physical<<12);
            T_PAGING_DIR *Dir = (T_PAGING_DIR *)(PDP->Entries[PDPI].Physical<<12);
            for (u64 DirI = 0; DirI < 512; DirI++) {
                if (!Dir->Entries[DirI].Present) {
                    continue;
                }
                kprintf("  PDP: %X -> %X\n", (ML4I<<39)|(PDPI<<30)|(DirI<<21), Dir->Entries[DirI].Physical<<12);
                T_PAGING_TAB *Tab = (T_PAGING_TAB *)(Dir->Entries[DirI].Physical<<12);
                for (u64 TabI = 0; TabI < 512; TabI++) {
                    if (!Tab->Entries[TabI].Present) {
                        continue;
                    }
                    kprintf("  Tab: %X -> %X\n", (ML4I<<39)|(PDPI<<30)|(DirI<<21)|(TabI<<12), Tab->Entries[TabI].Physical<<12);
                }
            }
        }
    }
}

void CKernelML4::Map(u64 Virtual, u64 Physical)
{
    ASSERT_ALIGNED(Virtual);
    ASSERT_ALIGNED(Physical);

    u64 PML4I = PAGING_GET_PML4_INDEX(Virtual);
    u64 PDPI = PAGING_GET_PDP_INDEX(Virtual);
    u64 DIRI = PAGING_GET_DIR_INDEX(Virtual);
    u64 TABI = PAGING_GET_TAB_INDEX(Virtual);

    CPagingStructure<T_PAGING_PDP_ENTRY> *PDP;
    CPagingStructure<T_PAGING_DIR_ENTRY> *Dir;
    CPagingStructure<T_PAGING_TAB_ENTRY> *Tab;

    m_ML4->GetOrNewEntry(PML4I, &PDP);
    PDP->GetOrNewEntry(PDPI, &Dir);
    Dir->GetOrNewEntry(DIRI, &Tab);
    Tab->SetEntry(TABI, Physical);
}

void CKernelML4::Map(u64 Virtual, u64 Physical, u64 Size)
{
    ASSERT_ALIGNED(Virtual);
    ASSERT_ALIGNED(Physical);
    ASSERT_ALIGNED(Size);

    for (u64 i = 0; i < Size/0x1000; i++)
        Map(Virtual+(i*0x1000), Physical+(i*0x1000));
}
