# cOS - A Modern Operating System

Winner of the 2023 Hack Club Arcade Showcase, out of over 2000 projects. 

## Background

I have recently started taking an interest in the low-level implementation details of operating systems for computers. The operating system is system software that manages the computer system and provides services to programs. It consists of a kernel that has full control of the computer system and other loadable modules such as a networking layer.
Some of the responsibilities of the kernel include:

- Managing physical memory
- Managing virtual address spaces (paging)
- Scheduling and switching between programs
- Handling hardware and software interrupts
- Setting up architecture-specific structures
- Loading programs in and out of memory


Currently there are only a few mainstream operating systems: Microsoft Windows, macOS and Linux. Each of these operating systems are over 20 years old and are well supported by most commercial software. Installation can be done using the Dockerfile. You will need QEMU (the emulator I used) for running locally!


## Project Outline

The mainstream operating systems mentioned in the previous section are extremely complex with millions of lines of code and have fixed designs for the sake of backwards compatibility. As a result, these operating systems are not used to teach undergraduate courses. Instead, there are operating systems written to be used in an educational context, such as MINIX and xv6. However, they do not support modern hardware and are often no longer maintained.
This project aims to investigate the process of creating an operating system that supports modern hardware to help me understand more about computer architectures and operating systems. Other beneficiaries of this project would be university students as they can learn about the mechanisms used by modern operating systems that are not utilised in older educational operating systems.

## Project Specification

These are checkboxes, but I will probably only review these guidlines at the end to check them off, rather than iteratively during the project. These are mostly for my own reference. 

_Edit: Quoted items are my evalutation on this completed checklist, and a link to evidence._

### cOS Kernel

- [x] - The kernel boots successfully.
  - [x] - The kernel boots on x86-64 compatible machines.
  - [x] - The kernel is bootable by any Multiboot-2 compliant bootloader.

  > The kernel boots successfully across different virtualisation software. As a result, it should also work on physical hardware, although I have not tested this. This is also demonstrated in testing by test 1.1.
  
- [x] - The kernel can parse a Multiboot-2 header.

  > This is demonstrated by being able to detect physical memory by reading the header, as in test 1.2. Certain fields however are not explicitly parsed, and the length of physical memory is assumed to be equal to the end memory address of physical memory.

- [x] - The kernel sets up paging.
  - [x] - 4-level paging

> Paging is functional and the whole of physical memory is mapped into the main address space. The kernel exposes a simple interface for applications to map virtual pages to physical pages or allocate virtual pages. It also handles page faults for user-space programs by copying pages from the main address space into the user-space address space if available. Tests 1.3 and 1.5 verify that the paging structures are properly set up.

- [x] - The kernel can load a Multiboot-2 module.

> The initial ramdisk is loaded as a module and the physical pages are protected from being overwritten. The kernel also maps the module into the address space at a fixed address. It can currently only use a single module; however, it still prevents other modules from being overwritten in memory.
      
- [x] - The kernel sets up x86-64 specific structures:
  - [x] - A Global Descriptor Table (GDT)
  - [x] - A Task State Segment (TSS)
  - [x] - An Interrupt Descriptor Table (IDT)

    > The Global Descriptor Table and Task State Segment are both managed and set up by the kernel. The Global Descriptor Table contains entries for different protection levels; ring 0 and ring 3. This helps protect the operating system from malicious or buggy user-space programs. Ring 1 and 2 could possibly be used for device drivers as to allow less protection than user-space but more protection than running at kernel level. These have been checked by tests 1.4 and 1.6. 
- [x] - The kernel enters long mode.

> The kernel jumps to a higher half 64-bit address which is where 64-bit code begins execution.
> 
- [x] - The kernel loads an initrd.

> The initrd is loaded from a Multiboot-2 module. It is in the form of a Unix Standard TAR file which is parsed and accessible from the virtual filesystem. Tests 2.4 and 2.5 demonstrate this. System files and drivers can be included in the initrd. The TAR file is uncompressed so it may be worth using a compressible format to reduce image sizes.

- [x] - The initrd is loaded as a Multiboot-2 module.
- [x] - The initrd is a Unix Standard TAR file.
- [x] - The kernel initialises interrupts.

> The Interrupt Descriptor Table is set up to call a custom interrupt handler, as verified in test 1.6. This uses virtual functions to find a relevant interrupt handler and call it. As a result, device drivers can easily handle interrupts by inheriting from the interrupt handler class. The Programmable Interrupt Controller (PIC) has also been set up as verified in test 1.7, and hardware interrupts are received in test 1.8. In the future, the Advanced Programmable Interrupt Controller (APIC) could be used to allow for multi-processor interrupt routing.

- [x] - The kernel initialises the Programmable Interval Timer (PIT).

> The PIT provides a global timer that is used for processes to sleep for a certain number of milliseconds. The interrupt itself is used to check if tasks can be switched and allows us to interrupt a malfunctioning program. Higher precision timers could be used to gain an even better global timer.

- [x] - The kernel initialises a kernel heap that can allocate and free arbitrarily sized buffers.
  - [x] - It must be able to supply word-aligned allocations.

  > The heap allocator works effectively and has been tested. It is used extensively in the kernel and therefore it is crucial that it is performant. The use of a binary search tree over a regular list of free allocations significantly improves performance. Rebalancing the binary search tree would likely further increase performance.

- [x] - The kernel initialises a scheduler that can switch between kernel tasks.
  - [x] - The kernel must also provide a mechanism to switch to user mode. (ring 3)

  > Task switching uses interrupt stacks to store the state of an interrupted task and restore state when resuming the task. The task switching is able to switch between kernel and user-space processes efficiently. In addition, the scheduler chooses which task should run next using a priority-based system. This has been tested in tests 4.1 through 4.6. Alternative scheduler designs might lead to a fairer and more performant scheduler.
  
- [x] - The kernel detects devices on Peripheral Component Interconnect (PCI) buses.
  - [x] - The kernel provides the ability to search for and configure devices on PCI buses.

  > All devices were detected in test 2.3. The network driver also configures the network card on the PCI bus successfully.

### cOS filesystem layer

