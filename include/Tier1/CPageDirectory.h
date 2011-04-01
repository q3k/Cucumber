#ifndef __CPAGEDIRECTORY_H__
#define __CPAGEDIRECTORY_H__

#include "types.h"
extern "C" {
    #include "Tier0/paging.h"
};

// This is more-or less just a C++ wrapper for T_PGAE_DIRECTORY.
namespace cb {
    class CPageDirectory {
        private:
            // The paging direcotry structure
            T_PAGING_DIRECTORY *m_Directory = 0;
            
            // A bitmap preising whether a table is owned by this page directory
            // (== can we delete it when we delete the directory itself)
            u32 m_OwnerBitmap;
            
            // Creates a new table in the address space. Do not use these in
            // other directories, as they will e automatically deleted when the
            // page directory is deleted - sorry, no reference counting yet!
            void CreateTable(u32 Virtual);
        public:
            // Creates a new page directory for a kernel task
            CPageDirectory(void);
            ~CPageDirectory(void);
            
            // Mapping
            void MapTable(u32 Virtual, u32 Physical, u8 User = 1, u8 RW = 1);
            void MapPage(u32 Virtual, u32 Physical, u8 User = 1, u8 RW = 1);
            void MapRange(u32 Virtual, u32 Physical, u32 Size, u8 User = 1,
                                                               u8 RW = 1);
            
            // Linking (usually to the kernel directory)
            void LinkTable(u32 Virtual, CPageDirectory *Source);
            void LinkPage(u32 Virtual, CPageDirectory *Source);
            void LinkRange(u32 Virtual, CPageDirectory *SOurce, u32 Size);
            
            // Copying (from any other page directory)
            void CopyTable(u32 Virtual, CPageDirectory *Source);
            void CopyPage(u32 Virtual, CPageDirectory *Source);
            void CopyRange(u32 Virtual, CPageDirectory *Source);
    };
};

#endif
