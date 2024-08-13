#ifndef TARINITRD_H
#define TARINITRD_H

#include "fs/vfs.h"
#include "types.h"
#include "libc/string.h"
#include "common/hashmap.h"
#include "common/doublyLinkedList.h"

namespace Filesystem {

    class TarReader : VFSDriver
    {
        class TarHeader
        {
        private:
            char m_filename[100];
            char m_mode[8];
            char m_uid[8];
            char m_gid[8];
            char m_size[12];
            char m_mtime[12];
            char m_chksum[8];
            char m_typeflag[1];
            char m_reserved[355];
        public:
            char* get_filename() { return m_filename; }
            void set_filename(char* filename) { strcpy(m_filename, filename); }
            uint64_t get_size()
            {
                uint64_t size = 0;
                unsigned int i;
                uint64_t count = 1;
            
                for (i = 11; i > 0; i--, count *= 8)
                    size += ((m_size[i - 1] - '0') * count);
            
                return size; 
            }
            TarHeader* get_next_header()
            {
                auto header = this;
                header++;
                auto pointer = reinterpret_cast<uint8_t*>(header);
                pointer += BYTE_ALIGN_UP(get_size(), TAR_ALIGN);
                return reinterpret_cast<TarHeader*>(pointer);
            }
        } __attribute__((packed));

        struct TarDirectoryEntry
        {
            char filename[256];
            uint64_t inodeNum;
        };
}

#endif