- [x] - The layer provides an interface for multiple filesystems mounted in a single virtual filesystem.
  - [x] - It provides an implementation for a driver for the USTAR tar file format.

  > The virtual filesystem can easily mount new filesystems with a simple interface. It can abstract file operations on a tree structure regardless of the underlying structure of the file. This is seen in the flat TAR file being mapped into a tree of file and directory nodes.

- [x] - The layer provides `read` and `write` interfaces for filesystem drivers.

> Filesystem drivers only need to provide a single interface to be mountable in the virtual filesystem. This also means that drivers for other operating systems can easily be ported and executed. 

### cOS networking layer

- [x] - The kernel can provide networking services.
  - [x] - It provides an adaptable interface for different network card drivers.
    - [x] - It provides an implementation for the AMD Am79C973 network card.
  - [x] - It provides an interface for sending and receiving Ethernet frames.
  - [x] - It provides an interface for sending and receiving Address Resolution Protocol (ARP) messages.
  - [x] - It provides an interface for handling Internet Protocol v4 (IPv4) messages.
  - [x] - It provides an interface for sending and receiving Internet Control Message Protocol (ICMP) messages.
  - [x] - It provides an interface for sending and receiving User Datagram Protocol (UDP) messages.
  - [x] - It provides an interface for sending and receiving Transmission Control Protocol (TCP) messages.
    - [x] - It supports reordering of received packets.
    - [x] - It supports acknowledgment and retransmission of missed packets.
       
  > The networking stack is very functional and provides a TCP/IP implementation with support for sockets. It exposes hardware-level communication as well as higher-level socket communication. It even allows for accessing the Internet through HTTP, which is a very complex procedure. Tests 3.1 to 3.8 all passed and demonstrated the strength and compliance of the networking stack. It would be useful to port an SSL/TLS library so that HTTPS can be used to access modern websites. Making the network stack multi-threaded with queues would also be valuable, especially for server applications. 

### cOS user-space

- [x] - The kernel can load ELF executables into an address space for a newly created task.

> This has been demonstrated with test 4.4. Static ELF executables can be completely loaded in and executed in accordance with the ELF specification. Relocatable ELF executables are not currently supported.

- [x] - The kernel provides a system calls interface to send data to the kernel.

> This uses the built-in `syscall` instruction, storing arguments in registers. This is very fast as it avoids some of the overhead of interrupts. Currently there are not many system calls available. Care must be taken to ensure all data coming from user-space is treated as untrustworthy and scrutinised with defensive programming.

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

### Hierarchy Charts

We show hierarchy charts for each part of the operating system, followed by a table describing the function of each module.

### Kernel

<p align="center">
<img width="447" alt="Screenshot 2024-08-13 at 14 39 05" src="https://github.com/user-attachments/assets/8eccf38c-bcb2-4405-a1cf-d1a42cd6d523" style="display: block; margin-left: auto; margin-right: auto;">
</p>


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

### Filesystem Layer

<p align="center">
<img width="361" alt="Screenshot 2024-08-13 at 19 36 29" src="https://github.com/user-attachments/assets/ee8300e7-f0a8-44cf-b2f4-0c8023fdfa4e">
</p>


<table>
  <tr>
   <td colspan="2" ><strong>Filesystem Layer</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>Module</strong> 
   </td>
   <td><strong>Function</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>Virtual Filesystem Manager</strong> 
   </td>
   <td><strong>Acts as the main interface for handling files. Manages a tree structure of files and redirects file handling operations to the relevant filesystem driver.</strong> 
   </td>
  </tr>
  <tr>
   <td>File tree 
   </td>
   <td>The file tree consists of directories and files that are arranged using nodes. Each node can have child nodes and are dynamically allocated when a directory or file is requested. 
   </td>
  </tr>
  <tr>
   <td>Filesystem driver interface 
   </td>
   <td>Provides a standard interface for opening, reading, writing, and closing files. Each function calls the function for the underlying driver. This will be implemented using virtual functions and inheritance. 
   </td>
  </tr>
  <tr>
   <td><strong>TAR driver</strong> 
   </td>
   <td><strong>Reads Unix Standard TAR files and mounts the initial ramdisk as a concrete filesystem.</strong> 
   </td>
  </tr>
  <tr>
   <td>TAR header parsing 
   </td>
   <td>TAR headers are 512-byte aligned, and these headers contain various metadata about each file in the TAR archive. These headers are parsed and stored so that the contents of files can be accessed. 
   </td>
  </tr>
  <tr>
   <td>VFS node translation 
   </td>
   <td>The driver is responsible for converting the flat, sequential storage of files into a node-based tree of files and directories so that the Virtual Filesystem can interpret them. This is done dynamically as the Virtual Filesystem calls for file operations. 
   </td>
  </tr>
</table>

### Networking layer

<p align="center">
<img width="421" alt="Screenshot 2024-08-13 at 19 38 45" src="https://github.com/user-attachments/assets/7dd50385-0955-4662-86a7-80b5e0365eb1">
</p>

<table>
  <tr>
   <td colspan="2" ><strong>Networking Layer</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>Module</strong> 
   </td>
   <td><strong>Function</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>Network driver</strong> 
   </td>
   <td><strong>Communicates with a network card to send and receive data over a network.</strong> 
   </td>
  </tr>
  <tr>
   <td>Configuration 
   </td>
   <td>Configures the network driver to send and receive Ethernet II frames. This involves configuring the PCI device and sending commands through I/O ports to enable the device.  
   </td>
  </tr>
  <tr>
   <td>Receive Ethernet frames 
   </td>
   <td>Allocates a circular buffer in physical memory to receive Ethernet frames. Informs the network card and maps it into the virtual address space. It also enables interrupts to receive on the device. 
   </td>
  </tr>
  <tr>
   <td>Send Ethernet frames 
   </td>
   <td>Allocates a circular buffer in physical memory to send Ethernet frames. Informs the network card and maps it into the virtual address space. 
   </td>
  </tr>
  <tr>
   <td><strong>Address Resolution Protocol</strong> 
   </td>
   <td><strong>Implements the Address Resolution Protocol for translation Internet Protocol version 4 addresses to MAC addresses.</strong> 
   </td>
  </tr>
  <tr>
   <td>Request address resolution 
   </td>
   <td>Sends an ARP request and waits to receive a response. Processes the response and caches MAC addresses that have already been resolved in a hash map. 
   </td>
  </tr>
  <tr>
   <td>Respond to requests 
   </td>
   <td>If the network card’s IP address is equal to the request’s resolution address, respond with an ARP response containing the network card’s MAC address. 
   </td>
  </tr>
  <tr>
   <td><strong>Internet Protocol v4</strong> 
   </td>
   <td><strong>Receive and send Internet Protocol v4 packets, forwarding them to specific IPv4 protocol handlers. This is done using inheritance, which registers a protocol handler in the main IPv4 class.</strong> 
   </td>
  </tr>
