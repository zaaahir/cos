# cOS

## Background

I have recently started taking an interest in the low-level implementation details of operating systems for computers. The operating system is system software that manages the computer system and provides services to programs. It consists of a kernel that has full control of the computer system and other loadable modules such as a networking layer.
Some of the responsibilities of the kernel include:

- Managing physical memory
- Managing virtual address spaces (paging)
- Scheduling and switching between programs
- Handling hardware and software interrupts
- Setting up architecture-specific structures
- Loading programs in and out of memory


Currently there are only a few mainstream operating systems: Microsoft Windows, macOS and Linux. Each of these operating systems are over 20 years old and are well supported by most commercial software.


## Project Outline

The mainstream operating systems mentioned in the previous section are extremely complex with millions of lines of code and have fixed designs for the sake of backwards compatibility. As a result, these operating systems are not used to teach undergraduate courses. Instead, there are operating systems written to be used in an educational context, such as MINIX and xv6. However, they do not support modern hardware and are often no longer maintained.
This project aims to investigate the process of creating an operating system that supports modern hardware to help me understand more about computer architectures and operating systems. Other beneficiaries of this project would be university students as they can learn about the mechanisms used by modern operating systems that are not utilised in older educational operating systems.

## Project Specification

These are checkboxes, but I will probably only review these guidlines at the end to check them off, rather than iteratively during the project. These are mostly for my own reference. 

### cOS Kernel

- [ ] - The kernel boots successfully.
  - [ ] - The kernel boots on x86-64 compatible machines.
  - [ ] - The kernel is bootable by any Multiboot-2 compliant bootloader. 
- [ ] - The kernel can parse a Multiboot-2 header.
- [ ] - The kernel sets up paging.
  - [ ] - 4-level paging
- [ ] - The kernel can load a Multiboot-2 module.
- [ ] - The kernel sets up x86-64 specific structures:
  - [ ] - A Global Descriptor Table (GDT)
  - [ ] - A Task State Segment (TSS)
  - [ ] - An Interrupt Descriptor Table (IDT)
- [ ] - The kernel enters long mode.
- [ ] - The kernel loads an initrd.
- [ ] - The initrd is loaded as a Multiboot-2 module.
- [ ] - The initrd is a Unix Standard TAR file.
- [ ] - The kernel initialises interrupts.
- [ ] - The kernel initialises the Programmable Interval Timer (PIT).
- [ ] - The kernel initialises a kernel heap that can allocate and free arbitrarily sized buffers.
  - [ ] - It must be able to supply word-aligned allocations.
- [ ] - The kernel initialises a scheduler that can switch between kernel tasks.
  - [ ] - The kernel must also provide a mechanism to switch to user mode. (ring 3)
- [ ] - The kernel detects devices on Peripheral Component Interconnect (PCI) buses.
  - [ ] - The kernel provides the ability to search for and configure devices on PCI buses.

### cOS filesystem layer

- [ ] - The layer provides an interface for multiple filesystems mounted in a single virtual filesystem.
  - [ ] - It provides an implementation for a driver for the USTAR tar file format.
- [ ] - The layer provides `read` and `write` interfaces for filesystem drivers.

### cOS networking layer

- [ ] - The kernel can provide networking services.
  - [ ] - It provides an adaptable interface for different network card drivers.
    - [ ] - It provides an implementation for the AMD Am79C973 network card.
  - [ ] - It provides an interface for sending and receiving Ethernet frames.
  - [ ] - It provides an interface for sending and receiving Address Resolution Protocol (ARP) messages.
  - [ ] - It provides an interface for handling Internet Protocol v4 (IPv4) messages.
  - [ ] - It provides an interface for sending and receiving Internet Control Message Protocol (ICMP) messages.
  - [ ] - It provides an interface for sending and receiving User Datagram Protocol (UDP) messages.
  - [ ] - It provides an interface for sending and receiving Transmission Control Protocol (TCP) messages.
    - [ ] - It supports reordering of received packets.
    - [ ] - It supports acknowledgment and retransmission of missed packets.

### cOS user-space

- [ ] - The kernel can load ELF executables into an address space for a newly created task.
- [ ] - The kernel provides a system calls interface to send data to the kernel.

## Design

My project will be to create an operating system (cOS) that runs on modern hardware and is able to run user-space programs. The project will consist of the following sections:

