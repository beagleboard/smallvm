# README #

## MicroBlocks Website ##

This repository contains the source code for MicroBlocks.
To learn more about MicroBlocks and how to use it please visit the MicroBlocks website,
<http://microblocks.fun>

## What is MicroBlocks? ##

[MicroBlocks](http://microblocks.fun) is a free, live, blocks programming system
for educators, learners, and makers that its creaters hope will become "the Scratch
of physical computing." It runs on the BBC
micro:bit, Calliope mini, AdaFruit CircuitPlaygraound Express,
NodeMCU, ESP32, many ARM-based Arudino boards, and
other tiny computers such as the Raspberry Pi.

The MicroBlock firmware (or *virtual machine*) runs on 32-bit embedded processors
with as little as 16k of RAM. It is intended to be simple,
easily ported and extended, and offer decent performance.
It includes a low-latency task scheduler that works
at timescales down to about 30 microseconds.
It will eventually include a garbage collected memory
to allow working with dynamic data structures -- within
the limits of the available RAM, of course!

MicroBlocks supports both *live* development and *autonomous operation*.
Live development allows the user to see the results of program changes
immediately, without the overhead of compiling and downloading.
Autonomous operation means that programs continue to run when the board
is untethered from the host computer. Because MicroBlocks programs run
directly on the microcontroller -- in contrast to tethered systems where
the program runs on a laptop and uses the microcontroller as an I/O device --
programs can work precisely at timescales down to 10's of microseconds.
For example, MicroBlocks can generate musical tones and servo waveforms
and it can analyze incoming signals such as those from an infrared remote
control.

Finally, since MicroBlocks stores the user's program
in persistent Flash memory, MicroBlocks will eventually
allow the user's program stored on the board to be read back into a
MicroBlocks development environment for inspection and
further development.

MicroBlocks supports user-defined blocks (commands and functions).
That mechanism is used to create MicroBlocks libraries, so advanced
users can explore the implementation of libraries learn how they work
or even extend them.

Built-in data types include integers, booleans, strings, and lists.

## How do I build the MicroBlocks firmware? ##

First of all, you may not need to. The firmware for many boards, including the BBC micro:bit,
Calliope mini, AdaFruit Circuit Playground Express, Citilab ED1, M5Stack, NodeMCU,
and ESP32 Dev board is can be installed using the "update firmware on board" menu
command in the interactive development environment (IDE).

However, if you have one of the other
[supported boards](https://bitbucket.org/john_maloney/smallvm/wiki/Devices),
or if you just want to build the firmware yourself, read on.

The MicroBlocks firmware, also called the **virtual machine**, is written in C and C++.
It is built on the Arduino platform and uses additional Arduino libraries for
features such as graphics and WiFi (on boards that support those features).

The source code repository is [here](https://bitbucket.org/john_maloney/smallvm/src/master/). To get the source code, you can either clone it:

    git clone https://john_maloney@bitbucket.org/john_maloney/smallvm.git

or download a snapshot of it by selecting "download repository" from the "..." menu to the right
of the "Clone" button on the repository home page.

### Building with PlatformIO ###

PlatformIO is the preferred build tool. PlatformIO is a cross platform build system
that is easy to install an use. It's only dependency is Python, which comes pre-installed
on MacOS and Linux and is easy to install on Windows. You can get the PlatformIO command
line interface (CLI) tools [here](https://platformio.org/install/cli).

To compile the firmware for all platforms, just enter the "smallvm" folder and run:

    pio run

To compile and install the VM for a particular board (e.g. the BBC micro:bit),
plug in the board and run:

    pio run -e microbit -t upload

### Building with the Arduino IDE ###

The MicroBlocks virtual machine can also be compiled and loaded onto a board using the
Arduino IDE (version 1.8 or later) with the appropriate board package and libraries
installed.

To build with the Arduino IDE, open the file *vm.ino*, select your board from the
boards manager, then click the upload button.

### Building for Raspberry Pi ###

To build the VM on the Raspberry Pi, run "./build" in the raspberryPi folder.
The Raspberry Pi version of MicroBlocks can control the digital I/O
pins of the Raspberry Pi.

On the Raspberry Pi, MicroBlocks can be run in two ways:

Desktop: If you run ublocks-pi with the "-p" switch,
it will create a pseudoterminal, and you can connect to that pseudoterminal
from the MicroBlocks IDE running in a window on the same Raspberry Pi.

Headless: If you configure your RaspberryPi Zero(W) to behave like a slave USB-serial
device, then it can be plugged into a laptop as if it were an Arduino or micro:bit.
The MicroBlocks VM can be started on a headless Pi either by connecting
to the Pi via SSH and running ublocks-pi from the command line
or by configuring Linux to start the VM at boot time.

### Building for generic Linux ###

To build the VM to run on a generic Linux computer, run "./build" in the linux folder.
Running the resulting executable creates a pseudoterminal that you can connect
to from the MicroBlocks IDE running on the same computer.

The generic Linux version of the VM can't control pins or other microcontroller I/O devices
(there aren't any!), but it can be used to study the virtual machine.
It's also handy for debugging, since the Linux VM can print debugging information
to stdout without interfering with the VM-IDE communications (which goes over
the pseudoterminal connection).

## MicroBlocks IDE ##

The MicroBlocks IDE is written in [GP Blocks](https://gpblocks.org). You'd find source code for the IDE is in the ide
and the gp libraries and build scripts in the the gp folder.

## Status ##

Although MicroBlocks is currently "alpha", it is stable and has been used by hundreds of people,
many of them complete beginners. The "alpha" status is because MicroBlocks is still evolving.
If you are writing documentation, you should to update it to track changes to the libraries, blocks, and UI.

We hope to enter "beta" by early 2020.

## Contributing ##

We welcome your feedback, comments, feature requests, and
[bug reports](https://bitbucket.org/john_maloney/smallvm/issues?status=new&status=open).

Since MicroBlocks is still under active development by the core team, we are not currently
soliciting code contributions or pull requests. However, if you are creating tutorials or other materials for MicroBlocks, please let us know so we can link to your website.

## Team ##

This project is a collaboration between John Maloney, Bernat Romagosa, and Jens Moenig,
with input and help from many others. It is under the fiscal sponsorship of the
[Software Freedom Conservancy](https://sfconservancy.org), a 501(c)(3) non-profit.

## License ##

MicroBlocks is licensed under the Mozilla Public License 2.0 (MPL 2.0).
