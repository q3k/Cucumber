#include "Tier0/heap.h"
#include "Tier0/kstdlib.h"
#include "Tier0/kstdio.h"
#include "Tier0/physmem.h"
#include "Tier0/paging.h"
#include "Tier0/panic.h"

T_HEAP *g_Heap;

T_HEAP_INDEX heap_index_initialize(void *Address, u32 MaxSize)
{
    T_HEAP_INDEX Index;
    Index.Array = (void**)Address;
    Index.MaxSize = MaxSize;
    Index.Size = 0;
        
    kmemset(Address, 0, MaxSize);
    
    return Index;
}

void heap_index_insert(T_HEAP_INDEX *Index, void *Value)
{
    u64 i = 0;
    while (i < Index->Size && heap_index_smaller(Index->Array[i], Value))
        i++;

    if (i == Index->Size)
        Index->Array[Index->Size++] = Value;
    else
    {
        void *Temporary = Index->Array[i];
        Index->Array[i] = Value;
        while (i < Index->Size)
        {
            i++;
            void *Temporary2 = Index->Array[i];
            Index->Array[i] = Temporary;
            Temporary = Temporary2;
        }
        Index->Size++;
    }
}

u8 heap_index_smaller(void *A, void *B)
{
    return (((T_HEAP_HEADER *)A)->Size < ((T_HEAP_HEADER *)B)->Size)>0 ?1:0;
}

void *heap_index_get(T_HEAP_INDEX *Index, u64 Position)
{
    return Index->Array[Position];
}

void heap_index_remove(T_HEAP_INDEX *Index, u64 Position)
{
    while (Position < Index->Size)
    { Index->Array[Position] = Index->Array[Position + 1];
        Position++;
    }
    Index->Size--;
}

T_HEAP *heap_create(u64 Size)
{
    u64 NumPages = Size / (1024 * 4);
    if (Size % (1024 * 4) != 0)
        NumPages++;

    kprintf("[i] Allocating %i pages (%i KB) for heap.\n", 
            NumPages, Size / 1000);
    
    u64 Start = 0;
    for (u32 i = 0; i < NumPages; i++)
    {
        if (!Start)
            Start = (u64)paging_scratch_allocate(); 
        else
            paging_scratch_allocate();
    }
    
    T_HEAP* Heap = (T_HEAP *)Start;
    u64 DataStart = Start;
    DataStart += sizeof(T_HEAP);
    
    Heap->Index = heap_index_initialize((void*)DataStart, HEAP_INDEX_SIZE / 4);
    
    DataStart += HEAP_INDEX_SIZE;
    kprintf("[i] Heap %x - %x\n", DataStart, Start + NumPages * 4096);
    Heap->Start = DataStart;
    Heap->End = Start + NumPages * 4096;
    
    T_HEAP_HEADER *Hole = (T_HEAP_HEADER *)DataStart;
    Hole->Size = Heap->End - Heap->Start;
    Hole->Magic = HEAP_HEADER_MAGIC;
    Hole->Hole = 1;
    
    heap_index_insert(&Heap->Index, (void*)Hole);
    return Heap;
}

s64 _heap_find_smallest_hole(T_HEAP *Heap, u64 Size, u8 Aligned)
{
    u64 Iterator = 0;
    while (Iterator < Heap->Index.Size)
    {
        T_HEAP_HEADER *Header = heap_index_get(&Heap->Index, Iterator);
        if (Aligned > 0)
        {
            u64 Location = (u64)Header;
            u64 Offset = 0;

            if (((Location + sizeof(T_HEAP_HEADER)) & 0xFFFFF000) != 0)
                Offset = 0x1000 - (Location + sizeof(T_HEAP_HEADER)) % 0x1000;
            
            u64 HoleSize = (u64)Header->Size - Offset;

            if (HoleSize >= Size)
                break;
        }
        else if (Header->Size >= Size)
            break;
        Iterator++;
    }

    if (Iterator == Heap->Index.Size)
        return -1;
    else
        return Iterator;
}

void _heap_expand(T_HEAP *Heap, u64 Size)
{
    /*u32 NumPages = Size / 0x1000; 
    if (Size % 0x1000 != 0)
        NumPages++;

    for (u64 i = 0; i < NumPages; i++)
    {
        u64 Page = physmem_allocate_page();
        u64 Physical = physmem_page_to_physical(Page);
        paging_map_kernel_page(Heap->End + i * 0x1000, Physical);
    }
    
    Heap->End = Heap->Start + NumPages * 0x1000;*/
    PANIC("Heap expansion not implemented.");
}