</table>

### Internet Protocol v4

<p align="center">
<img width="385" alt="Screenshot 2024-08-13 at 19 39 10" src="https://github.com/user-attachments/assets/f009305e-a4a4-47ef-b5b1-c7f3a2189397">
</p>

<table>
  <tr>
   <td colspan="2" ><strong>Internet Protocol v4</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>Module</strong> 
   </td>
   <td><strong>Function</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>User Datagram Protocol</strong> 
   </td>
   <td><strong>Implements the User Datagram Protocol using sockets.</strong> 
   </td>
  </tr>
  <tr>
   <td>Port management 
   </td>
   <td>Uses a class to represent a port that can send and receive data by calling the relevant UDP functions. 
   </td>
  </tr>
  <tr>
   <td>Handler 
   </td>
   <td>Classes can use inheritance to call a function upon receiving UDP messages on a port. 
   </td>
  </tr>
  <tr>
   <td><strong>Transmission Control Protocol</strong> 
   </td>
   <td><strong>Implements the Transmission Control Protocol using sockets.</strong> 
   </td>
  </tr>
  <tr>
   <td>Port management 
   </td>
   <td>Uses a class to represent a port that can send and receive data by calling the relevant TCP functions. 
   </td>
  </tr>
  <tr>
   <td>Connection establishment 
   </td>
   <td>Can passive open or connect to a server, establishing a connection with a two-way handshake, sending SYN packets, and managing state. 
   </td>
  </tr>
  <tr>
   <td>Received packet reordering 
   </td>
   <td>Uses a buffer to hold received packets and reorder them by synchronisation number. Sends to the application layer when there is a sequential number of packets, and the acknowledgement number is updated. 
   </td>
  </tr>
  <tr>
   <td>Retransmission 
   </td>
   <td>Uses a buffer to hold transmitted packets and starts a timer to retransmit them if the receiver does not acknowledge them. 
   </td>
  </tr>
</table>

### Userspace

<p align="center">
<img width="302" alt="Screenshot 2024-08-13 at 19 40 38" src="https://github.com/user-attachments/assets/44e92cd0-54cf-4d7f-a9e9-ba026f3dac30">
</p>

<table>
  <tr>
   <td colspan="2" ><strong>User-space</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>Module</strong> 
   </td>
   <td><strong>Function</strong> 
   </td>
  </tr>
  <tr>
   <td><strong>ELF Loader</strong> 
   </td>
   <td><strong>Loads an ELF file to be executed as a task.</strong> 
   </td>
  </tr>
  <tr>
   <td>Loads in segments 
   </td>
   <td>Loads in program executable segments into a task’s address space. 
   </td>
  </tr>
  <tr>
   <td><strong>System Calls</strong> 
   </td>
   <td><strong>Implements a system calls interface for user-space processes to request services from the operating system.</strong> 
   </td>
  </tr>
  <tr>
   <td>Syscall handler 
   </td>
   <td>Executes when <code>syscall</code> is called. Takes arguments from registers loaded by the user-space program. Calls various kernel functions to execute syscall 
   </td>
  </tr>
</table>

## Testing

I will test using white-box testing, as black-box testing would not be appropriate for the kernel. Instead, code segments are run in the kernel or as processes. 

### Processor Initialisation 

It is the operating system’s responsibility to properly initialise the processor to be able to execute code at various privilege levels, receive interrupts and use virtual memory addressing. To execute the operating system, I will use virtualisation software to emulate real hardware. I will be using QEMU.

<table>
  <tr>
   <td>Test ID 
   </td>
   <td>Description 
   </td>
   <td>Expected result 
   </td>
  </tr>
  <tr>
   <td>1.1 
   </td>
   <td>The operating system will boot. This is tested by running in QEMU. 
   </td>
   <td>The operating system boots without a General Protection Fault. 
   </td>
  </tr>
  <tr>
   <td>1.2 
   </td>
   <td>All physical memory is detected by the operating system. This is tested by outputting the memory region map and comparing to QEMU’s input parameters. 
   </td>
   <td>The regions of physical memory correspond to the QEMU memory length. 
   </td>
  </tr>
  <tr>
   <td>1.3 
   </td>
   <td>Physical memory is loaded into the virtual memory address space. This is tested by printing the contents of a virtual memory address corresponding to <code>0x1000</code> and comparing to a debug log of physical memory contents. 
   </td>
   <td>The virtual memory access matches the physical memory access. 
   </td>
  </tr>
  <tr>
   <td>1.4 
   </td>
   <td>The GDT is loaded with a valid entry. This is tested by checking the QEMU monitor GDT register (GDTR). 
   </td>
   <td>The GDTR holds a valid GDT address as reported by QEMU. 
   </td>
  </tr>
  <tr>
   <td>1.5 
   </td>
   <td>Page allocation is functional. This is tested by allocating address <code>0x1000</code>.  
   </td>
   <td>Without allocating, a page fault occurs. After allocating, no page fault occurs on access.  
   </td>
  </tr>
  <tr>
   <td>1.6 
   </td>
   <td>The IDT is loaded with a valid entry. This is tested by checking the QEMU monitor IDT register (IDTR). 
   </td>
   <td>The IDTR holds a valid IDT address as reported by QEMU. 
   </td>
  </tr>
  <tr>
   <td>1.7 
   </td>
   <td>The Programmable Interrupt Controller is initialised and remapped to <code>0x20</code> (PIC0) and <code>0x28</code> (PIC1). This would be tested by reading the QEMU monitors PIC offsets. 
   </td>
   <td>The PIC offsets are remapped. 
   </td>
  </tr>
  <tr>
   <td>1.8 
   </td>
   <td>Hardware interrupts call the interrupt handler. This is tested by sending keyboard scan-codes. 
   </td>
   <td>The keyboard scan-codes are printed on the screen. 
   </td>
  </tr>
  <tr>
   <td>1.9 
   </td>
   <td>Unmapped pages cause page fault. This is tested by attempting to access an unmapped page in low memory. 
   </td>
   <td>A page fault occurs. 
   </td>
  </tr>
