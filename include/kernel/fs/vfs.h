#ifndef VFS_H
#define VFS_H
#include "types.h"
#include "common/doublyLinkedList.h"
#include "common/hashmap.h"
#include "libc/string.h"

namespace Filesystem
{

    struct VirtualFilesystemNode;
    struct VirtualFilesystemDirectoryEntry;

    class VFSDriver
    {
    public: 
        virtual uint64_t vfsdriver_read(VirtualFilesystemNode*, uint64_t, uint64_t, char*) = 0;
        virtual uint64_t vfsdriver_write(VirtualFilesystemNode*, uint64_t, uint64_t, char*) = 0;
        virtual void vfsdriver_open(VirtualFilesystemNode*, uint64_t) = 0;
        virtual void vfsdriver_close(VirtualFilesystemNode*) = 0;
        virtual VirtualFilesystemDirectoryEntry* vfsdriver_readdir(VirtualFilesystemNode*, uint64_t index) = 0;
        virtual VirtualFilesystemNode* vfsdriver_finddir(VirtualFilesystemNode*, char* name) = 0;
        virtual uint64_t vfsdriver_filelen(VirtualFilesystemNode*) = 0;
    };

    struct VirtualFilesystemNode {
        uint64_t inodeNum;
        char name[256];

        void* fs;
        
        enum Flag
        {
            VFS_FILE = 0x01,
            VFS_DIRECTORY = 0x02
        };

        uint64_t flags = 0;
        uint64_t refcount = 0;
        
        uint64_t creationTime = 0;
        uint64_t modificationTime = 0;

        VFSDriver* driver;
    };

    struct VirtualFilesystemDirectoryEntry {
        uint64_t inodeNum;
        char name[256];
    };

    struct taskfd
    {
        uint64_t fd;
        uint64_t offset;
        Filesystem::VirtualFilesystemNode* node;
    };
}

#endif
