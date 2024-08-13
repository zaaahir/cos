# cOS

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

### Hierarchy Charts

TBD