</table>

### Kernel Resource Management

I will also test that the kernel manages hardware resources properly. This includes the kernel heap, the PCI bus and the virtual filesystem.


<table>
  <tr>
   <td>Test ID 
   </td>
   <td>Description 
   </td>
   <td>Expected result 
   </td>
  </tr>
  <tr>
   <td>2.1 
   </td>
   <td>The kernel heap allocations can be filled with data and is not overwritten with successive allocations. This is tested by ensuring data stored in an allocation remains the same. 
   </td>
   <td>The allocation data is not overwritten. 
   </td>
  </tr>
  <tr>
   <td>2.2 
   </td>
   <td>Allocations larger than a page can be made. This is tested by allocating <code>8192</code> bytes. 
   </td>
   <td>The kernel does not crash or raise any exception. 
   </td>
  </tr>
  <tr>
   <td>2.3 
   </td>
   <td>All devices on the PCI bus are detected. This is tested by comparing the detected devices to the QEMU monitor PCI devices. 
   </td>
   <td>All devices are the same. 
   </td>
  </tr>
  <tr>
   <td>2.4 
   </td>
   <td>Reading files from initial ramdisk (initrd). This is tested by placing a file inside the initrd and reading it. 
   </td>
   <td>The displayed contents is the same as the file content. 
   </td>
  </tr>
  <tr>
   <td>2.5 
   </td>
   <td>Read files inside directories from initial ramdisk. This is tested by placing a directory inside the initrd and reading it. 
   </td>
   <td>The displayed contents is the same as the file content 
   </td>
  </tr>
  <tr>
   <td>2.6 
   </td>
   <td>Event dispatching works. This is tested by receiving keyboard events in a task. 
   </td>
   <td>The keyboard scan-codes are printed on the screen by the task. 
   </td>
  </tr>
</table>

### Networking 

To test networking, I will use QEMU’s command line to save a dump of network communications. This dump can then be read with Wireshark to check whether protocols are conformed to. 


<table>
  <tr>
   <td>Test ID 
   </td>
   <td>Description 
   </td>
   <td>Expected result 
   </td>
  </tr>
  <tr>
   <td>3.1 
   </td>
   <td>Able to send data over the network. This is tested by calling the network driver’s send function with a string. 
   </td>
   <td>Wireshark displays that only the string has been sent over the network.  
   </td>
  </tr>
  <tr>
   <td>3.2 
   </td>
   <td>Able to send well-formed Ethernet packets over the network. This is tested by calling the Ethernet layer manager’s send function with a string. 
   </td>
   <td>Wireshark displays a valid Ethernet frame has been sent (other than an invalid EtherType). 
   </td>
  </tr>
  <tr>
   <td>3.3 
   </td>
   <td>Able to resolve an IP address to a MAC address. This is tested by calling the Address Resolution Protocol layer manager’s resolve function for the gateway IP address. 
   </td>
   <td>Wireshark displays a valid ARP request and a valid ARP response from the gateway. The result is cached. 
   </td>
  </tr>
  <tr>
   <td>3.4 
   </td>
   <td>Able to send IPv4 packets. This is tested by calling the IPv4 layer manager’s send function with a string to send to the gateway IP. 
   </td>
   <td>Wireshark displays an ARP response and reply and then a valid IPv4 frame (other than the protocol type). 
   </td>
  </tr>
  <tr>
   <td>3.5 
   </td>
   <td>Able to send UDP packets. This is tested by calling the UDP layer manager’s send function with a string to send to the gateway IP. 
   </td>
   <td>Wireshark displays a compliant UDP transmission. 
   </td>
  </tr>
  <tr>
   <td>3.6 
   </td>
   <td>Able to receive UDP packets. This is tested by listening on a socket for connections and sending data using netcat. 
   </td>
   <td>The correct transmitted data is printed on the screen. 
   </td>
  </tr>
  <tr>
   <td>3.7 
   </td>
   <td>Able to connect to a remote TCP server. This is tested by connecting to a server hosted on the host machine using netcat.   
   </td>
   <td>Wireshark displays a compliant TCP transmission. 
   </td>
  </tr>
  <tr>
   <td>3.8 
   </td>
   <td>Able to send a HTTP POST request to an online website. This is tested by connecting to a webhook monitoring website webhook.site. 
   </td>
   <td>Wireshark displays a compliant HTTP transmission. 
   </td>
  </tr>
</table>

### Task Management and User-space 

<table>
  <tr>
   <td>Test ID 
   </td>
   <td>Description 
   </td>
   <td>Expected result 
   </td>
  </tr>
  <tr>
   <td>4.1 
   </td>
   <td>Load multiple tasks. Each task should execute. This is tested by running multiple tasks that print to the screen. 
   </td>
   <td>Each task prints to the screen.  
   </td>
  </tr>
  <tr>
   <td>4.2 
   </td>
   <td>Load multiple tasks with varying priorities. The tasks should execute in order of priority.  
   </td>
   <td>The tasks print in order of priority. 
   </td>
  </tr>
  <tr>
   <td>4.3 
   </td>
   <td>Block all tasks to ensure the kernel continues to run. 
   </td>
   <td>The kernel does not crash. 
   </td>
  </tr>
  <tr>
   <td>4.4 
   </td>
   <td>Load an ELF file into an address space and begin executing in userspace. The ELF file should also use the system calls interface. This is tested by compiling a program, loading the binary from the initrd and executing it in userspace. 
   </td>
   <td>The program prints to the screen using the system call. 
   </td>
  </tr>
  <tr>
   <td>4.5 
   </td>
   <td>Ensure code is running in user-space by trying to perform a privileged instruction. This is tested by calling the <code>cli</code> instruction which will cause a General Protection Fault. 
   </td>
   <td>A General Protection Fault is raised. 
   </td>
  </tr>
  <tr>
   <td>4.6 
   </td>
   <td>Processes should be able to block for a certain number of milliseconds by calling <code>sleep_for</code>. This is tested by having a process print to the screen and then sleeping repeatedly. 
   </td>
   <td>The process prints to the screen at a fixed rate. 
   </td>
  </tr>
