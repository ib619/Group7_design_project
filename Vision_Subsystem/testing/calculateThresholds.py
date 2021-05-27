from typing import DefaultDict
import cv2
import numpy as np
from math import sqrt, pi, cos, sin

img = cv2.imread('images/edgedetection.jpg')
img = cv2.resize(img, (640, 480))

# img = cv2.GaussianBlur(img, (5, 5), 0)
scale_percent = 25 # percent of original size
width = int(img.shape[1] * scale_percent / 100)
height = int(img.shape[0] * scale_percent / 100)
dim = (width, height)

# resize image
resized = cv2.resize(img, dim, interpolation = cv2.INTER_AREA)
print(resized.shape)
hsv = cv2.cvtColor(resized, cv2.COLOR_BGR2HSV)
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
lower_grey = np.array([0,0,0])
upper_grey = np.array([100,60,100])
# Works for 380 Gain
# lower_grey = np.array([20,0,30])
# upper_grey = np.array([80,63,47])

# Threshold the HSV image to get only blue colors
green_mask = cv2.inRange(hsv, lower_green, upper_green)
red_mask =  cv2.inRange(hsv, lower_red, upper_red)
blue_mask =  cv2.inRange(hsv, lower_blue, upper_blue)
yellow_mask =  cv2.inRange(hsv, lower_yellow, upper_yellow)
grey_mask =  cv2.inRange(hsv, lower_grey, upper_grey)
mask = yellow_mask 

# mask = green_mask + red_mask + blue_mask+ yellow_mask +grey_mask


# # Bitwise-AND mask and original image

kernel = np.ones((3,3),np.uint8)
erosion = cv2.erode(mask,kernel,iterations = 1)
print(mask.shape)
res = cv2.bitwise_and(resized, resized, mask= erosion)

print(erosion.shape)
grey = cv2.cvtColor(resized, cv2.COLOR_BGR2GRAY)
grey_edit = np.where(grey > 30, 255, 0)
np.savetxt("greyscale.csv", grey_edit, delimiter=",", fmt='%d')

# edges = cv2.Canny(grey,100,200)
print(grey_edit)
edges_coords = []
for i in range(120):
    for j in range(160):
        if grey_edit[i][j] == 255:
            edges_coords.append((i,j))
# print(edges_coords)
# circles = cv2.HoughCircles(grey, cv2.HOUGH_GRADIENT, 1, 240/ 8, param1=100, param2=10, minRadius=10, maxRadius=60)

# print(circles)
# if circles is not None:
#     circles = np.uint16(np.around(circles))
#     for i in circles[0, :]:
#         center = (i[0], i[1])
#         # circle center
#         cv2.circle(resized, center, 1, (0, 255, 0), 3)
#         # circle outline
#         radius = i[2]
#         cv2.circle(resized, center, radius, (0, 0, 255), 3)
rmin =5
rmax = 30
steps = 4 
threshold = 0.6

points = []
for  r in range(rmin, rmax + 1):
    points.append((r, 0, r))
    points.append((r, r, 0))
    points.append((r, 0, -r))
    points.append((r,-r,0))
    # for t in range(steps):
    #     points.append((r, int(r*cos(2*pi*t/steps)), int(r*sin(2*pi*t/steps))))
print(points)
acc = DefaultDict(int)
print(len(edges_coords), len(points))
for x, y in edges_coords:
    for r, dx, dy in points:
        a = x - dx
        b = y - dy 
        acc[(a,b,r)] += 1
print(len(acc))
circles = []
for k, v in acc.items():
    x , y ,r = k
    if v / steps >= threshold and all((x - xc) ** 2 + (y - yc) ** 2 > rc ** 2 for xc, yc, rc in circles):
        print(v / steps, x, y, r)
        circles.append((x, y, r))
print(circles)
for x, y, r in circles:
    cv2.circle(resized, (y, x), r, (0, 0, 255), 3)

while(1):
    cv2.imshow('frame',img)
    cv2.imshow('resized frame',img)
    # cv2.imshow('mask',mask)
    # cv2.imshow('res',res)
    # cv2.imshow('grey',grey)
    cv2.imshow('erosion',erosion)
    # cv2.imshow('edge', edges)
    cv2.imshow("detected circles", resized)
    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break
cv2.destroyAllWindows()
