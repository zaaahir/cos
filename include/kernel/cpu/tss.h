#ifndef TSS_H
#define TSS_H
#include "types.h"
#include "cpu/gdt.h"

namespace CPU {
    struct TaskStateSegment64
    {
    private:
        uint32_t m_resv0;
        uint64_t m_rsp0;
        uint64_t m_rsp1;
        uint64_t m_rsp2;
        uint64_t m_resv1;
        uint64_t m_ist1;
        uint64_t m_ist2;
        uint64_t m_ist3;
        uint64_t m_ist4;
        uint64_t m_ist5;
        uint64_t m_ist6;
        uint64_t m_ist7;
        uint64_t m_resv2;
        uint32_t m_resv3;
    public:
        uint64_t get_rsp0() { return m_rsp0; }
        void set_rsp0(uint64_t rsp0) { m_rsp0 = rsp0; }
    } __attribute__((packed));

    void set_task_state_segment(TaskStateSegment64* tss);
    void refresh_task_state_segment();
    void update_kernel_stack_pointer(void* stackPointer);
}

#endif
