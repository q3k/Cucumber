#ifndef __CPAGEDIRECTORY_H__
#define __CPAGEDIRECTORY_H__

#include "types.h"
extern "C" {
    #include "Tier0/paging.h"
};

// This is more-or less just a C++ wrapper for T_PAGING_ML4.
namespace cb {
    class CKernelML4 {
        protected:
            // The paging direcotry structure
            T_PAGING_ML4 *m_Directory;
            
            // A bitmap precising whether a table is owned by the page directory
            // (== can we delete it when we delete the directory itself)
            volatile u32 m_OwnerBitmap[32];
            
            // Check whether a table is owned by us
            bool IsTableOurs(u32 Virtual);
            
            // Creates a new table in the address space. Do not use these in
            // other directories, as they will e automatically deleted when the
            // page directory is deleted - sorry, no reference counting yet!
            void CreateTable(u32 Virtual, u8 User = 1, u8 RW = 1);
            
            // Whether it was created empty
            bool m_bCreatedEmpty;
        public:
            // Creates a new page directory for a kernel task
            CPageDirectory(bool Empty = false);
            ~CPageDirectory(void);
            
            // Mapping
            void MapTable(u32 Virtual, u32 Physical, u8 User = 1, u8 RW = 1);
            void MapPage(u32 Virtual, u32 Physical, u8 User = 1, u8 RW = 1);
            void MapRange(u32 Virtual, u32 Physical, u32 Size, u8 User = 1,
                                                               u8 RW = 1);
            void UnmapPage(u32 Virtual);
            
            // Linking (usually to the kernel directory)
            void LinkTable(u32 Virtual, CPageDirectory *Source);
            
            // Copying (from any other page directory)
            void CopyTable(u32 Virtual, CPageDirectory *Source,
                           bool Deep = false, u8 User = 1, u8 RW = 1);
            void CopyPage(u32 Virtual, CPageDirectory *Destination, u8 User = 1,
                          u8 RW = 1);
            
            // Translates a virtual adddress
            u32 Translate(u32 Virtual);
    };
};

#endif
