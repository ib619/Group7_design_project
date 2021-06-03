#include <stdio.h>
#include "I2C_core.h"
#include "esp_link.h"
#include "terasic_includes.h"
#include "mipi_camera_config.h"
#include "mipi_bridge_config.h"

#include "auto_focus.h"

#include <fcntl.h>
#include <unistd.h>

//EEE_IMGPROC defines
#define EEE_IMGPROC_MSG_START ('R'<<16 | 'B'<<8 | 'B')

//offsets
#define EEE_IMGPROC_STATUS 0
#define EEE_IMGPROC_MSG 1
#define EEE_IMGPROC_ID 2
#define EEE_IMGPROC_BBCOL 3

#define EXPOSURE_INIT 0x002000
#define EXPOSURE_STEP 0x100
#define GAIN_INIT 0x380
#define GAIN_STEP 0x080
#define DEFAULT_LEVEL 3

#define MIPI_REG_PHYClkCtl		0x0056
#define MIPI_REG_PHYData0Ctl	0x0058
#define MIPI_REG_PHYData1Ctl	0x005A
#define MIPI_REG_PHYData2Ctl	0x005C
#define MIPI_REG_PHYData3Ctl	0x005E
#define MIPI_REG_PHYTimDly		0x0060
#define MIPI_REG_PHYSta			0x0062
#define MIPI_REG_CSIStatus		0x0064
#define MIPI_REG_CSIErrEn		0x0066
#define MIPI_REG_MDLSynErr		0x0068
#define MIPI_REG_FrmErrCnt		0x0080
#define MIPI_REG_MDLErrCnt		0x0090

void mipi_clear_error(void){
    MipiBridgeRegWrite(MIPI_REG_CSIStatus,0x01FF); // clear error
    MipiBridgeRegWrite(MIPI_REG_MDLSynErr,0x0000); // clear error
    MipiBridgeRegWrite(MIPI_REG_FrmErrCnt,0x0000); // clear error
    MipiBridgeRegWrite(MIPI_REG_MDLErrCnt, 0x0000); // clear error

    MipiBridgeRegWrite(0x0082,0x00);
    MipiBridgeRegWrite(0x0084,0x00);
    MipiBridgeRegWrite(0x0086,0x00);
    MipiBridgeRegWrite(0x0088,0x00);
    MipiBridgeRegWrite(0x008A,0x00);
    MipiBridgeRegWrite(0x008C,0x00);
    MipiBridgeRegWrite(0x008E,0x00);
    MipiBridgeRegWrite(0x0090,0x00);
}

void mipi_show_error_info(void){

    alt_u16 PHY_status, SCI_status, MDLSynErr, FrmErrCnt, MDLErrCnt;

    PHY_status = MipiBridgeRegRead(MIPI_REG_PHYSta);
    SCI_status = MipiBridgeRegRead(MIPI_REG_CSIStatus);
    MDLSynErr = MipiBridgeRegRead(MIPI_REG_MDLSynErr);
    FrmErrCnt = MipiBridgeRegRead(MIPI_REG_FrmErrCnt);
    MDLErrCnt = MipiBridgeRegRead(MIPI_REG_MDLErrCnt);
    printf("PHY_status=%xh, CSI_status=%xh, MDLSynErr=%xh, FrmErrCnt=%xh, MDLErrCnt=%xh\r\n", PHY_status, SCI_status, MDLSynErr,FrmErrCnt, MDLErrCnt);
}

void mipi_show_error_info_more(void){
    printf("FrmErrCnt = %d\n",MipiBridgeRegRead(0x0080));
    printf("CRCErrCnt = %d\n",MipiBridgeRegRead(0x0082));
    printf("CorErrCnt = %d\n",MipiBridgeRegRead(0x0084));
    printf("HdrErrCnt = %d\n",MipiBridgeRegRead(0x0086));
    printf("EIDErrCnt = %d\n",MipiBridgeRegRead(0x0088));
    printf("CtlErrCnt = %d\n",MipiBridgeRegRead(0x008A));
    printf("SoTErrCnt = %d\n",MipiBridgeRegRead(0x008C));
    printf("SynErrCnt = %d\n",MipiBridgeRegRead(0x008E));
    printf("MDLErrCnt = %d\n",MipiBridgeRegRead(0x0090));
    printf("FIFOSTATUS = %d\n",MipiBridgeRegRead(0x00F8));
    printf("DataType = 0x%04x\n",MipiBridgeRegRead(0x006A));
    printf("CSIPktLen = %d\n",MipiBridgeRegRead(0x006E));
}

