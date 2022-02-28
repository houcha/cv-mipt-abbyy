import sys
import cv2

img1 = cv2.imread(sys.argv[1])
img2 = cv2.imread(sys.argv[2])

psnr = cv2.PSNR(img1, img2)

print(psnr)
