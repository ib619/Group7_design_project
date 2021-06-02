from typing import DefaultDict
import cv2
import numpy as np
from math import sqrt, pi, cos, sin


height, width = 480, 640
img = cv2.imread('images/grey_edge.jpg')
resized = cv2.resize(img, (width, height))
print(img)
# img = cv2.GaussianBlur(img, (5, 5), 0)
# scale_percent = 100 # percent of original size
# width = int(img.shape[1] * scale_percent / 100)
# height = int(img.shape[0] * scale_percent / 100)
# dim = (width, height)

# resize image
# resized = cv2.resize(img, dim, interpolation = cv2.INTER_AREA)
print(resized.shape)
hsv = cv2.cvtColor(resized, cv2.COLOR_BGR2HSV)
	# Set up the detector with default parameters
grey = cv2.cvtColor(resized, cv2.COLOR_BGR2GRAY)
print(grey)
grey_edit = np.where(grey > 2, 255, 0)
print(grey_edit.shape)
detector = cv2.SimpleBlobDetector_create()
 
# Detect blobs.
keypoints = detector.detect(grey)
print(keypoints) 
# Draw detected blobs as red circles.
# cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS ensures the size of the circle corresponds to the size of blob
im_with_keypoints = cv2.drawKeypoints(resized, keypoints, np.array([]), (0,0,255), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
# # Bitwise-AND mask and original image

# kernel = np.ones((3,3),np.uint8)
# erosion = cv2.erode(mask,kernel,iterations = 1)
# dilation = cv2.erode(erosion,kernel,iterations = 1)
# print(mask.shape)
# res = cv2.bitwise_and(resized, resized, mask= erosion)

# print(erosion.shape)

# np.savetxt("greyscale.csv", grey_edit, delimiter=",", fmt='%d')

# edges = cv2.Canny(grey,100,200)
# print(grey_edit)
# edges_coords = []
# for i in range(height):
#     for j in range(width):
#         if grey_edit[i][j] == 255:
#             edges_coords.append((i,j))
# # print(edges_coords)
# # circles = cv2.HoughCircles(grey_edit, cv2.HOUGH_GRADIENT, 1, 240/ 8, param1=100, param2=10, minRadius=10, maxRadius=60)

# rmin =40
# rmax = 80
# steps = 4 
# threshold = 0.9

# points = []
# for  r in range(rmin, rmax + 1):
#     points.append((r, 0, r))
#     points.append((r, r, 0))
#     points.append((r, 0, -r))
#     points.append((r,-r,0))
#     # for t in range(steps):
#     #     points.append((r, int(r*cos(2*pi*t/steps)), int(r*sin(2*pi*t/steps))))
# print(points)
# acc = DefaultDict(int)
# print(len(edges_coords), len(points))
# for x, y in edges_coords:
#     for r, dx, dy in points:
#         a = x - dx
#         b = y - dy 
#         acc[(a,b,r)] += 1
# print(len(acc))
# circles = []
# for k, v in acc.items():
#     x , y ,r = k
#     if v / steps >= threshold and all((x - xc) ** 2 + (y - yc) ** 2 > rc ** 2 for xc, yc, rc in circles):
#         print(v / steps, x, y, r)
#         circles.append((x, y, r))
# print(circles)
# for x, y, r in circles:
#     cv2.circle(resized, (y, x), r, (0, 0, 255), 3)

while(1):
    cv2.imshow('frame',img)
    # cv2.imshow('resized frame',grey)
    cv2.imshow('grey',grey)
    # cv2.imshow('erosion',erosion)
    # cv2.imshow('dilation',dilation)
    # cv2.imshow('edge', edges)
    cv2.imshow("detected circles", im_with_keypoints)
    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break
cv2.destroyAllWindows()
