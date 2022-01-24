# Data Aquisition Sock Project
Firmware and accompanying processing and visualization software for the data aquisition sock capstone project being conducted by Gary Huang, Yao Li, Joby Matwick, and Jason Zhang.

## Firmware
PlatformIO is used to manage tools and libraries for the embedded firmware, and handles project building. Steps for getting this setup are included below.

### Environment Setup
1. Download and install [VSCode](https://code.visualstudio.com/).
2. Install the [PlatformIO IDE VSCode extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) and wait for the install to complete. Restart VSCode when prompted.
3. Clone this repo to a location on your system (`git clone https://github.com/jobymatwick/DataSock.git`).
4. Open the cloned repo as a folder in VSCode. The PlatformIO extension will identify the project and begin downloading all needed tools and libraries (This can take a few minutes).
5. Once complete, check that everything is working by doing build of the project. Either with `Ctrl-Alt-B` or by pressing the checkmark button at the left of the bottom toolbar in VSCode.

### Embedded Console
A console interface is exposed over the USB-Serial interface on the Teensy to facilitate debugging and development. The [command listing](console-commands.md) document contains a list of all implemeted commands.
