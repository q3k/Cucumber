#ifndef __CPAGEDIRECTORY_H__
#define __CPAGEDIRECTORY_H__

#include "types.h"
extern "C" {
    #include "Tier0/paging.h"
};

// This is more-or less just a C++ wrapper for T_PGAE_DIRECTORY.
namespace cb {
    class CPageDirectory {
        protected:
            // The paging direcotry structure
            T_PAGING_DIRECTORY *m_Directory;
            
            // A bitmap precising whether a table is owned by the page directory
            // (== can we delete it when we delete the directory itself)
            u32 m_OwnerBitmap[32];
            
            // Check whether a table is owned by us
            bool IsTableOurs(u32 Virtual);
            
            // Creates a new table in the address space. Do not use these in
            // other directories, as they will e automatically deleted when the
            // page directory is deleted - sorry, no reference counting yet!
            void CreateTable(u32 Virtual, u8 User = 1, u8 RW = 1);
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
            
            // Copying (from any other page directory)
            void CopyTable(u32 Virtual, CPageDirectory *Source, bool Deep,
                           u8 User = 1, u8 RW = 1);
            void CopyPage(u32 Virtual, CPageDirectory *Source, u8 User = 1,
                          u8 RW = 1);
            
            // Translates a virtual adddress
            u32 Translate(u32 Virtual);
    };
};

#endif
