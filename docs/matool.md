The **matool** is a multi-purpose for texture files (`.mat`).
The tool can create new `MAT` files, modify existing or extract images from `MAT` files.

## Usage
```
Indiana Jones and The Infernal Machine MAT file tool
Command-line interface tool for MAT image format.

  Usage: matool <command> [sub-command] [options]

Command:                       Description:
  create                         Create new MAT file
  extract                        Extract images from MAT file
  info                           Print to the console information about MAT file
  modify                         Modify existing MAT file
  help                           Show this message
```

In the terminal window (`cmd.exe` on Windows) enter `matool` following by selected *command* and *sub-command*. 
```
matool <command> [sub-command] [options]
```
To get help for a specific command enter `help` as command following by *command* and *sub-command* of interest.  

 ### **Commands**
  * **`create`** - Create a new MAT file.

    **Available encodings:**
      * **rgb555** - Encode images in RGB555 format.
      * **rgb555be** - Encode images in big-endian byte order RGB555 format.
      * **rgb565** - Encode images in RGB565 format.
      * **rgb565be** -Encode images in big-endian byte order RGB565 format.
      * **rgba4444** - Encode images in RGBA4444 format.
      * **rgba4444be** - Encode images in big-endian byte order RGBA4444 format.
      * **argb4444** - Encode images in ARGB4444 format.
      * **argb4444be** - Encode images in big-endian byte order ARGB4444 format.
      * **rgba5551** - Encode images in RGBA5551 format.
      * **rgba5551be** - Encode images in big-endian byte order RGBA5551 format.
      * **argb1555** - Encode images in ARGB1555 format.
      * **argb1555be** - Encode images in big-endian byte order ARGB1555 format.
      * **rgb24** - Encode images in RGB888 format.
      * **rgb24be** - Encode images in big-endian byte order RGB888 format.
      * **rgba32** - Encode images in RGBA8888 format.
      * **rgba32be** - Encode images in big-endian byte order RGBA8888 format.
      * **argb32** - Encode images in ARGB8888 format.
      * **argb32be** - Encode images in big-endian byte order ARGB8888 format.

    Encodings used by PC version of the game:
      * <b>rgb565</b> - MAT files with non-transparent images.
      * <b>rgba4444</b> - MAT files with transparent images.
      * <b>rgba5551</b> - MAT files with transparent images for 3D models in the inventory menu.

    **Command options:**
      * `--mipmap=<LOD>` - Generate mipmap for every image.  
      If no *LOD* number is provided or *LOD* is 0 then the maximum number of mipmap levels will be generated.  
      *Note: The game can use at max 4 mipmap levels.*
      * `--no-srgb` - No sRGB conversion is made when generating mipmaps.  
      *Note: Mipmaps generated in the MAT files from the original PC version of the game didn't use sRGB conversion.*

    **Sub-commands**:
      * **`batch`** - Create multiple MAT files using existing MAT files as a reference. 

  * **`extract`** - Extract images from one or more MAT files.

    **Command options:**
      * `--bmp` - Extract images in BMP format.  
      By default, images are extracted in PNG format.
      * `--max-tex` - Max number of cel images to extract from each MAT file.  
      By default, all images are extracted.
      * `--mipmap` - Extract also mipmap LOD images from MAT file.  
     By default, only top image at LOD 0 is extracted from each texture.
.
  * **`info`** - Print to the console information about MAT file.

  * **`modify`** - Modify existing MAT file.

    **Mod options:**
      * `--encoding` - Change encoding.*
      * `--mipmap=<LOD>` - Change mipmap levels.  
      If no *LOD* number is provided or *LOD* is 0 then the maximum number of Mipmap levels will be generated.

    **Other options:**
      * `--no-srgb` - No sRGB conversion is made when generating Mipmaps.  
      *Note: Mipmaps generated in the MAT files from the original PC version of the game didn't use sRGB conversion.*

## Usage examples:
  - Create new MAT file from one image using RGB565 encoding:
  ```
     matool create rgb565 <path_to_image_file_bmp|png>
  ```

   - Create new MAT file from 3 images using RGBA32 encoding:
  ```
     matool create rgba32 <image__cel_0.bmp|png> <image__cel_1.bmp|png> <image__cel_2.bmp|png> <image__cel_3.bmp|png>
  ```

  - Create new MAT file with all mipmap levels from one image using RGB565 encoding:
  ```
     matool create rgb565 --mipmap <path_to_image_file_bmp|png>
  ```

  - Create new MAT file with 4 mipmap levels from one image using RGB565 encoding:
  ```
     matool create rgb565 --mipmap=4 <path_to_image_file_bmp|png>
  ```

  - Create new MAT files in bulk:
  ```
     matool create batch <path_to_image_folder_BMP|PNG> <path_to_reference_MAT_folder>
  ```

  - Create new MAT files in bulk and force 32 bit color depth:
  ```
     matool create batch --force-8bpc <path_to_image_folder_BMP|PNG> <path_to_reference_MAT_folder>
  ```

  - Extract images from MAT file:
  ```
     matool extract <path_to_MAT_file>
  ```

  - Extract images from multiple MAT files:
  ```
     matool extract <path_to_MAT_folder>
  ```

  - Extract images from MAT file in BMP format:
  ```
     matool extract --bmp <path_to_MAT_file>
  ```

  - Extract images including mipmap LOD images from MAT file:
  ```
     matool extract --mipmap <path_to_MAT_file>
  ```

  - Extract only 1 cel image from MAT file:
  ```
     matool extract --max-tex=1 <path_to_MAT_file>
  ```

  - Change encoding of existing MAT file to RGB565be:
  ```
     matool modify --encoding=rgb565be <path_to_MAT_file>
  ```

  - Change number of mipmap levels of existing MAT file:
  ```
     matool modify --mipmap=2 <path_to_MAT_file>
  ```