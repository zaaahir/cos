#include "fs/vfs.h"

namespace Filesystem
{

    int VirtualFilesystemManager::register_filesystem(RealFilesystem filesystem)
    {
        filesystem.m_mountNumber = m_fsAllocNum++;
        m_mountedFilesystems.append(filesystem);
        return filesystem.m_mountNumber;
    }
    
    void VirtualFilesystemManager::unregister_filesystem(int mountNum)
    {
        for (auto it = m_mountedFilesystems.first(); !it.is_end(); ++it)
        {
            if (it->m_mountNumber == mountNum)
            {
                m_mountedFilesystems.remove(it);
                break;
            }
        }
    }

    uint64_t VirtualFilesystemManager::read(VirtualFilesystemNode* vnode, uint64_t offset, uint64_t count, void* buf)
    {
        if (!vnode) return 0;
        return vnode->driver->vfsdriver_read(vnode, offset, count, (char*)buf);
    }

    uint64_t VirtualFilesystemManager::write(VirtualFilesystemNode* vnode, uint64_t offset, uint64_t count, void *buf)
    {
        if (!vnode) return 0;
        return vnode->driver->vfsdriver_write(vnode, offset, count, (char*)buf);
    }

    VirtualFilesystemNode* VirtualFilesystemManager::finddir(VirtualFilesystemNode* vnode, char* filename)
    {
        if (!vnode) return nullptr;
        return vnode->driver->vfsdriver_finddir(vnode, filename);
    }
