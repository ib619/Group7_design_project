from typing import DefaultDict
import cv2
import numpy as np
from math import sqrt, pi, cos, sin

img = cv2.imread('../images/testing.jpg')
img = cv2.resize(img, (640, 480))

# Gaussian Filter
img = cv2.GaussianBlur(img, (5, 5), 0)

# HSV Colour Space Conversion
hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

# Colour Threshold
# define range of red color in HSV
# Works for 780 Gain
lower_red = np.array([0,90,90])
upper_red = np.array([15,255,255])

# define range of green color in hsv
# Works for 780 Gain
lower_green = np.array([55,100,100])
upper_green = np.array([65,180,180])


# define range of blue color in HSV
# Works for 780 Gain
lower_blue = np.array([75,50,75])
upper_blue = np.array([110,255,255])

# define range of yellow color in hsv
# Works for 780 Gain
lower_yellow = np.array([18,150,80])
upper_yellow = np.array([25,255,255])

# define range of grey color in hsv
# Works for 780 Gain
lower_grey = np.array([20,0,0])
upper_grey = np.array([45,40,65])
lower_grey2 = np.array([67,0,30])
upper_grey2 = np.array([110,70,70])

# Threshold the HSV image to get only blue colors
green_mask = cv2.inRange(hsv, lower_green, upper_green)
red_mask =  cv2.inRange(hsv, lower_red, upper_red)
blue_mask =  cv2.inRange(hsv, lower_blue, upper_blue)
yellow_mask =  cv2.inRange(hsv, lower_yellow, upper_yellow)
grey_mask =  cv2.inRange(hsv, lower_grey, upper_grey)
grey2_mask =  cv2.inRange(hsv, lower_grey2, upper_grey2)
# mask = grey_mask

mask = green_mask + red_mask + blue_mask+ yellow_mask +grey_mask + grey2_mask

# # Bitwise-AND mask and original image

# Morph Filters
kernel = np.ones((3,3),np.uint8)
erosion = cv2.erode(mask,kernel,iterations = 1)
dilation = cv2.erode(erosion,kernel,iterations = 1)
res = cv2.bitwise_and(img, img, mask= dilation)

grey = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

# Edge Detection
edges = cv2.Canny(grey,100,200)

# Hough Transform
detected_circles = img 
circles = cv2.HoughCircles(grey, cv2.HOUGH_GRADIENT, 1, 50, param1=200, param2=10, minRadius=0, maxRadius=0)
print(circles)
for x, y, r in circles[0]:
    cv2.circle(detected_circles, (int(y),int(x)), int(r), (0, 0, 255), 3)




while(1):
    cv2.imshow('frame',img)
    cv2.imshow('mask',mask)
    cv2.imshow('res',res)
    cv2.imshow('grey',grey)
    cv2.imshow('erosion',erosion)
    cv2.imshow('dilation',dilation)
    cv2.imshow('edge', edges)
    cv2.imshow("detected circles", detected_circles)
    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break
cv2.destroyAllWindows()
