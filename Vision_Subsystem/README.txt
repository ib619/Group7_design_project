# Vision Subsystem

## Running it
1. Connect the FPGA and the esp32.
2. Plug in the JTAG UART cable and the vga cable to the vga capture.
3. Open Quartus Programmer to flash the sof file. The latest files can be found in Vision_Subsystem\sof_elf_files folder
4. Flash the Elf file onto the FPGA. There are two methods of doing this:
- Open NiosII Software Build Tools for Eclipse to build and flash the elf file onto the FPGA. This is needed when there is a need 
to tweak the gain and shutter speed. Send 'e' via the NiosII Command to decrease shutter speed (Allow more light into the video) 
and 't' to increase gain.
- Do it via command line using nios2-download (Do remember to include the -g flag)

## Debugging
1. Download Quartus 16.1 (This is required for local compilation as some of the IPs used in this project 
is no longer available in later versions). Quartus is needed to compile the hardware and this will produce a 
sof file in the Vision_Subsystem\DE10_LITE_D8M_VIP_16\output_files folder.

2. Use NiosII Software Build Tools for Eclipse (Found in Tools section of Quartus) to make edits to the 
software C code that will be uploaded onto the NiosII processor.

3. To recompile the hardware on Quartus, you will need to update the file path for the gaussian coefficient file in the platform designer.
To do this, open Qsys (Found under Tools section of Quartus) and go to alt_vip_cl_2dfir_0 and update the full path of gaussiancoeffs_5.txt.
Generate HDL and once that is done, compile the hardware.