</table>


### Evidence


<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/98eafbec-c08a-42fc-b2cf-54b47a729fd5" alt="Screenshot of QEMU as evidence for test 1.1.">
</p>

<p align="center">
<i>Screenshot of QEMU as evidence for test 1.1.</i>
</p>


<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/23894dce-de67-4ba2-8c36-2f49cbf94c09" alt="Screenshot of memory map as evidence for test 1.2. ">
</p>

<p align="center">
<i>Screenshot of memory map as evidence for test 1.2. </i>
</p>


<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/984673a3-0c72-43d6-8c27-5127d9243fad" alt="Screenshot of accessing the virtual address of the physical address 0x1000 as evidence for test 1.3.  ">
</p>

<p align="center">
<i>Screenshot of accessing the virtual address of the physical address 0x1000 as evidence for test 1.3.  </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/2f492813-2529-4c13-b568-9be97022c0f2" alt="Screenshot of contents of 0x1000 from QEMU monitor as evidence for test 1.3. The xp command returns the content of a physical address. ">
</p>

<p align="center">
<i>Screenshot of contents of 0x1000 from QEMU monitor as evidence for test 1.3. <br>
    The xp command returns the content of a physical address.  </i>
</p>

```c
printf("Contents of 0x1000: "); 
printf(*(uint8_t*)Memory::VirtualAddress(Memory::PhysicalAddress(0x1000)).get()); 
```

<p align="center">
<i>Kernel code used to access address for test 1.3.   </i>
</p>


<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/6a70237d-47e8-4b17-af30-615f6263ecce" alt="Screenshot of registers from QEMU monitor for test 1.4.">
</p>

<p align="center">
<i>Screenshot of registers from QEMU monitor for test 1.4. </i>
</p>



<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/d3882b68-49e6-4f2c-847e-be064ef49e95" alt="Screenshot of page fault before allocating page for test 1.5. ">
</p>

<p align="center">
<i>Screenshot of page fault before allocating page for test 1.5.  </i>
</p>

```c
printf("Value of virtual address 0x1000"); 
printf(*(uint8_t*)0x1000); 
```


<p align="center">
<i>Kernel code used to access address and cause page fault for test 1.5.  </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/7de8f9a5-a2fd-4940-9754-4a372d56f218" alt="Screenshot of no page fault occurring after allocating page for test 1.5. ">
</p>

<p align="center">
<i>Screenshot of no page fault occurring after allocating page for test 1.5. </i>
</p>

```cpp
Memory::VirtualMemoryAllocationRequest request(Memory::VirtualAddress(0x1000), true, true); 
Memory::MemoryManager::instance().alloc_page(request); 

printf("Value of virtual address 0x1000: "); 

printf(*(uint8_t*)0x1000); 
```

<p align="center">
<i>Kernel code used to allocate page for test 1.5.   </i>
</p>


<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/059f4333-a3cd-4d7b-b245-9c6334ef49d6" alt="Screenshot of registers from QEMU monitor for test 1.6. ">
</p>

<p align="center">
<i>Screenshot of registers from QEMU monitor for test 1.6. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/7eb78dc9-7c18-4b8c-841a-872670906350" alt="Screenshot of PIC offsets from QEMU monitor for test 1.7. ">
</p>

<p align="center">
<i>Screenshot of PIC offsets from QEMU monitor for test 1.7. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/bce971e4-4cb1-413d-a000-0c518180b04e" alt="Screenshot of keyboard scan-codes being printed for test 1.8. ">
</p>

<p align="center">
<i>Screenshot of keyboard scan-codes being printed for test 1.8. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/31b17980-67a7-4fd0-9497-21083b20287e" alt="Screenshot of page fault occurring by accessing address 4321 for test 1.9. ">
</p>

<p align="center">
<i>Screenshot of page fault occurring by accessing address 4321 for test 1.9. </i>
</p>

```cpp
printf(*(uint8_t*) 4321); 
```

<p align="center">
<i>Kernel code used to access address for test 1.9.  </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/1cb1efb0-e675-4b76-a5a3-98efb4a4684e" alt="Screenshot of allocation not being overwritten for test 2.1. ">
</p>

<p align="center">
<i>Screenshot of allocation not being overwritten for test 2.1.  </i>
</p>

```cpp
// Allocate 10 bytes. 
    auto allocation = kmalloc(10, 0); 
    uint8_t magicBytes[10] = {0x03, 0x11, 0x3F, 0x5C, 0x91, 0x11, 0x44, 0x53, 0xDE, 0xB0}; 
    // Copy magicBytes into allocation. 
    memcpy(allocation, magicBytes, 10); 
    // Allocate more bytes. 
    auto secondAllocation = kmalloc(100, 0); 
    kfree(secondAllocation); 
    // Check first allocation has not been overwritten. 
    bool overwritten = false; 
    for (int i = 0; i < 10; i++) 
    { 
        if (static_cast<uint8_t*>(allocation)[i] != magicBytes[i]) 
        { 
            overwritten = true; 
            break; 
        } 
    } 
    if (!overwritten) 
        printf("Allocation was not overwritten."); 
```

<p align="center">
<i>Kernel code used to allocate memory for test 2.1. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/084133b8-c96b-44ab-b723-2051b108256e" alt="Screenshot of kernel not crashing for test 2.2.">
</p>

<p align="center">
<i>Screenshot of kernel not crashing for test 2.2. </i>
</p>

```cpp
// Allocate 8192 bytes. 
auto allocation = kmalloc(8192, 0); 
```

<p align="center">
<i>Kernel code used to allocate memory for test 2.2. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/2b5a3e14-43cd-4dff-a08a-1c2e1b238525" alt="Screenshot of detected PCI devices for test 2.3.">
</p>

