The **cndtool** is a multi-purpose tool for compact game level files (`.cnd`).
The tool can list, extract, add, replace, or remove game assets stored in a `CND` file. It can also extract and convert level geometry resources (level surface vertices and surface UV texture vertices) to [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format.

**Add or replace**:
  - animation files (`.key`)
  - texture files (`.mat`)

**Convert**
  - CND to [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file)

**Extract**:
  - animation files (`.key`)
  - texture files (`.mat`)
  - sound files (`.wav`)

## Usage
```
Indiana Jones and the Infernal Machine CND file tool

Command line interface tool to extract and modify
game assets stored in a CND level file.

  Usage: cndtool <command> [sub-command] [options]

Command:                       Description:
  add                            Add or replace game assets
  convert                        Convert CND file
  extract                        Extract game assets
  list                           Print to the console stored game assets
  remove                         Remove one or more game assets
  help                           Show this message or help for specific command
```

In the terminal window (`cmd.exe` on Windows) enter `cndtool` following by selected *command* and *sub-command*. 
```
cndtool <command> [sub-command] [options]
```
To get help for a specific command enter `help` as command following by *command* and *sub-command* of interest.  

 ### **Commands**
  * **`add`** - Add or replace game assets.

    **Sub-commands:**
      * **`animation`** - Add or replace stored animation assets (`.key`).
      * **`material`** - Add or replace stored texture assets (`.mat`).

  * **`convert`** - Convert CND file format to another format.

    **Sub-commands:**
      * **`obj`** - Extract level geometry and convert to [Wavefront .obj](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format.  
      **Command options:**
        * `--no-mat` - Don't extract texture assets in PNG format for level surfaces (walls, ground, sky).  
        *Note: Surface images have to be put manually into the **mtl** folder where the **.obj** file is created.*

  * **`extract`** - Extract animation (`.key`), texture (`.mat`) and sound (`.wav`) game assets from CND file.  
  *Note: Sound assets are by default extracted in IndyWV format.*

    **Command options:**
      * `--mat-bmp` - Convert extracted material assets to BMP format.
      * `--mat-png` - Convert extracted material assets to PNG format.
      * `--mat-max-tex` - Max number of images to convert from each material file. By default all are converted.
      * `--mat-mipmap` - Extract also MipMap LOD images when converting material file.
      * `--sound-wav` - Convert extracted sound assets in IndyWV format to WAV format.
      * `--no-key` - Don't extract animation assets.
      * `--no-mat` - Don't extract material assets.
      * `--no-sound` - Don't extract sound assets.

  * **`list`** - Print to the console stored game assets in CND file.
  * **`remove`** - Remove one or more game assets from CND file.

    **Sub-commands:**
      * **`animation`** - Remove stored animation assets (`.key`).
      * **`material`** - Remove stored material assets (`.mat`).
## Usage examples:
  - Extract all game assets:
  ```
     cndtool extract <path_to_cnd_file>
  ```

  - Extract all game assets and extract **PNG** images from extracted **MAT** files:
  ```
     cndtool extract --mat-png <path_to_cnd_file>
  ```

  - Extract all game assets then extract **BMP** images from extracted **MAT** files and  
  convert extracted sound assets to **WAV** format:
  ```
     cndtool extract --mat-bmp --sound-wav <path_to_cnd_file>
  ```

  - Extract all game assets except for animation assets:
  ```
     cndtool extract --no-key <path_to_cnd_file>
  ```

  - Replace existing texture assets:
  <pre>
    cndtool add material --replace &#60path_to_cnd_file> &#60path_to_mat_file>

    <i>Note: <b>&#60path_to_mat_file></b> can be multiple paths to *.mat files delimitated by space. 
    e.g.: 
    cndtool add material --replace &#60cnd_file> &#60mat_file_1> &#60mat_file_2> &#60mat_file_3> ...</i></pre>
  - Convert level geometry to OBJ:  
    *Note: See tutorial how to import OBJ converted level into Blender [here](cnd2obj.md).*
  ```
     cndtool convert obj <path_to_cnd_file>
  ```