# fracanim
Fractal image generator (see sample image in file 'image.png'). C++ code based on the following algorithm:
<pre>
int w, h - output image width and height 
double coef1, coef2 - coefficients with default values 1.0 and 0.0
double fx, fy;
int x, y;
for (int j = 0; j < 500; j++) {
  fx <- random number between 0.0 and 1.0
  fy <- random number between 0.0 and 1.0
  col <- random color from fixed table (8 colors)
  for (int i = 0; i < 1000; i++) {
    x = int(fx * w); // get X coordinate using output image width
    y = int(fy * h); // get Y coordinate using output image height
    AddPixel(x, y, col); // save coordinates and color to container, increase color weight  
    fx = frac(fx + sin(fy - coef2));
    fy = frac(fy + cos(fx * coef1));
  }
}
// finally draw saved pixels from container,
// mix pixel color with black background using pixel weight
</pre>
Uses [libpng](https://github.com/libpng/libpng/tree/main "libpng") library for saving generated images to PNG files. Requires C++20. Compiled and tested under Windows and Ubuntu (WSL).

The code above generates one image. The program allows to save series of images with animation of **coef1** and **coef2** coefficients between specified values.

Default range: **coef1** from 1.0 to 2.0, **coef2** from 0.0 to 0.5.

Number of working threads can be specified for faster images generation and saving. Half number of (logical) CPU cores is used by default.

**WARNING**: this loads CPU up to 100%, use with care.

## Command line
<pre>
Usage: "fracanim.exe" [options]
options can be:
-help			display this help
-width {N}		set output image width in pixels, 1280 by default
-height {N}		set output image height in pixels, 720 by default
-outfolder {path}	set output folder (will be created it doesn't exist) for saving image files
-steps {N}		set number of output images (animation steps), 1 by default
-coef1 {v}		set value (float) of coefficient 1, 1.0 by default
-coef2 {v}		set value (float) of coefficient 2, 0.0 by default
-coef1end {v}		set ending value (float) of coefficient 1, 2.0 by default
-coef2end {v}		set ending value (float) of coefficient 2, 0.5 by default
-threads {N}		set number of running threads: use -threads max to use CPU cores number,
use -threads half to use 1/2 CPU cores number (default) or specify a number, e.g. -threads 4
Pressing 'q' stops writing image series.
</pre>
If one image is generated, file name 'image.png' is used, and 'imageNNNNN.png' for image series.
### Examples
<pre>
fracanim.exe -outfolder D:\tmp\png -coef1 1.2 -coef2 0.1
</pre>
Creates one image 'D:\tmp\png\image.png' with default width and height (1280x720), uses values: coef1=1.2 and coef2=0.1
<pre>
fracanim.exe -outfolder D:\tmp\png -coef1 1.0 -coef2 0.125 -width 800 -height 450
</pre>
Creates one image 'D:\tmp\png\image.png' with 800x450 width and height, uses values: coef1=1.0 and coef2=0.125

This was used to create sample image in file 'image.png' 
<pre>
fracanim.exe -outfolder D:\tmp\png -steps 1000 -width 1920 -height 1080
</pre>
Creates 1000 images (1920x1080 pixels) in 'D:\tmp\png' folder running on all available CPU cores. Coefficients are changed in 1000 steps between default values - coef1: 1.0 -> 2.0, coef2: 0.0 -> 0.5
<pre>
fracanim.exe -outfolder D:\tmp\png -steps 1000 -width 1920 -height 1080 -coef1 1.0 -coef1end 5.0 -coef2 0 -coef2end 0 -threads max
</pre>
Creates 1000 images (1920x1080 pixels) in 'D:\tmp\png' folder running on all available CPU cores (with maximal CPU load). Only animation of coef1 from 1.0 to 5.0, coef2 isn't used (set to zero).