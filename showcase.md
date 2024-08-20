# cOS - A Modern Operating System

Read [the main README](https://github.com/zaaahir/cos/README.md) for more details on technicals and specifics, this is just a brief overview!

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

 - The kernel boots successfully.
   - The kernel boots on x86-64 compatible machines.
   - The kernel is bootable by any Multiboot-2 compliant bootloader. 
 - The kernel can parse a Multiboot-2 header.
 - The kernel sets up paging.
   - 4-level paging
 - The kernel can load a Multiboot-2 module.
 - The kernel sets up x86-64 specific structures:
   - A Global Descriptor Table (GDT)
   - A Task State Segment (TSS)
   - An Interrupt Descriptor Table (IDT)
 - The kernel enters long mode.
 - The kernel loads an initrd.
 - The initrd is loaded as a Multiboot-2 module.
 - The initrd is a Unix Standard TAR file.
 - The kernel initialises interrupts.
 - The kernel initialises the Programmable Interval Timer (PIT).
 - The kernel initialises a kernel heap that can allocate and free arbitrarily sized buffers.
   - It must be able to supply word-aligned allocations.
 - The kernel initialises a scheduler that can switch between kernel tasks.
   - The kernel must also provide a mechanism to switch to user mode. (ring 3)
 - The kernel detects devices on Peripheral Component Interconnect (PCI) buses.
   - The kernel provides the ability to search for and configure devices on PCI buses.

### cOS filesystem layer

 - The layer provides an interface for multiple filesystems mounted in a single virtual filesystem.
   - It provides an implementation for a driver for the USTAR tar file format.
 - The layer provides `read` and `write` interfaces for filesystem drivers.

### cOS networking layer

 - The kernel can provide networking services.
   - It provides an adaptable interface for different network card drivers.
     - It provides an implementation for the AMD Am79C973 network card.
   - It provides an interface for sending and receiving Ethernet frames.
   - It provides an interface for sending and receiving Address Resolution Protocol (ARP) messages.
   - It provides an interface for handling Internet Protocol v4 (IPv4) messages.
   - It provides an interface for sending and receiving Internet Control Message Protocol (ICMP) messages.
   - It provides an interface for sending and receiving User Datagram Protocol (UDP) messages.
   - It provides an interface for sending and receiving Transmission Control Protocol (TCP) messages.
     - It supports reordering of received packets.
     - It supports acknowledgment and retransmission of missed packets.

### cOS user-space

- [ ] - The kernel can load ELF executables into an address space for a newly created task.
- [ ] - The kernel provides a system calls interface to send data to the kernel.

