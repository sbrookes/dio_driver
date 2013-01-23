
Full documentation is included in the source. If time permits,
     design spec will be replicated here for the benefit of 
     future users and maintenence.

The reason for writing this now is to stress that user-land programs
    wanting to write to the DIO card sould use the open, read, write,
    close family of SYSCALLS directly, rather than using a wrapper
    like fread or fwrite. These wrappers mess with parameters before 
    invoking the syscall, behavior that is unacceptable for this device.