<p align="center">
<i>Screenshot of detected PCI devices for test 2.3. </i>
</p>


<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/98a1f750-cd2a-4ad8-9758-79f9c621f058" alt="Screenshot of QEMU monitor dump of emulated PCI devices for test 2.3.">
</p>

<p align="center">
<i>Screenshot of QEMU monitor dump of emulated PCI devices for test 2.3. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/e46ea115-b1e4-4836-b583-330056a5a52a" alt="Screenshot of reading initrd/afile.txt for test 2.4. ">
</p>

<p align="center">
<i>Screenshot of reading initrd/afile.txt for test 2.4.  </i>
</p>

```
Hello there!
```

<p align="center">
<i>Contents of initrd/afile.txt  </i>
</p>

```cpp
auto file = Filesystem::VirtualFilesystemManager::instance().open_file("initrd/afile.txt", 0); 
    auto fileLength = Filesystem::VirtualFilesystemManager::instance().filelen(file); 
    // Allocate a buffer to copy the contents of the file into. 
    // We add one to the length to add a null terminator. 
    auto fileBuffer = (char*) kmalloc(fileLength + 1, 0); 
    Filesystem::VirtualFilesystemManager::instance().read(file, 0, fileLength, fileBuffer); 
    // Add null terminator to string. 
    fileBuffer[fileLength] = '\0'; 
    printf("File contents: "); 
    printf(fileBuffer);
```

<p align="center">
<i>Kernel code used to read file for test 2.4. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/f997d6f1-cad0-42c3-8964-2339f0aec419" alt="Screenshot of reading initrd/dir/afile.txt for test 2.5. ">
</p>

<p align="center">
<i>Screenshot of reading initrd/dir/afile.txt for test 2.5. </i>
</p>

```cpp
auto file = Filesystem::VirtualFilesystemManager::instance().open_file("initrd/dir/afile.txt", 0); 
    auto fileLength = Filesystem::VirtualFilesystemManager::instance().filelen(file); 
    // Allocate a buffer to copy the contents of the file into. 
    // We add one to the length to add a null terminator. 
    auto fileBuffer = (char*) kmalloc(fileLength + 1, 0); 
    Filesystem::VirtualFilesystemManager::instance().read(file, 0, fileLength, fileBuffer); 
    // Add null terminator to string. 
    fileBuffer[fileLength] = '\0'; 
    printf("File in directory contents: "); 
    printf(fileBuffer);
```

<p align="center">
<i>Kernel code used to read file for test 2.5.  </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/645b46f7-1825-4122-85c1-7f3e446a97c4" alt="Screenshot of receiving keyboard events for test 2.6.  ">
</p>

<p align="center">
<i>Screenshot of receiving keyboard events for test 2.6. </i>
</p>

```cpp
auto eventDescriptor = Events::EventDispatcher::instance().register_event_listener(Task::TaskManager::instance().get_current_task(), "HID/Keyboard", 0); 
    while (1) 
    { 
        Events::EventDispatcher::instance().block_event_listen(Task::TaskManager::instance().get_current_task(), eventDescriptor); 
        auto scanCode = (uint8_t)Events::EventDispatcher::instance().read_from_event_queue(Task::TaskManager::instance().get_current_task(), eventDescriptor); 
        printf("Scan code received: "); 
        printf(scanCode); 
        printf("\n");
    }
```

<p align="center">
<i>Kernel code used to receive keyboard events for test 2.6.</i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/8fab77fc-8925-4b53-81fe-694225cf5c79" alt="Screenshot of Wireshark showing “Hello network!”
  transmitted on the network for test 3.1. ">
</p>

<p align="center">
<i>Screenshot of Wireshark showing “Hello network!”<br>
  transmitted on the network for test 3.1.  </i>
</p>


```cpp
AMDPCNETIIIDriver.send_data((uint8_t*)"Hello network!", 14); 
```

<p align="center">
<i>Kernel code used to transmit on network for test 3.1. </i>
</p>


<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/02d872e4-1a05-4dd2-94fb-4574baaa0e40" alt="Screenshot of Wireshark showing a compliant Ethernet II frame with the payload “Hello network!” transmitted on the network for test 3.2. ">
</p>

<p align="center">
<i>Screenshot of Wireshark showing a compliant Ethernet II frame with the payload<br>
  “Hello network!” transmitted on the network for test 3.2.  </i>
</p>

```cpp
// The MAC destination is set to 0xFFFFFFFFFFFF for a broadcast message. 
Networking::Ethernet::EthernetLayerManager::instance()->send_data(0xFFFFFFFFFFFF, 0, (uint8_t*)"Hello network!", 14); 
```

<p align="center">
<i>Kernel code used to transmit Ethernet II frame on network for test 3.2. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/24bf0d47-d064-4cb9-bcab-c5b938c2b785" alt="Screenshot of getting the MAC address for the gateway IP 10.0.0.2 for test 3.3. ">
</p>

<p align="center">
<i>Screenshot of getting the MAC address for the gateway IP 10.0.0.2 for test 3.3. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/cca26e6b-2b9e-4574-a27a-7cb5523f7729" alt="Screenshot of Wireshark showing compliant ARP request sent by the kernel and a response which is processed and cached for test 3.3. ">
</p>

<p align="center">
<i>Screenshot of Wireshark showing compliant ARP request sent by the kernel and a<br> response which is processed and cached for test 3.3.  </i>
</p>

```cpp
// Gateway IP. 

uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2; 

// Gateway IP big endian. 

uint32_t gip_be = ((uint32_t)gip4 << 24) 

                | ((uint32_t)gip3 << 16) 

                | ((uint32_t)gip2 << 8) 

                | (uint32_t)gip1;
                uint64_t MACAddress = 0; 

Networking::AddressResolutionProtocol::AddressResolutionProtocolManager::instance()->send_ARP_request(gip_be); 

while (!MACAddress) 

{ 

    MACAddress = Networking::AddressResolutionProtocol::AddressResolutionProtocolManager::instance()->find_cached_IP_address(gip_be); 

} 

printf("MAC address (big endian): "); 

printf(MACAddress); 
```

