Introduction
============
In this assignment, you will be implementing the device drivers for the HAWX
operating system. Recall the basic layout of the HAWX system:

    +------------+                +-----------+
    |  Computer  |     UART       | SERIAL    |
    |    CPU     | <------------> |  TERMINAL |
    |    RAM     |                |           |
    +------------+                +-----------+
          ^                      / .:::::::. /
          | VIRTIO               ------------
          |
        __V___ 
       | HARD |
       | DISK | 
        ------

There are thus two devices that we need to communicate with: 

1.) The Universal Asynchronous Receiver/Transmitter (UART) device, which is
    attached to a serial terminal.
2.) The VIRTIO block device, which is attached to a hard disk.

These devices are controlled by the kernel via memory mapped registers.
The exact properties of these devices are determined by their manufacturers,
and so we must read the device's documentation to understand how to communicate
with them. While these are technically simulated devices in qemu, they 
do operate like their real counterparts! Their specifications can be found here:
  - UART: https://uart16550.readthedocs.io/en/latest/index.html
  - VIRTIO: https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.html

As you have no doubt noticed, these documents are not the friendliest to read.
In fact, reading these documents without some sort of context would be 
pretty much useless. They are really just a reference for the device's
registers and their meanings. To provide that context, we will turn to the xv6
source code, in particular these files:
  - `kernel/uart.c`
  - `kernel/virtio_disk.c`
Reading over this code, and then looking up corresponding registers in the manual,
will make your life easier while doing this assignment!


Device Interrupts
=================
Both of these devices are inherently slow. It takes a few milliseconds for 
either device to respond to a command. Because this is an eternity in CPU
time-scales, we don't want to wait for each device transaction to complete.
Instead, we want to do something that looks more like this:
  1. Send a command to the device.
  2. Move on to other work.
  3. Let the device interrupt us when it is done.
The device interrupts are handled by the kernel's trap handler, which we 
cover in another assignment. For now, we will just assume that the kernel
responds to these interrupts by calling the appropriate device driver
function.

The UART device's interrupt function is `uartintr` in `kernel/uart.c`, and 
the VIRTIO block device's interrupt function is `virtio_disk_intr` in
`kernel/virtio_disk.c`. These functions are triggered by the following events:
  - UART: When a character is received from the serial terminal.
  - UART: When a character has completed transmission to the serial terminal.
  - VIRTIO: When a disk operation is completed.
As we set out writing the device drivers, it is important that you maintain 
this sort sequence of "start, move on, interrupt" when dealing with 
these devices.


Implementing the UART Driver
===========================
The UART device is the simplest device to implement, and it also happens
to be the most important to start with. To see why, try compiling and running
the program as is:

```
$ make qemu
...
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 
1 -nographic -global virtio-mmio.force-legacy=false -drive file=disk.img,if=none
,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
```

You see that it will compile and run, but the system says nothing. That
stands to reason considering that it can't! Poor thing can't even panic.
We should help it out. Have a look a the `uart.c` file. You need to fill
in the drivers. We'll start by implementing functions to allow the 
driver to work in a non-interrupt driven way. This will allow us to
at least see a little input. 

Implement the following functions:
- `uartinit`: Initialize the UART device.
- `uartputc`: Write a character to the UART device.
- `uartflush`: Forcibly write from `PORT_CONSOLEOUT` to the UART device.

Be sure to read the comments in this file (and others) for hints about
how to write these functions. A lot of code can be appropriated from
xv6, so you'll also want to keep that handy.

Once you have implemented these, if you run the system, you should see some
output:

```
$ make qemu
...
HAWX kernel is booting

UART initialization test...PASSED
UART flush test...PASSED
```

At this point, the system hangs. That's because the UART device is not
handling interrupts, and the next test in the list is to test the
interrupt handler. Let's fix that!

Implement the following functions:
- `uartintr`: Handle an interrupt from the UART device.
- `uartstart`: Start a write to the UART device.

Now, if you run the system, you'll get the following:

