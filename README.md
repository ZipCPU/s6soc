# Description

This CMOD-S6 SoC grew out of the desire to demonstrate that a useful ZipCPU
soft core implementation could be made in a very small space.  In 
particular, one of the purposes of the ZipCPU was to be able to operate successfully in a very area-challenged environment.  The CMOD-S6, as sold by Digilent
Inc., provides this environment for this project.

# The CPU

For those not familiar with the ZipCPU, it is a soft core CPU designed
specifically for small area implementations.  The CPU is a full 32-bit CPU,
designed as a RISC load/store architecture, having a full set of thirty-two
32-bit registers (of which 16 may be used at any one time), and has a single
wishbone bus for both instructions and data (Von Neumann architecture).  The
particular implementation of the ZipCPU used for this SoC project is not
pipelined, nor does it have either instruction or data caches--they simply
wouldn't fit within the FPGA.  Still, a CPU is a CPU and this CPU will 
execute the instructions given to it faithfully.

# Peripherals

A SoC is really a soft core CPU combined with a bus, giving the CPU access to
a variety of peripherals.  In this case, the CMod-S6 SoC offers the user with the following peripherals:

1. An I/O space containing
  a. an interrupt controller
  b. the address of the last bus error
  c. a system timer
  d. a watchdog timer
  e. an audio controller consisting of a PRM driver and another (supporting) timer
  f. a GPIO controller capable of implementing SPI and I2C (SPI is working, as this is used to drive the display successfully)
  g. UART Rx/Tx
  h. support for the on-board LED's and buttons, as well as for ...
  i. an external 16-character keypad controller.
2. A debug scope, capable of recording 1024 words of debugging information within the core upon any trigger.
3. A 16-kB On-chip block RAM
4. 16-MB flash for holding both the FPGA configuration as well as any user programs.  (The configuration takes about 512kB of flash.)

All of these peripherals have been tested, and they are known to work.

# The Demo Task

This board will be (has been!) proven with the (imaginary) task of implementing
a security light for a home.  The light works in this fashion: when someone
presses the doorbell (one of the on-board buttons), the system will then play
a doorbell sound on the audio port, and turn on the outdoor lights for a half
an hour.  Further, the keypad will allow a user to set the current time, and
set times when the outdoor lights should not be turned on (i.e., during the
daytime).  Finally, the GPIO pins will be used to control a 2-line display that
will show either a blank screen (if not being used), the time of the last
doorbell press, or a menu driven screen for use with the keypad.

The UART will be (has been) used primarily as a debug port, both to output
current status (ala debug by printf), as well as to allow access to a second
S6 configuration which can be used for programming the flash.

# Current Status

20160523: I am going to place this project down in my "done" category of
projects.  It currently does all that I have asked of it and all that I intended
the project to do.  Please feel free to write if you have comments, thoughts,
questions, or even suggestions.

20170126: I'm in the process of updating the project to work with the newer version of the ZipCPU--the one that can handle the more traditional 8-bit bytes, rather than the 32-bit bytes the original ZipCPU could only handle.

20170309: All of the prior ZipOS functionality now works (again) using the new ZipCPU.

20170321: The CPU can now execute instructions from flash in about 20 clocks
per instruction--a number which includes the 8-20 clocks just to read from the
flash.  Further, because this uses a simpler flash controller, and a simpler
set of UART controllers, the whole CPU takes even fewer LUTs than before.

