#ifndef KHEAP_H
#define KHEAP_H
#include "types.h"
#include "common/binarySearchTree.h"
#include "print.h"
#include "process/spinlock.h"
#include "memory/MemoryManager.h"
#include <stddef.h>

namespace Memory {

    #define HEAP_BASE 0xFFFFDF8000000000
    #define HEAP_INDEX_SIZE 0x8000000000
    #define HEAP_END 0xFFFFFF8000000000

    #define WORD_ALIGN 0x1
    struct HeapChunkHeader
    {
        bool isHole;
        uint64_t size;
    }__attribute__((packed));

    struct HeapChunkFooter
    {
        uint64_t size;
    }__attribute__((packed));

    struct HeapIndexEntry
    {
        uint64_t size;
        uint8_t* ptr;
        bool operator== (const HeapIndexEntry &entry) const
        {
            if (size == entry.size && ptr == entry.ptr)
                return true;
            return false;
        }

    }__attribute__((packed));

    class KernelHeapManager
    {
    public:
        ~KernelHeapManager() = default;
        void* kmalloc(uint64_t size, uint64_t flags);
        void kfree(void* ptr);
        static KernelHeapManager& instance()
        {
            static KernelHeapManager instance;
            return instance;
        }
        KernelHeapManager(KernelHeapManager const&) = delete;
        KernelHeapManager& operator=(KernelHeapManager const&) = delete;
    private:
        KernelHeapManager();
        Common::BinarySearchTree<HeapIndexEntry> m_bst;
        uint8_t* m_nextIndexPagePointer;
        uint8_t* m_nextHeapPagePointer;
        uint64_t* m_currentAllocPointer;
        static KernelHeapManager* m_kernelHeapManager;
        void insert_entry(HeapIndexEntry entry);
        void remove_entry(HeapIndexEntry entry);
        void write_entry_tags(HeapIndexEntry entry, bool isHole);
        HeapChunkHeader* get_corresponding_header(HeapChunkFooter* footer);
        HeapChunkHeader* get_next_header(HeapChunkHeader* header);
        HeapChunkHeader* get_prev_header(HeapChunkHeader* header);
        HeapIndexEntry get_entry(HeapChunkHeader* header);
        HeapIndexEntry get_entry(HeapChunkFooter* footer);
        HeapChunkHeader* get_header(HeapIndexEntry entry);
        void merge_forwards(HeapIndexEntry* entry);
        void merge_backwards(HeapIndexEntry* entry);
        void expand();
        HeapIndexEntry* find_smallest_entry_greater_than(uint64_t size);
        Spinlock m_spinlock;
    };

}

static void* kmalloc(uint64_t size, uint64_t flags) { return Memory::KernelHeapManager::instance().kmalloc(size, flags); }

static void kfree(void* ptr) { Memory::KernelHeapManager::instance().kfree(ptr); }
/* new and delete operators for kernel objects */

#endif