- The core kernel that is responsible for initialising and managing the processor; handing hardware operations and memory; executing and switching between processes; and providing services to processes.
- The filesystem layer that is responsible for providing a virtual filesystem interface for processes to access files in a concrete filesystem agnostic manner.
- The networking layer that is responsible for managing network operations and sockets by interfacing with the network card driver and managing various networking protocol stacks.
- The user-space in which most user programs execute with C Library support. This is separated from the kernel using the protection mechanisms provided by the processor.

### Target Architecture

The instruction set architecture which generally defines how software controls the computer processor. Although most architectures have a similar processing model, with interrupt and virtual memory support, they also have differences that mean they are not interoperable. They also have different machine code instructions (instruction sets) so must be recompiled with different binaries targeting different architectures. The x86 is the most used computer architecture in the world, with the x86-64 being the modern 64-bit adaptation that is supported by most mainstream operating systems. This architecture, as a result, has extensive documentation and has two systems programming manuals written for it: the Intel manual and the AMD manual. This makes it a suitable choice for an investigative project into modern operating systems. Another contender is the ARM 64-bit architecture (AArch64 or ARM64). This is a newer architecture that is most commonly used in mobile applications. There is not as much documentation available for the ARM architecture and it is not as widely used for desktop personal computers. As a result, we target the x86-64 architecture for our operating system.

### Cross Compilation

The default system compilers make assumptions about the host machine (the machine that the program will run on), such as whether it has support for dynamic loading and whether it is freestanding. As a result, we must compile our own cross- compiler.
A cross-compiler is a compiler where the host machine (machine that the binaries will execute on) is different from the build machine (machine that compiles the binaries). Our host machine will be running cOS, however we do not have a compiler to build cOS! We make a generic cross-compiler that targets a generic x86-64 machine with no operating system in order to build our operating system. We use `gcc` as our compiler, and our host machine is `x86-64-elf` (we can eventually create our own toolchain and our host machine will be `x86-64-cos`). We will also build this using a Docker container, as this allows our build process to be easily reproducible by anyone. A deep dive into the build process is beyond the scope of this documentation.

### Hierarchy Charts (In Progress)

We show hierarchy charts for each part of the operating system, followed by a table describing the function of each module.

### Kernel

<img width="447" alt="Screenshot 2024-08-13 at 14 39 05" src="https://github.com/user-attachments/assets/8eccf38c-bcb2-4405-a1cf-d1a42cd6d523" style="display: block; margin-left: auto; margin-right: auto;">



<table>
  <tr>
    <td colspan="2">
  <div style="display: flex; justify-content: center; align-items: center; height: 100%;">
    <strong>Kernel</strong>
  </div>
