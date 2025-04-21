# JPEGAA  -  Illustrating image degradation from multiple JPEG encoding cycles
The JPEGAA application (JPEG Ad Absurdum) illustrates the degradation of an image resulting from multiple iterations of
JPEG encoding.

The application will document the the base (starting) image and will then perform the requested number of cycles of:-

1.	Encode and save the image as a JPEG in the images/JPCnn.jpeg file (where nn is the cycle number)
2.	Reload the image from disk.
3.	Document the reloaded image.
4.	Save a bitmap of the reloaded image in the images/JPCnn.bmp file (where nn is the cycle number)

The input image (CIMG-2X3.gif) was chosen as it is provocative for JPEG encoding, showing significant ghosting artefacts
from the first cycle of encoding.