<p align="center">
<i>Kernel code used to resolve gateway IP address using ARP for test 3.3. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/202c7728-3db9-4442-85e1-644a2df6c729" alt="Screenshot of Wireshark showing a compliant IPv4 packet transmitted (other than protocol type) for test 3.4.  ">
</p>

<p align="center">
<i>Screenshot of Wireshark showing a compliant IPv4 packet transmitted <br> (other than protocol type) for test 3.4. </i>
</p>

```cpp
uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2; 

uint32_t gip_be = ((uint32_t)gip4 << 24) 

                | ((uint32_t)gip3 << 16) 

                | ((uint32_t)gip2 << 8) 

                | (uint32_t)gip1; 

Networking::InternetProtocolV4::InternetProtocolManager::instance()->send_data(gip_be, 0x0, (uint8_t*)"Hello network!", 14); 
```

<p align="center">
<i>Kernel code used to send IPv4 packet for test 3.4. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/f879e58e-9435-4406-81fd-49dc1b4bb040" alt="Screenshot of Wireshark showing a compliant UDP packet transmitted for test 3.5. ">
</p>

<p align="center">
<i>Screenshot of Wireshark showing a compliant UDP packet transmitted for test 3.5.  </i>
</p>

```cpp
uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2; 

uint32_t gip_be = ((uint32_t)gip4 << 24) 

                | ((uint32_t)gip3 << 16) 

                | ((uint32_t)gip2 << 8) 

                | (uint32_t)gip1; 

auto socket = udpManager->connect(gip_be, 1234); 

udpManager->send(socket, (uint8_t*)"Hello network!", 14); 
```

<p align="center">
<i>Kernel code used to transmit UDP packet for test 3.5.  </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/9f44d1dc-637e-472b-b34e-bd58f7de7bb2" alt="Screenshot of Wireshark showing a compliant UDP packet being received for test 3.6.  ">
</p>

<p align="center">
<i>Screenshot of Wireshark showing a compliant UDP packet being received for test 3.6.   </i>
</p>

```cpp
auto socket = udpManager->listen(1234); 
```

<p align="center">
<i>Kernel code used to receive UDP packet for test 3.6.  </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/1f135782-8240-481f-9c64-46beee7b9322" alt="Screenshot of Wireshark showing a compliant TCP connection when connecting to a remote endpoint for test 3.7. ">
</p>

<p align="center">
<i>Screenshot of Wireshark showing a compliant TCP connection when connecting<br> to a remote endpoint for test 3.7. </i>
</p>


```cpp
uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2; 

uint32_t gip_be = ((uint32_t)gip4 << 24) 

                | ((uint32_t)gip3 << 16) 

                | ((uint32_t)gip2 << 8) 

                | (uint32_t)gip1; 

auto socket = tcpManager->connect(gip_be, 1234); 

while (socket->m_state != Networking::TransmissionControlProtocol::TransmissionControlProtocolSocket::State::ESTABLISHED) {} 

socket->send((uint8_t*)"Hello network!", 14); 
```

<p align="center">
<i>Kernel code used to connect to a remote TCP endpoint for test 3.7. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/48a71b0a-bf22-4c53-8be1-78b9584c015d" alt="Screenshot of Wireshark showing a compliant HTTP POST request sent for test 3.8.  ">
</p>

<p align="center">
<i>Screenshot of Wireshark showing a compliant HTTP POST request sent for test 3.8.  </i>
</p>

```cpp
uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2; 

uint32_t gip_be = ((uint32_t)gip4 << 24) 

                | ((uint32_t)gip3 << 16) 

                | ((uint32_t)gip2 << 8) 

                | (uint32_t)gip1; 

auto socket = tcpManager->connect(gip_be, 3939); 

while (socket->m_state != Networking::TransmissionControlProtocol::TransmissionControlProtocolSocket::State::ESTABLISHED) {} 

socket-> send((uint8_t*)"POST /d493159e-39a2-4015-a5b7-cfcc05a9d739 HTTP/1.1\nHost: webhook.site\nAccept: application/json\nContent-Type: application/json\nContent-Length: 15\n\n{\"test\":\"data\"}", 163); 
```

<p align="center">
<i>Kernel code used to connect to a remote TCP endpoint for test 3.8.   </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/56bc1af3-c8ec-44fa-b9c0-6c4279c1222d" alt="Screenshot of kernel running multiple tasks for test 4.1.">
</p>

<p align="center">
<i>Screenshot of kernel running multiple tasks for test 4.1.  </i>
</p>

```cpp
void task_one() 
{ 
    printf("Task one\n"); 
    while (1); 
} 

void task_two() 
{ 
    printf("Task two\n"); 
    while (1); 
} 

void task_three() 
{ 
    printf("Task three\n"); 
    while (1); 
} 

void task_four() 
{ 
    printf("Task four\n"); 
    while (1); 
} 

auto task1 = Task::TaskManager::instance().create_new_kernel_task(); 
task1.set_entry_point((void*)& task_one); 
auto task2 = Task::TaskManager::instance().create_new_kernel_task(); 
task2.set_entry_point((void*)& task_two); 
auto task3 = Task::TaskManager::instance().create_new_kernel_task(); 
task3.set_entry_point((void*)& task_three); 
auto task4 = Task::TaskManager::instance().create_new_kernel_task(); 
task4.set_entry_point((void*)& task_four); 

Task::TaskManager::instance().add_task(task1); 
Task::TaskManager::instance().add_task(task2); 
Task::TaskManager::instance().add_task(task3); 
Task::TaskManager::instance().add_task(task4); 
Task::TaskManager::instance().run();
```

<p align="center">
<i>Kernel code to run multiple tasks for test 4.1. </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/42d2914d-a414-4afa-888b-2989269b3330" alt="Screenshot of kernel running different priority tasks for test 4.2. ">
</p>

<p align="center">
<i>Screenshot of kernel running different priority tasks for test 4.2. </i>
</p>


