import cv2
import numpy as np
import os

# Delete Previous Frame Data
import shutil 
pathToDelete = r"C:\Users\kanie\sorry-for-being-bad-at-git\hackthe6ix\frames"
shutil.rmtree( pathToDelete )
ReCreatePath = r"C:\Users\kanie\sorry-for-being-bad-at-git\hackthe6ix\frames"
os.makedirs( ReCreatePath )


# Read an image in ycbcr color space
image = cv2.imread('bufferTest.txt')

# # show original image
# cv2.imshow("image data to image", image)
# cv2.waitKey(0)

# Convert the image to RGB color space
RGB_image = cv2.cvtColor(image, cv2.COLOR_YCrCb2BGR)

# show RGB image
cv2.imshow("RGB image", RGB_image)
cv2.waitKey(0)