bool MIPI_Init(void){
    bool bSuccess;

    bSuccess = oc_i2c_init_ex(I2C_OPENCORES_MIPI_BASE, 50*1000*1000,400*1000); //I2C: 400K
    if (!bSuccess)
        printf("failed to init MIPI- Bridge i2c\r\n");

    usleep(50*1000);
    MipiBridgeInit();

    usleep(500*1000);

    MipiCameraInit();
    MIPI_BIN_LEVEL(DEFAULT_LEVEL);
    usleep(1000);

    return bSuccess;
}
alt_16 estimate_dist(alt_16 y_coord){
    if (y_coord < 330){
        return 200 ;
    } else if  (y_coord < 345){
        return -3 * (y_coord - 330) + 200;
    } else 	if (y_coord < 365){
        return -2 * (y_coord - 330) + 185;
    } else 	if  (y_coord < 430){
        return -( y_coord-330)  + 150;
    } else {
        return 50;
    }
}
alt_8 estimate_angle(alt_16 x_coord){
    // Max Range +- 20, so each pixel will be a shift in 0.0625 degree
    return (x_coord >> 4 )+ (x_coord >> 7) - 22;
}



int main()
{
    //////////////////////////////////////////////////////////////////
    // Initialising Camera
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    printf("DE10-LITE D8M VGA Demo\n");
    printf("Imperial College EEE2 Project version\n");
    IOWR(MIPI_PWDN_N_BASE, 0x00, 0x00);
    IOWR(MIPI_RESET_N_BASE, 0x00, 0x00);

    usleep(2000);
    IOWR(MIPI_PWDN_N_BASE, 0x00, 0xFF);
    usleep(2000);
    IOWR(MIPI_RESET_N_BASE, 0x00, 0xFF);

    printf("Image Processor ID: %x\n",IORD(0x42000,EEE_IMGPROC_ID));

    usleep(2000);


    // MIPI Init
    if (!MIPI_Init()){
        printf("MIPI_Init Init failed!\r\n");
    }else{
        printf("MIPI_Init Init successfully!\r\n");
    }

    mipi_clear_error();
    usleep(50*1000);
    mipi_clear_error();
    usleep(1000*1000);
    mipi_show_error_info();
    printf("\n");

    alt_u16 bb [10][4];
    alt_u16 bin_level = DEFAULT_LEVEL;
//    alt_u8  manual_focus_step = 10;
    alt_u16  current_focus = 300;
//    int boundingBoxColour = 0;
    alt_u32 exposureTime = EXPOSURE_INIT;
    alt_u16 gain = GAIN_INIT;
    alt_u8 time_delay = 0;

    alt_u16 x, y;
    alt_u32 area1, area2;
    alt_8 angle;
    alt_16 dist;
    int bright_pix_count;
    alt_u16 centroid_x1, centroid_x2, centroid_y1, centroid_y2, h1, h2, w1, w2;


    OV8865SetExposure(exposureTime);
    OV8865SetGain(gain);
    Focus_Init();
    while(1){
        // printf("In While Loop\n");
        // touch KEY0 to trigger Auto focus
        if((IORD(KEY_BASE,0)&0x03) == 0x02){
    //        	printf("Doing auto focus\n");
            current_focus = Focus_Window(320,240);
        }
        // touch KEY1 to ZOOM
        if((IORD(KEY_BASE,0)&0x03) == 0x01){
            printf("Zooming\n");
            if(bin_level == 3 )bin_level = 1;
            else bin_level ++;
    //				printf("set bin level to %d\n",bin_level);
            MIPI_BIN_LEVEL(bin_level);
            usleep(500000);
        }

        ///////////////////////////////////////////////////////////////////////////
        // Receiving Bboxes from image processor
        alt_u8 idx = 0;
        alt_u8 color = 0;
        while ((IORD(0x42000,EEE_IMGPROC_STATUS)>>8) & 0xff) { 	//Find out if there are words to read
            int word = IORD(0x42000,EEE_IMGPROC_MSG); 			//Get next word from message buffer
            if (word == EEE_IMGPROC_MSG_START){ 					//Newline on message identifier
                printf("Collecting Message\n");
                printf("\n");
            }
            if (color ==  10){
                printf("Getting Colour\n");
                bright_pix_count = word;
                break;
            }
            if (idx == 0){
                printf("RBB MSG ID: ");
                printf("%08x\n ",word);
            } else {
                x =(word & 0xffff0000)>> 16;
                y = (word & 0x0000ffff);
                if (idx == 1){
                    bb[color][0] = x;
                    bb[color][1] = y;
                } else if (idx == 2){
                    bb[color][2] =x;
                    bb[color][3] = y;
                    idx = 0;
                    color += 1;
                }
            }
            idx += 1;
        }

        //////////////////////////////////////////////////////////////////////
        // Software checking of bboxes and selection of Centroids
        for (alt_u8 i = 0; i < 5; i++){
            centroid_x1 = bb[i*2][0];
            centroid_x2 = bb[i*2 + 1][0];
            centroid_y1 = bb[i*2][1];
            centroid_y2 = bb[i*2 + 1][1];
            h1 = bb[i*2][2];
            h2 = bb[i*2 + 1][2];
            w1 = bb[i*2][3];
            w2 = bb[i*2 + 1][3];
            area1 = h1*w1;
            area2 = h2*w2;
            if (i == 0) {
                printf("Color Red 1: ");
                printf("Centroid1: (%d,%d), ",centroid_x1, centroid_y1);
                printf("H_W: (%d,%d), ",h1, w1);
    //                printf("Area: %d\n, ",area1);
    //                printf("Color Red 2: ");
                printf("Centroid2: (%d,%d), ", centroid_x2, centroid_y2);
                printf("H_W: (%d,%d), ",h2, w2);
    //                printf("Area: %d, ",area2);
            } else if (i == 1) {
                printf("Color Green: ");
                printf("Centroid1: (%d,%d), ",centroid_x1, centroid_y1);
    //                printf("Area: %d, ",area1);
                printf("H_W: (%d,%d), ",h1, w1);
    //                printf("Color Green 2: ");
                printf("Centroid2: (%d,%d), ",centroid_x2, centroid_y2);
    //                printf("Area: %d, ",area2);
                printf("H_W: (%d,%d), ",h2, w2);
            } else if (i == 2) {
                printf("Color Blue: ");
                printf("Centroid1: (%d,%d), ",centroid_x1, centroid_y1);
    //                printf("Area: %d, ",area1);
                printf("H_W: (%d,%d), ",h1, w1);
    //                printf("Color Blue 2: ");
                printf("Centroid2: (%d,%d), ",centroid_x2, centroid_y2);
    //                printf("Area: %d, ",area2);
                printf("H_W: (%d,%d), ",h2, w2);
            }else if (i == 3) {
                printf("Color Grey: ");
                printf("Centroid1: (%d,%d), ",centroid_x1, centroid_y1);
    //                printf("Area: %d, ",area1);
                printf("H_W: (%d,%d), ",h1, w1);
    //                printf("Color Grey 2: ");
                printf("Centroid2: (%d,%d), ",centroid_x2, centroid_y2);
    //                printf("Area: %d, ",area2);
                printf("H_W: (%d,%d), ",h2, w2);
            } else if (i == 4) {
                printf("Color Yellow: ");
                printf("Centroid1: (%d,%d), ",centroid_x1, centroid_y1);
    //                printf("Area: %d, ",area1);
                printf("H_W: (%d,%d), ",h1, w1);
    //                printf("Color Yellow 2: ");
                printf("Centroid2: (%d,%d), ",centroid_x2, centroid_y2);
    //                printf("Area: %d, ",area2);
                printf("H_W: (%d,%d), ",h2, w2);
            }

            //////////////////////////////////////////////////////////////////////
            // Send info to esp32 via i2C
            // Error in detection if area is more than a 20000 or if it is too high up the image
            // Check Colour 2 first because it is likely to be nearer to the camera
            if  ((area2 <20000 ) & (centroid_y2 > 100) & (h2 < 150) &  (w2 < 150)) {
                printf("Decided on Colour 2: ");
                angle = estimate_angle(centroid_x2);
                dist = estimate_dist(centroid_y2);
                printf("Dist: %d, Angle: %d", dist, angle);
                updateColour(0x40000,  1, angle, dist, i);
            } else if (w2 < 150) {
                printf("Decided on Colour 2 assume reflection: ");
                angle = estimate_angle(centroid_x2);
                dist = estimate_dist(centroid_y2 - h2/4);
                printf("Dist: %d, Angle: %d", dist, angle);
                updateColour(0x40000,  1, angle, dist, i);
            } else if ((area1 <20000 ) & (centroid_y1 > 100) & (h1 < 150) &  (w1 < 150)){
                printf("Decided on Colour 1: ");
                angle = estimate_angle(centroid_x1);
                dist = estimate_dist(centroid_y1);
                printf("Dist: %d, Angle: %d", dist, angle);
                updateColour(0x40000,  1, angle, dist, i);
            } else {
                printf("Not Detected/Error");
                updateColour(0x40000,  0, 0, 0, i);
            }
            printf(";\n");
        }
        printf("\n");

        //////////////////////////////////////////////////////////////////////
        // Auto Brightness
        printf("Number of bright pixels: %d\n ",bright_pix_count);
        if (time_delay == 0){
            if (bright_pix_count < 30000){
                if (gain < 2500) {
                    gain += GAIN_STEP;
                    OV8865SetGain(gain);
                    printf("Increasing Gain to %x\n", gain);
                }
            }
            if (bright_pix_count > 70000){
                if (gain > 180) {
                    gain -= GAIN_STEP;
                    OV8865SetGain(gain);
                    printf("Decreasing Gain to %x\n", gain);
                }
            }
        }
        time_delay += 1;
        if ( time_delay == 10) {
            time_delay = 0;
        }

       //Process input commands
       int in = getchar();
       switch (in) {
//       	   case 'e': {
//       		   exposureTime += EXPOSURE_STEP;
//       		   OV8865SetExposure(exposureTime);
//       		   printf("\nExposure = %x\n", exposureTime);
//       	   	   break;}
//       	   case 'd': {
//       		   exposureTime -= EXPOSURE_STEP;
//       		   OV8865SetExposure(exposureTime);
//       		   printf("\nExposure = %x ", exposureTime);
//       	   	   break;}
       	   case 't': {
       		   gain += GAIN_STEP;
       		   OV8865SetGain(gain);
       		   printf("\nGain = %x\n", gain);
       	   	   break;}
       	   case 'g': {
       		   gain -= GAIN_STEP;
       		   OV8865SetGain(gain);
       		   printf("\nGain = %x\n", gain);
       	   	   break;}
//       	   case 'r': {
//        	   current_focus += manual_focus_step;
//        	   if(current_focus >1023) current_focus = 1023;
//        	   OV8865_FOCUS_Move_to(current_focus);
//        	   printf("\nFocus = %x\n",current_focus);
//       	   	   break;}
//       	   case 'f': {
//        	   if(current_focus > manual_focus_step) current_focus -= manual_focus_step;
//        	   OV8865_FOCUS_Move_to(current_focus);
//        	   printf("\nFocus = %x\n ",current_focus);
//       	   	   break;}
       }


	   //Main loop delay
	   usleep(100000);

   };
  return 0;
}
