// Kinda like a heap, but not really.
// (the following sizes are for x86)
// Based on a very large number of 4k pages. They are kept in a linear list
// of 512 64-bit (4kbyte) bitmaps. If there aren't any free pages in the
// bitmap, oh we want to mark a page as reserved further down in the list,
// then we create a new one and link the together. Freeing pages does not
// (yet?) free bitmaps. We always reserve one page for extending the list.

#include "Tier0/physical_alloc.h"
#include "Tier0/kstdio.h"
#include "Tier0/panic.h"

T_PHYSALLOC_NODE g_physalloc_root_node;
T_PHYSALLOC_NODE *g_physalloc_top_node;
u64 g_physalloc_list_size = 1;
u64 g_physalloc_space_for_next = 0;
u64 g_physalloc_mem_max = 0;

void physmem_zero_node(T_PHYSALLOC_NODE *Node)
{
	for (u64 i = 0; i < PHYSALLOC_NUM_BITMAPS; i++)
		Node->Bitmaps[i] = 0;

	Node->Next = 0;
}

void physmem_init(u64 MemorySize)
{
    // Create the first node
	physmem_zero_node(&g_physalloc_root_node);
	g_physalloc_top_node = &g_physalloc_root_node;
	g_physalloc_mem_max = MemorySize;
}

void physmem_create_node(void)
{
	if (!g_physalloc_space_for_next)
		PANIC("No space for next physmem node! :o");

	T_PHYSALLOC_NODE *NewNode = (T_PHYSALLOC_NODE *)g_physalloc_space_for_next;
	physmem_zero_node(NewNode);

	g_physalloc_top_node->Next = NewNode;
	g_physalloc_top_node = NewNode;

	g_physalloc_space_for_next = physmem_allocate_page();
	g_physalloc_list_size++;
}

T_PHYSALLOC_NODE *physmem_traverse_list(u64 Index)
{
	if (Index > g_physalloc_list_size)
		PANIC("Tried to traverse list too far!");

	u32 Current = 0;
	T_PHYSALLOC_NODE *Node = &g_physalloc_root_node;

	while (Current < Index)
		Node = Node->Next;
		Current++;

	return Node;
}

void physmem_set_node_space(u64 Space)
{
	g_physalloc_space_for_next = Space;
}

void physmem_mark_as_used(u64 Page)
{
	T_PHYSALLOC_NODE *Node;

    if (g_physalloc_list_size * PHYSALLOC_NUM_BITMAPS * 64 >= Page)
    	Node = physmem_traverse_list(Page / (PHYSALLOC_NUM_BITMAPS * 64));
    else
    {
    	while (g_physalloc_list_size * PHYSALLOC_NUM_BITMAPS * 64 < Page)
    	{
    		physmem_create_node();
    	}
    	Node = g_physalloc_top_node;
    }

    u16 OffsetInBitmaps = Page / (PHYSALLOC_NUM_BITMAPS * 64);
    u8 OffsetInBitmap = Page % 64;

    //kprintf("p: marking %i (%i %i)\n", Page, OffsetInBitmaps, OffsetInBitmap);

    u64 Bitmap = Node->Bitmaps[OffsetInBitmaps];
    Bitmap |= (1 << OffsetInBitmap);
    Node->Bitmaps[OffsetInBitmaps] = Bitmap;
}

u64 physmem_allocate_page(void)
{
	T_PHYSALLOC_NODE *Node = &g_physalloc_root_node;
	for (u64 nNode = 0; nNode < g_physalloc_list_size; nNode++)
	{
		for (u16 nBitmap = 0; nBitmap < PHYSALLOC_NUM_BITMAPS; nBitmap++)
		{
			u64 Bitmap = Node->Bitmaps[nBitmap];
			if (Bitmap != 0xFFFFFFFFFFFFFFFF)
			{
				for (u8 nBit = 0; nBit < 64; nBit++)
				{
					if ((Bitmap & (1 << nBit)) == 0)
					{
						Bitmap |= (1 << nBit);
						Node->Bitmaps[nBitmap] = Bitmap;
						kprintf("physmem: allocated %i\n", nNode * PHYSALLOC_NUM_BITMAPS + nBitmap * 64 + nBit);
						return nNode * PHYSALLOC_NUM_BITMAPS + nBitmap * 64 + nBit;
					}
				}
			}
		}
		Node = Node->Next;
	}

	// Still no space? Create a new node, if there is still space left in the memory
	if (g_physalloc_mem_max && ((g_physalloc_list_size + 1) * PHYSALLOC_NUM_BITMAPS * 64 * PHYSALLOC_PAGE_SIZE > g_physalloc_mem_max))
	{
		kprintf("physmem: extending\n");
		physmem_create_node();
		Node = g_physalloc_top_node;
		Node->Bitmaps[0] = 0x1;
		return ((g_physalloc_list_size - 1) * PHYSALLOC_NUM_BITMAPS * 64) + 1;
	}

	PANIC("Out of memory!");
	return 0;
}

void physmem_free_page(u64 Page)
{
	T_PHYSALLOC_NODE *Node = physmem_traverse_list(Page / (PHYSALLOC_NUM_BITMAPS * 64));
	u16 OffsetInBitmaps = Page % (PHYSALLOC_NUM_BITMAPS * 64);
	u8 OffsetInBitmap = Page % 64;

	u64 Bitmap = Node->Bitmaps[OffsetInBitmaps];
	Bitmap ^= (1 << OffsetInBitmap);
	Node->Bitmaps[OffsetInBitmaps] = Bitmap;
}

u64 physmem_page_to_physical(u64 Page)
{
	return Page * PHYSALLOC_PAGE_SIZE;
}

u64 physmem_physical_to_page(u64 Physical)
{
	return Physical / PHYSALLOC_PAGE_SIZE;
}