```cpp
void task_one() 
{ 
    printf("Task one\n"); 
    while (1); 
}  

void task_two() 
{ 
    printf("Task two\n"); 
    while (1); 
} 


void task_three() 
{ 
    printf("Task three\n"); 
    while (1); 
} 

void task_four() 
{ 
    printf("Task four\n"); 
    while (1);
} 


auto task1 = Task::TaskManager::instance().create_new_kernel_task(600); 
task1.set_entry_point((void*)& task_one); 
auto task2 = Task::TaskManager::instance().create_new_kernel_task(270); 
task2.set_entry_point((void*)& task_two); 
auto task3 = Task::TaskManager::instance().create_new_kernel_task(23); 
task3.set_entry_point((void*)& task_three); 
auto task4 = Task::TaskManager::instance().create_new_kernel_task(512); 
task4.set_entry_point((void*)& task_four); 

Task::TaskManager::instance().add_task(task1);
Task::TaskManager::instance().add_task(task2); 
Task::TaskManager::instance().add_task(task3); 
Task::TaskManager::instance().add_task(task4); 
Task::TaskManager::instance().run();
```

<p align="center">
<i>Kernel code to run multiple tasks for test 4.2.  </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/1fbbbaa7-9b65-4476-b18a-7440c4d92a81" alt="Screenshot of kernel receiving keyboard interrupts as all tasks block for test 4.3. ">
</p>

<p align="center">
<i>Screenshot of kernel receiving keyboard interrupts as all tasks block for test 4.3.  </i>
</p>

```cpp
void task_one() 
{ 
    printf("Task one\n"); 
    Task::TaskManager::instance().block_task(); 
} 

void task_two() 
{ 
    printf("Task two\n"); 
    Task::TaskManager::instance().block_task(); 
} 

void task_three() 
{ 
    printf("Task three\n"); 
    Task::TaskManager::instance().block_task(); 
} 

void task_four() 
{ 
    printf("Task four\n"); 
    Task::TaskManager::instance().block_task(); 
} 

auto task1 = Task::TaskManager::instance().create_new_kernel_task(600); 
task1.set_entry_point((void*)& task_one); 
auto task2 = Task::TaskManager::instance().create_new_kernel_task(270); 
task2.set_entry_point((void*)& task_two); 
auto task3 = Task::TaskManager::instance().create_new_kernel_task(23); 
task3.set_entry_point((void*)& task_three); 
auto task4 = Task::TaskManager::instance().create_new_kernel_task(512); 
task4.set_entry_point((void*)& task_four); 

Task::TaskManager::instance().add_task(task1); 
Task::TaskManager::instance().add_task(task2); 
Task::TaskManager::instance().add_task(task3); 
Task::TaskManager::instance().add_task(task4); 
Task::TaskManager::instance().run(); 
```

<p align="center">
<i>Kernel code to run multiple tasks for test 4.3.  </i>
</p>

<p align="center">
<img width="700" alt="Screenshot 2024-08-19 at 11 31 53" src="https://github.com/user-attachments/assets/9d397584-3f93-4ae1-9c9a-e0471ce95099" alt="Screenshot of user-space program executing a system call for test 4.4. ">
</p>

<p align="center">
<i>Screenshot of user-space program executing a system call for test 4.4.  </i>
</p>

```cpp
void elf_task() 
{ 
    Task::TaskManager::instance().lock(); 
    auto currentTask = Task::TaskManager::instance().get_current_task_pointer(); 
    Loader::prepare_elf("initrd/userspaceProgram", currentTask); 
    auto entryPoint = currentTask->get_entry_point(); 
    Task::TaskManager::instance().unlock(); 
    start_userspace_task_sysret((uint64_t)entryPoint, (uint64_t)(currentTask->get_kstack())); 

} 
```

<p align="center">
<i>Kernel code to load ELF executable and run in user-space for test 4.4. </i>
</p>

```cpp
void _start() 
{ 
    syscall(SYSCALL_DPRINTF, (uint64_t)"Hello from userspace!"); 
    while(1); 
} 
```

<p align="center">
<i>ELF executable source code for test 4.4. </i>
</p>

```cpp
void _start() 
{ 
    asm volatile ("cli"); 
    while (1); 
}
```

<p align="center">
<i>ELF executable source code that causes General Protection<br> Fault for test 4.5.  </i>
</p>

```cpp
void task_one() 
{ 
    while (1) 
    { 
        printf("Task one. "); 
        Task::TaskManager::instance().sleep_for(1000); 
    } 
} 
```

<p align="center">
<i>Kernel code that prints to screen roughly every second for test 4.6.</i>
</p>

## Potential Improvements 

I have a few ideas on potential improvements to this project from my own reflection, which I will now discuss. 

- **Graphical User Interface (GUI)**: Providing a GUI would massively improve how users interact with the operating system. Currently, there is virtually no interactivity other than loading different programs. Using modern HD graphics would be ideal, however this would require implementing graphics drivers for each graphics card and this is infeasible. Instead, implementing HD graphics for the VMWare SVGA II card, which is a virtual graphics card that is available in most virtual machines, would likely be more than sufficient for people using an educational operating system.

- **Multiple Processors**: Modern operating systems utilise multiple processors (or cores) to execute processes or threads simultaneously as opposed to only using pre-emptive task switching. Implementing this may require a redesign of the scheduler and interrupts. However, locking primitives such as the spinlock are already functional with multiple processors, making much of the kernel and its data structures ready for multiple processors. This could be implemented with Symmetric Multiprocessing (SMP).

- **Improving system calls and porting a C library**: The system calls act as an interface between the user-space and the kernel, where the user-space can request services. Increasing the number of system calls means that user-space programs can provide more functionality. For example, a common system call is the mmap call which is used to map memory. With enough system calls, a C library can be ported that abstracts the system calls, leaving user-space programs to use the standardised C library.

- **Custom bootloader**: At the moment, we use the GRUB bootloader to load the operating system and Multiboot-2 modules into memory. However this restricts the design of how the kernel boots and makes us reliant on the bootloader. With a custom bootloader, we would have complete control over how our operating system is loaded and we could write a custom installer for our operating system.


## Resources

1 - OSDev Wiki (https://osdev.wiki/wiki/Expanded_Main_Page)

2 - Operating System Concepts, 10th edition (Abraham Silberschatz, Greg Gagne, and Peter Baer Galvin)