u32 _heap_contract(T_HEAP *Heap, u64 Size)
{
    /*if (Size & 0x1000)
    {
        Size &= 0x1000;
        Size += 0x1000;
    }

    if (Size < HEAP_MIN_SIZE)
        Size = HEAP_MIN_SIZE;
    
    u64 OldSize = Heap->End - Heap->Start;
    u64 NumberToDelete = (OldSize - Size) / 0x1000;

    for (int i = 0; i < NumberToDelete; i++)
    {
        u64 Virtual = Heap->End + i * 0x1000;
        u64 Physical;
        paging_get_physical(Virtual, &Physical);
        u64 Page = Physical / 0x1000;
        physmem_free_page(Page);
    }

    Heap->End -= NumberToDelete * 0x1000;*/

    // TODO: Implement this?
    return (Heap->End - Heap->Start);
}

/*void *heap_alloc_p(T_HEAP *Heap, u64 Size, u8 Aligned, u64 *Physical)
{
    void *Address = heap_alloc(Heap, Size, Aligned);
    
    if (Physical != 0)
        paging_get_physical((u64)Address, Physical);
    
    return Address;
}*/

void *heap_alloc(T_HEAP *Heap, u64 Size, u8 Aligned)
{
    u64 RealSize = Size + sizeof(T_HEAP_HEADER) + sizeof(T_HEAP_FOOTER);
    s64 Iterator = _heap_find_smallest_hole(Heap, RealSize, Aligned);
    if (Iterator == -1)
    {
        u64 OldSize = Heap->End - Heap->Start;
        u64 OldEnd = Heap->End;
        _heap_expand(Heap, OldSize + RealSize);
        u64 NewSize = Heap->End - Heap->Start;

        Iterator = 0;
        u64 Last = 0;
        s64 LastIndex = -1;

        while (Iterator < Heap->Index.Size)
        {
            u64 Location = (u64)heap_index_get(&Heap->Index, Iterator);
            if (Location > Last)
            {
                Last = Location;
                LastIndex = Iterator;
            }
            Iterator++;
        }

        if (LastIndex == -1)
        {
            T_HEAP_HEADER *Header = (T_HEAP_HEADER *)OldEnd;
            Header->Magic = HEAP_HEADER_MAGIC;
            Header->Size = NewSize - OldSize;
            Header->Hole = 1;

            T_HEAP_FOOTER *Footer = (T_HEAP_FOOTER *)(OldEnd + Header->Size
                                    - sizeof(T_HEAP_FOOTER));
            Footer->Magic = HEAP_FOOTER_MAGIC;
            Footer->Header = Header;

            heap_index_insert(&Heap->Index, (void*)Header);
        }
        else
        {
            T_HEAP_HEADER *Header = (T_HEAP_HEADER *)Last;
            Header->Size += NewSize - OldSize;

            T_HEAP_FOOTER *Footer = (T_HEAP_FOOTER *)((u64)Header
                                    + Header->Size - sizeof(T_HEAP_FOOTER));
            Footer->Header = Header;
            Footer->Magic = HEAP_FOOTER_MAGIC;
        }

        return heap_alloc(Heap, Size, Aligned);
    }

    T_HEAP_HEADER *Header = (T_HEAP_HEADER*)heap_index_get(&Heap->Index, 
                            Iterator);
    u64 HoleStart = (u64)Header;
    u64 HoleSize = Header->Size;

    if (HoleSize - RealSize < sizeof(T_HEAP_HEADER) + sizeof(T_HEAP_FOOTER))
    {
        Size += (HoleSize - RealSize);
        RealSize = HoleSize;
    }

    if (Aligned && HoleStart & 0xFFFFFFFFFFFFF000)
    {
        u64 NewLocation = HoleStart + 0x1000 - (HoleStart & 0xFFF)
                          - sizeof(T_HEAP_HEADER);
        Header->Size = 0x1000 - (HoleStart & 0xFFF) - sizeof(T_HEAP_HEADER);
        Header->Magic = HEAP_HEADER_MAGIC;
        Header->Hole = 1;

        T_HEAP_FOOTER *Footer = (T_HEAP_FOOTER*)(NewLocation
                                 - sizeof(T_HEAP_FOOTER));
        Footer->Magic = HEAP_FOOTER_MAGIC;
        Footer->Header = Header;

        HoleStart = NewLocation;
        HoleSize -= Header->Size;
    }
    else
        heap_index_remove(&Heap->Index, Iterator);

    T_HEAP_HEADER *BlockHeader = (T_HEAP_HEADER *)HoleStart;
    BlockHeader->Magic = HEAP_HEADER_MAGIC;
    BlockHeader->Size = RealSize;
    BlockHeader->Hole = 0;

    T_HEAP_FOOTER *BlockFooter = (T_HEAP_FOOTER *)(HoleStart + RealSize
                                - sizeof(T_HEAP_FOOTER));
    BlockFooter->Magic = HEAP_FOOTER_MAGIC;
    BlockFooter->Header = BlockHeader;

    if (HoleSize - RealSize > 0)
    {
        T_HEAP_HEADER *NewHoleHeader = (T_HEAP_HEADER*)(HoleStart + RealSize);
        NewHoleHeader->Magic = HEAP_HEADER_MAGIC;
        NewHoleHeader->Size = HoleSize - RealSize;
        NewHoleHeader->Hole = 1;

        T_HEAP_FOOTER *NewHoleFooter = (T_HEAP_FOOTER*)((u64)NewHoleHeader
            + NewHoleHeader->Size - sizeof(T_HEAP_FOOTER));
//for (;;) {}
        if ((u64)NewHoleFooter < Heap->End)
        {
            NewHoleFooter->Magic = HEAP_FOOTER_MAGIC;
            NewHoleFooter->Header = NewHoleHeader;
        }

        heap_index_insert(&Heap->Index, (void*)NewHoleHeader);
    }

    return (void *)((u64)BlockHeader + sizeof(T_HEAP_HEADER));
}

