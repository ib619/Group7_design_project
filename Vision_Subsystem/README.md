# Vision Subsystem

## Folder Structure
```
Vision_Subsystem
│   README.md  
│
└───DE10_LITE_D8M_VIP_16                     
│   └───ip            
|   |   |   └───EEE_IMGPROC   
|   |   |   |   └───colour_threshold 
|   |   |   |   └───gaussian_filter
|   |   |   |   └───line_buffers
|   |   |   |   └───morph_filters
|   |   |   |   └───perspective_transform
|   |   |   |    ...
|   |   |   └───i2c_opencores  
|   |   |   └───TERASIC_AUTO_FOCUS
|   |   |   └───TERASIC_CAMERA
|   |   |   i2c_adapter.v     
|   |
|   └───software       
|   |   |   └───D8M_Cam_v2   
|   |   |   └───D8M_Camera_Test_bsp
|   |   |   ...            
|   |   ...
|   ...
|                 
└───doc      
|            
└───sof_elf_files --> Contains various iterations of the SOF and ELF files      
|
└───test
    └───colour_threshold
    └───gaussian_filter
    └───hsv_hist --> Histogram plotting of the Coloured Balls
    └───images --> Contains all the testing images
    └───morph_filter
    └───perspective_transform
```
## Capabilities
1. 5x5 Gaussian Filter  (Separable Convolution)
2. RGB to HSV conversion
3. Erosion Filter
4. Dilation Filter
5. Perspective Transform Mapping

## Testing
The tests have been arranged based on their respective capabilities. Each capabilities contain Python scripts that use OpenCV and Skimage to run test on images captured from the VGA output of the FPGA. The Altera-Modelsim simulation for the verilog implementation is also contained in those folders.

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
