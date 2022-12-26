The **cndtool** is a multi-purpose tool for compact game level files (`.cnd`).  
The tool can:
 - List, extract, add, replace, or remove game assets stored in `CND` file(s). 
 - Convert CND file format to [NDY level format](https://github.com/Jones3D-The-Infernal-Engine/Documentation/blob/main/ndy.md) and vice versa.
 - Extract and convert level geometry (level surface vertices and surface UV texture vertices) to [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format.

**Add or replace**:
  - animation files (`.key`)
  - texture files (`.mat`)

**Convert**
  - CND to [NDY level format](https://github.com/Jones3D-The-Infernal-Engine/Documentation/blob/main/ndy.md)
  - [NDY level format](https://github.com/Jones3D-The-Infernal-Engine/Documentation/blob/main/ndy.md) to CND
  - CND to [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file)

**Extract**:
  - animation files (`.key`)
  - texture files (`.mat`)
  - sound files (`.wav`)
  - templates (`.tpl`)

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

      * **`cnd`** - Convert [NDY](https://github.com/Jones3D-The-Infernal-Engine/Documentation/blob/main/ndy.md), a text based level format to binary CND file format. The command supports batch mode conversion of multiple NDY files if folder is specified instead of single CND file.

        ```
        Usage: cndtool convert cnd [options] <ndy-file-path|ndy-folder> <game-assets-folder>
        ```

        **Command positional arguments:**
          * `ndy-file-path|ndy-folder` - Path to single NDY file or folder with multiple NDY files.
          * `game-assets-folder` - Folder where animation, texture, script and sound files can be located.  
                                   Assets can be located in sub-folders: `cog` for cog scripts, `mat` for textures, `key` or `3do\key` for animations and `sound`, `wv` or `wav` for sound files.

        **Command options:**
          * `--no-key` - Don't extract animation assets from CND.
          * `--no-mat` - Don't extract texture assets from CND.
          * `--no-sound` - Don't extract sound assets from CND.
          * `--output-dir` - Output directory.
          * `--verbose` - Verbose log printout to the console.

      * **`ndy`** - Convert CND binary format to [NDY](https://github.com/Jones3D-The-Infernal-Engine/Documentation/blob/main/ndy.md) text based level format, and extract stored mat, key, sound resources. The command supports batch mode conversion of multiple CND files if folder is specified instead of single CND file.

        ```
        Usage: cndtool convert ndy [options] <cnd-file-path|cnd-folder> <cog-scripts-folder>
        ```

        **Command positional arguments:**
        * `cnd-file-path|cnd-folder` - Path to single CND file or folder with multiple CND files.
        * `cog-scripts-folder` - Path to the folder where tool can find required cog scripts.  
                                 Scripts can be located at `cog` sub-folder.

        **Command options:**
          * `--no-key` - Don't extract animation assets from CND.
          * `--no-mat` - Don't extract texture assets from CND.
          * `--no-sound` - Don't extract sound assets from CND.
          * `--output-dir` - Output directory.
          * `--verbose` - Verbose log printout to the console.

      * **`obj`** - Extract level geometry and convert to [Wavefront .obj](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format.  

        ```
        Usage: cndtool convert obj [options] <cnd-file-path>
        ```

        **Command positional arguments:**
        * `cnd-file-path|cnd-folder` - Path to single CND file or folder with multiple CND files.

        **Command options:**
          * `--no-mat` - Don't extract texture assets in PNG format for level surfaces (walls, ground, sky).  
            *Note: Surface images have to be put manually into the **mtl** folder where the **.obj** file is created.*
          * `--output-dir` - Output directory.
          * `--verbose` - Verbose log printout to the console.

  * **`extract`** - Extract animation (`.key`), texture (`.mat`), sound (`.wav`) and template (`.tpl`) game assets from CND file. The command can extract from single file or multiple file(s) at once if folder is specified.  
  If sound assets are compressed in WV format (by default) unless specified otherwise no conversion takes place by default. Templates are extracted to one file - `ijim.tpl`. If output file is specified templates from multiple CND files are written to single file.

    ```
    Usage: cndtool extract [options] <cnd-file-path|cnd-folder>
    ```

    **Command positional arguments:**
      * `cnd-file-path|cnd-folder` - Path to single CND file or folder with multiple CND files.

    **Command options:**
      * `--mat-bmp` - Convert extracted material assets to BMP format.
      * `--mat-png` - Convert extracted material assets to PNG format.
      * `--mat-max-tex` - Max number of images to convert from each material file. By default all are converted.
      * `--mat-mipmap` - Extract also MipMap LOD images when converting material file.
      * `--sound-wav` - If extracted sounds are compressed in WV (IndyWV) format, convert them to uncompressed WAV format. 
      * `--soundbank` - Extract whole soundbank track from CND file.
      * `--template-overwrite` - Overwrite any existing template.
      * `--no-key` - Don't extract animation assets.
      * `--no-mat` - Don't extract material assets.
      * `--no-sound` - Don't extract sound assets.
      * `--no-template` - Don't extract template assets.
      * `--output-dir`, `-o` - Output directory.
      * `--verbose` - Verbose log printout to the console.

  * **`list`** - Print to the console stored game assets in CND file.
  * **`remove`** - Remove one or more game assets from CND file.

    **Sub-commands:**
      * **`animation`** - Remove stored animation assets (`.key`).
      * **`material`** - Remove stored material assets (`.mat`).

## Usage examples:
  - Convert level geometry to OBJ:  
    *Note: See tutorial how to import OBJ converted level into Blender [here](cnd2obj.md).*
  ```
     cndtool convert obj <path_to_cnd_file>
  ```

  - Convert level to NDY file format:  
    *Note: NDY file format docs can be found [here](https://github.com/Jones3D-The-Infernal-Engine/Documentation).*
  ```
     cndtool convert ndy <path_to_cnd_file> <path_to_COG_script_folder|path_to_GOB_file>
  ```

  - Extract all game assets:
  ```
     cndtool extract <path_to_cnd_file>
  ```

  - Extract all game assets to specific folder:
  ```
     cndtool extract <path_to_cnd_file> -o=some_folder
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
