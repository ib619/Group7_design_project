import cv2
import numpy as np

img = cv2.imread('images/exposure2000_gain780.jpg')
img = cv2.resize(img, (640, 480)) 
img = cv2.GaussianBlur(img, (5, 5), 0)
hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

# define range of red color in HSV
# Works for 780 Gain
lower_red = np.array([0,90,90])
upper_red = np.array([15,255,255])
# Works for 380 Gain
# lower_red = np.array([0,127,63])
# upper_red = np.array([18,191,127])

# define range of green color in hsv
# Works for 780 Gain
lower_green = np.array([55,100,100])
upper_green = np.array([65,180,180])
# Works for 380 Gain
# lower_green = np.array([45,127,63])
# upper_green = np.array([65,165,95])


# define range of blue color in HSV
# Works for 780 Gain
lower_blue = np.array([75,50,75])
upper_blue = np.array([110,255,255])
# Works for 380 Gain
# lower_blue = np.array([60,60,60])
# upper_blue = np.array([90,130,80])

# define range of yellow color in hsv
# Works for 780 Gain
lower_yellow = np.array([18,150,80])
upper_yellow = np.array([25,255,255])
# Works for 380 Gain
# lower_yellow = np.array([18,140,65])
# upper_yellow = np.array([27,220,120])

# define range of grey color in hsv
# Works for 780 Gain
lower_grey = np.array([0,0,10])
upper_grey = np.array([90,32,70])
# Works for 380 Gain
# lower_grey = np.array([20,0,30])
# upper_grey = np.array([80,63,47])

# Threshold the HSV image to get only blue colors
green_mask = cv2.inRange(hsv, lower_green, upper_green)
red_mask =  cv2.inRange(hsv, lower_red, upper_red)
blue_mask =  cv2.inRange(hsv, lower_blue, upper_blue)
yellow_mask =  cv2.inRange(hsv, lower_yellow, upper_yellow)
grey_mask =  cv2.inRange(hsv, lower_grey, upper_grey)
mask = grey_mask

# mask = green_mask + red_mask + blue_mask+ yellow_mask  
# # Bitwise-AND mask and original image

kernel = np.ones((5,5),np.uint8)
# h,s,v = cv2.split(res)
erosion = cv2.erode(mask,kernel,iterations = 1)
dilation = cv2.dilate(erosion,kernel,iterations = 1)

edges = cv2.Canny(img,100,200)

res = cv2.bitwise_and(img, img, mask= dilation)

while(1):
    cv2.imshow('frame',img)
    cv2.imshow('mask',mask)
    # cv2.imshow('res',res)
    cv2.imshow('erosion',erosion)
    # cv2.imshow('dilation',dilation)
    # cv2.imshow('edge',edges)
    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break
cv2.destroyAllWindows()