void heap_free(T_HEAP *Heap, void *Data)
{
    if (Data == 0) return;
    T_HEAP_HEADER *Header = (T_HEAP_HEADER *)((u64)Data
                             - sizeof(T_HEAP_HEADER));
    T_HEAP_FOOTER *Footer = (T_HEAP_FOOTER *)((u64)Header + Header->Size
                             - sizeof(T_HEAP_FOOTER));

    if (Header->Magic != HEAP_HEADER_MAGIC)
        return;
    if (Footer->Magic != HEAP_FOOTER_MAGIC)
        return;
    Header->Hole = 1;

    u8 ShouldAdd = 1;

    T_HEAP_FOOTER *FooterLeft = (T_HEAP_FOOTER *)((u64)Data
                                - sizeof(T_HEAP_FOOTER));
    if (FooterLeft->Magic == HEAP_FOOTER_MAGIC && FooterLeft->Header->Hole)
    {
        u64 OurSize = Header->Size;
        Header = FooterLeft->Header;
        Footer->Header = Header;
        Header->Size += OurSize;
        ShouldAdd = 0;
    }

    T_HEAP_HEADER *HeaderRight = (T_HEAP_HEADER *)((u64)Footer
                                 + sizeof(T_HEAP_FOOTER));
    if (HeaderRight->Magic == HEAP_HEADER_MAGIC && HeaderRight->Hole)
    {
        Header->Size += HeaderRight->Size;
        Footer = (T_HEAP_FOOTER *)((u64)HeaderRight + HeaderRight->Size
                  - sizeof(T_HEAP_FOOTER));
        Footer->Header = Header;
        
        u64 Iterator = 0;
        while (Iterator < Heap->Index.Size &&
               heap_index_get(&Heap->Index, Iterator) != (void *)HeaderRight)
            Iterator++;

        if (Iterator != -1)
            heap_index_remove(&Heap->Index, Iterator);
    }

    if ((u64)Footer + sizeof(T_HEAP_FOOTER) == Heap->End)
    {
        u64 OldSize = Heap->End - Heap->Start;
        u64 NewSize = _heap_contract(Heap, (u64)Header - Heap->Start);

        if (Header->Size > (OldSize - NewSize))
        {
            Header->Size -= (OldSize - NewSize);
            Footer = (T_HEAP_FOOTER *)((u64)Header + Header->Size
                      - sizeof(T_HEAP_FOOTER));
            Footer->Magic = HEAP_FOOTER_MAGIC;
            Footer->Header = Header;
        }
        else
        {
            u64 Iterator = 0;
            while (Iterator < Heap->Index.Size &&
                   heap_index_get(&Heap->Index, Iterator) != (void *)Header)
                Iterator++;

            if (Iterator < Heap->Index.Size)
            {
                heap_index_remove(&Heap->Index, Iterator);
                ShouldAdd = 0;
            }
        }
    }

    if (ShouldAdd)
        heap_index_insert(&Heap->Index, (void *)Header);
}

void heap_init_simple(void)
{
    g_Heap = heap_create(HEAP_INITIAL_SIZE);
}

void *kmalloc(u64 Size)
{
    return heap_alloc(g_Heap, Size, 0);
}

/*void *kmalloc_p(u64 Size, u8 Aligned, u64 *Physical)
{
    return heap_alloc_p(g_Heap, Size, Aligned, Physical);
}*/

void kfree(void *Data)
{
    heap_free(g_Heap, Data);
}
