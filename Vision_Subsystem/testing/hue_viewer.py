import matplotlib.pyplot as plt
from skimage import io
from skimage.color import rgb2hsv
filename = "images/yellow_gain380.jpg"
name = (filename.split(".")[0]).split("/")[1]
rgb_img = io.imread(filename)
hsv_img = rgb2hsv(rgb_img)
hue_img = hsv_img[:, :, 0]
sat_img = hsv_img[:, :, 1]
value_img = hsv_img[:, :, 2]

# fig, (ax0, ax1, ax2) = plt.subplots(ncols=3, figsize=(8, 2))

# ax0.imshow(sat_img)
# ax0.set_title("Saturation image")
# ax0.axis('off')
# ax1.imshow(hue_img, cmap='hsv')
# ax1.set_title("Hue channel")
# ax1.axis('off')
# ax2.imshow(value_img)
# ax2.set_title("Value channel")
# ax2.axis('off')

# fig.tight_layout()
# fig.savefig('various_spectrums_{}.png'.format(name))

# ########################################

fig, (ax0, ax1, ax2) = plt.subplots(ncols=3, figsize=(8, 3))

ax0.hist(hue_img.ravel(), 512)
ax0.set_title("Histogram of the Hue channel")
ax0.set_xbound(0, 1)
ax1.hist(sat_img.ravel(), 512)
ax1.set_title("Histogram of the Sat channel")
ax1.set_xbound(0, 1)
ax2.hist(value_img.ravel(), 512)
ax2.set_title("Histogram of the Val channel")
ax2.set_xbound(0, 1)


fig.tight_layout()
fig.savefig('Hsv_Hist_{}.png'.format(name))


############################################
double_thresh = True
# hue_threshold = 0.09 # Pink Ball
# binary_img = hue_img < hue_threshold

# hue_lower_threshold = 0.10 # Yellow Ball
# hue_upper_threshold = 0.14
# binary_img1 = hue_img < hue_upper_threshold 
# binary_img2 = hue_img > hue_lower_threshold
# binary_img = binary_img1 * binary_img2

# hue_lower_threshold = 0.32 # Green Ball Needs further tuning!
# hue_upper_threshold = 0.35
# binary_img1 = hue_img < hue_upper_threshold 
# binary_img2 = hue_img > hue_lower_threshold
# binary_img = binary_img1 * binary_img2

# hue_lower_threshold = 0.31 # Grey Ball Needs further tuning!
# hue_upper_threshold = 0.34
# binary_img1 = hue_img < hue_upper_threshold 
# binary_img2 = hue_img > hue_lower_threshold
# binary_img = binary_img1 * binary_img2

# hue_lower_threshold = 0.41 # Blue Ball
# hue_upper_threshold = 0.48
# binary_img1 = hue_img < hue_upper_threshold 
# binary_img2 = hue_img > hue_lower_threshold
# binary_img = binary_img1 * binary_img2

# fig, (ax0, ax1) = plt.subplots(ncols=2, figsize=(8, 3))

# ax0.hist(hue_img.ravel(), 512)
# ax0.set_title("Histogram of the Hue channel with threshold")
# if (double_thresh):
#     ax0.axvline(x=hue_lower_threshold, color='r', linestyle='dashed', linewidth=2)
#     ax0.axvline(x=hue_upper_threshold, color='r', linestyle='dashed', linewidth=2)
# else:
#     ax0.axvline(x=hue_threshold, color='r', linestyle='dashed', linewidth=2)
# ax0.set_xbound(hue_lower_threshold - 0.1, hue_upper_threshold + 0.1)
# ax1.imshow(binary_img)
# ax1.set_title("Hue-thresholded image")
# ax1.axis('off')

# fig.tight_layout()
# fig.savefig('Hue_threshold_{}.png'.format(name))