</td>
   </td>
  </tr>
  <tr>
   <td><strong>Module</strong> 
   </td>
   <td><strong>Function</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>Boot</strong> 
   </td>
   <td><strong>Sets up the execution environment for the kernel. This involves initialising the processor and other hardware to run 64-bit code. </strong> 
   </td>
  </tr>
  <tr>
   <td>Long mode 
   </td>
   <td>Sets up temporary paging structures that map physical memory at a high address as a prerequisite for entering long mode. Then long mode can be enabled on the processor and a far jump can be made to the kernel’s main C entry point. 
   </td>
  </tr>
  <tr>
   <td>Multiboot 
   </td>
   <td>Parses the Multiboot 2 header and allows the kernel to iterate through relevant Multiboot 2 tags with a custom class. 
   </td>
  </tr>
  <tr>
   <td>Module loading 
   </td>
   <td>Iterates through Multiboot 2 module tags and ensures that they are not overwritten by kernel data by passing their memory addresses to the memory manager. 
   </td>
  </tr>
  <tr>
   <td><strong>CPU Management</strong> 
   </td>
   <td><strong>Sets up more architecture specific structures such as the Global Descriptor Table and Task State Segment. This also enables hardware and software interrupts, as well as a timer.</strong> 
   </td>
  </tr>
  <tr>
   <td>Global Descriptor Table 
   </td>
   <td>The GDT contains descriptors for different memory segments as well as the TSS. We set up the GDT with entries for ring 0 code, ring 0 data, ring 3 code, and ring 3 data. These segments cover the whole of memory in long mode. 
   </td>
  </tr>
  <tr>
   <td>Task State Segment 
   </td>
   <td>The TSS is used in long mode to load the stack pointer when a privilege change occurs. We set up an empty TSS that we later install with a kernel stack pointer when a task switch occurs. 
   </td>
  </tr>
  <tr>
   <td>Interrupts 
   </td>
   <td>An Interrupt Descriptor Table is required to enable interrupts, which contains entries for each interrupt vector. To receive interrupts, classes can use inheritance to conform as interrupt handlers, causing a pointer to a member function being installed in the IDT. Additionally, the Programmable Interrupt Controller is initialised to receive hardware interrupts, and the Programmable Interval Timer is also enabled. 
   </td>
  </tr>
  <tr>
   <td><strong>Memory Manager</strong> 
   </td>
   <td><strong>Manages both physical memory and virtual memory address spaces, providing an abstracted interface to the rest of the kernel.</strong> 
   </td>
  </tr>
  <tr>
   <td>Physical bitmap 
   </td>
   <td>Physical memory is divided into 4KiB pages, and a bitmap records which pages are being used. The kernel memory region and module regions are set to being used. To find a free page, we iterate through the bitmap to find the first unset bit and convert the bit number into a page address. 
   </td>
  </tr>
  <tr>
   <td>Paging 
   </td>
   <td>New paging structures are initialised to map physical memory as well as modules into a master kernel address space. We manage page tables by allocating physical memory for them and deallocating them when they are no longer needed. We can also change address spaces by changing the CR3 register. 
   </td>
  </tr>
  <tr>
   <td>Virtual page allocator 
   </td>
   <td>The page allocator is used to allocate a physical page and then map it into a given address space at a logical address. 
   </td>
  </tr>
  <tr>
   <td><strong>Kernel Heap Allocator</strong> 
   </td>
   <td><strong>Manages a heap of memory allocations that can be arbitrarily sized.</strong> 
   </td>
  </tr>
  <tr>
   <td>Heap allocator 
   </td>
   <td>Allows the kernel to request memory allocations. When this happens, the heap allocator allocates physical memory pages if needed and then finds an available memory space to partition for the allocation. 
   </td>
  </tr>
  <tr>
   <td><strong>Devices</strong> 
   </td>
   <td><strong>Manages the Peripheral Components Interconnect and interrupt-based or I/O-based devices</strong> 
   </td>
  </tr>
  <tr>
   <td>PCI bus iteration 
   </td>
   <td>Provides an interface to use the PCI bus. Detects all devices on the PCI bus that drivers can use to configure and communicate with the devices. 
   </td>
  </tr>
  <tr>
   <td>Keyboard 
   </td>
   <td>PS/2 keyboards send interrupts which we handle, reading the scan-codes from an I/O port.  
   </td>
  </tr>
  <tr>
   <td><strong>Events</strong> 
   </td>
   <td><strong>The Events system allows for tasks to register to receive messages from event senders. </strong> 
   </td>
  </tr>
  <tr>
   <td>Sender registry 
   </td>
   <td>Classes can use inheritance to register as an event sender. They manage process wait and listen queues, waking and broadcasting event messages to these processes when needed. 
   </td>
  </tr>
  <tr>
   <td>Dispatcher 
   </td>
   <td>The dispatcher is responsible to signalling to processes when an event message has been sent. This includes modifying wait and listen queues when necessary. 
   </td>
  </tr>
  <tr>
   <td>Task blocking 
   </td>
   <td>Tasks can choose to wait for an event message. This blocks the task (stops from being scheduled) until the dispatcher dispatches an event message to them, preventing them from wasting processor time. 
   </td>
  </tr>
  <tr>
   <td><strong>Task Manager</strong> 
   </td>
   <td><strong>Performs pre-emptive multitasking by switching between tasks according to a scheduler.</strong> 
   </td>
  </tr>
  <tr>
   <td>Task creation 
   </td>
   <td>Creating new resources such as an address space, file descriptor/event descriptor tables, and a stack. Starts a task by faking an interrupt stack and returning from it. 
   </td>
  </tr>
  <tr>
   <td>Task scheduling 
   </td>
   <td>Uses a scheduler to choose the next task that should execute on the processor. This is talked about in more detail later. 
   </td>
  </tr>
  <tr>
   <td>Task switching 
   </td>
   <td>Uses interrupts to switch between tasks; the stack pointer is switched to the new task’s stack pointer and returning from the interrupt executes the new task. 
   </td>
  </tr>
  <tr>
   <td>Synchronisation 
   </td>
   <td>Provides synchronisation primitives. This includes a spinlock, which disables interrupts and busy-waits if the lock is acquired. Also provides a mutex blocks a task if it is already acquired and adds it to a wait queue that is awoken when the mutex is released. 
   </td>
  </tr>
</table>

## Resources

1 - OSDev Wiki (https://osdev.wiki/wiki/Expanded_Main_Page)