```
$ make qemu
...

HAWX kernel is booting

UART initialization test...PASSED
UART flush test...PASSED
Interrupt driven output test...PASSED
Type the word "PASSED" and press enter: 
```

We're almost there! Of course, you've probably noticed that you can't
type. If you followed the hints in `uartintr`, you would have implemented
the thing that would do the echoing. The thing is, we have not yet 
implemented the function that retrieves characters from the UART device.
That's the last thing we need to do. Implement:
- `uartgetc`: Read a character from the UART device.

Now, if you run the system, you'll get the following:

```
$ make qemu
.
HAWX kernel is booting

UART initialization test...PASSED
UART flush test...PASSED
Interrupt driven output test...PASSED
Type the word "PASSED" and press enter: PASSED
Interrupt driven input test...PASSED
Writing to disk.....
```

Note that you'll have to type the word "PASSED" and press enter. If you 
mistype it, the test will fail. The final test verifies that you are 
actually using interrupts and not cheating using busy-waiting. Have a look
in `tests.c` if you want to see the sorcery behind this test.

And that's it! You've implemented the UART driver. Now let's move on to
the disk driver. 


Implementing the VIRTIO Block Device Driver
===========================================
The VIRTIO block device is a little more complicated than the UART device,
though the description here will be a little simpler. The reason for this
is that the disk is pretty much an "all or nothing" proposition. Instead,
let's focus on a narrative of what the driver should do, and then you
can set about grabbing and modifying the code to make it happen!

The key things to watch out for are the differences between our driver 
and the xv6 driver. xv6 uses a buffer-cache scheme which folds the 
file system into the kernel. We are not doing that. Instead, we are using
the `PORT_DISKCMD` port to send commands to the disk. The message format 
is as follows:

    +-+-------+----+----+
    |M|BLOCKID|DATA|MSG |
    +-+-------+----+----+
    M        - 1 Character R for read, W for write
    Block ID - 7 Characters Decimal Block ID to operate on
    Data     - 4 Characters Decimal Port to use as block buffer
    Message  - 4 Characters Decimal Port to write response message

This message is processed by the `get_disk_msg` function. It may be a good idea
to go ahead and implement this now.

The disk driver will respond to these messages as each command completes. The
response messages have the following format:

   +-+-+-------+
   |M|S|BLOCKID|
   +-+-+-------+
   M        - 1 Character, echoing the specified mode.
   S        - 1 Character, S for success, F for failure
   Block ID - 7 Characters Decimal Block ID to operate on

These messages are written by the `write_disk_response` function. Now, 
the question you should be asking yourself is "From where do we get the
information for the message?" The answer is from the `disk.info` array. The
`id` parameter is your index into this array. Go ahead and implement this 
function.

Now, let's go ahead and implement the rest of the driver. Here is the
order I suggest you explore these functions. 

1. `virtio_disk_init`
2. `virtio_disk_start`
3. `virtio_disk_intr` 

Be sure to read the code and comments very carefully so that you can get the
correct behavior out of the driver. Pay close attention to the differences
in this file versus what you will see in xv6. This will give you strong
hints about how to proceed!

When you get it working, you should see the following when you run the 
system:
```
$ make qemu
...

HAWX kernel is booting

UART initialization test...PASSED
UART flush test...PASSED
Interrupt driven output test...PASSED
Type the word "PASSED" and press enter: PASSED
Interrupt driven input test...PASSED
Writing to disk...PASSED
Reading from disk...PASSED
Empty port disk write...PASSED
Partial port disk write...PASSED
Non-empty port disk read...PASSED
Descriptor exhaustion test...PASSED
panic: All done! For now...
```

The first two disk tests are about the basic ability to successfully read 
and write to and from the disk. The others test details. For example, you
need to verify the ports you are using in read and write operations. You 
also need to make sure that you deallocate allocated descriptors for failed
operations. If the last test hangs, you are probably not deallocating
the descriptors properly.

Submission
==========
Once all the tests complete successfully, run the command:

```
make turnin
```

This will create the file `hawx-drivers-turnin.tar.gz` which you can submit
via Canvas.
