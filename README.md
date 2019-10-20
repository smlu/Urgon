# Indiana Jones and the Infernal Machine Mod Tools
This repository contains command line tools (`gobext` and `cndtool`) which can be used to extract and import/replace game resources from `GOB` resource files and `CND` compact level files.
Tools can be downloaded from [Releases](https://github.com/smlu/ProjectMarduk/releases) page.

If you need tool to edit `3do` models and `key` animations use blender add-on: [blender-ijim](https://github.com/smlu/blender-ijim).
To edit `mat` material files use gimp plugin: [gimp-ijim](https://github.com/smlu/gimp-ijim).

## Getting Started
  1. Extract to any directory the `.zip` file downloaded from the [Releases](https://github.com/smlu/ProjectMarduk/releases) page .
  2. Run terminal window. (On windows use `cmd.exe`)
  3. In the opened terminal window change current directory to the root directory of extracted files.
     **Windows example**
     If extracted files are located in `C:\Users\<username>\Desktop\imtools` you would type into terminal window:
        ```
     cd "C:\Users\<username>\Desktop\imtools"
     ```

## Usage
  To run selected tool just enter its name in terminal window and press enter. e.g. `gobext` or `cndtool`.

### gobext
This tool can extract all game resource files ( e.g.: models, scripts, level files etc..) stored in a `*.gob` file.
*Note: For the extracted files to be used in the game they must be put in the `Resource` folder located in root folder where game is installed.*

**Using gobext:**
```
 gobext <path_to_gob_file>
```

To extract files to a specific folder use `-o` flag:
```
 gobext <path_to_gob_file> -o <path_to_output_folder>
```

### cndtool
Multi purpose tool for compact game level files (`.cnd`).
Tool can list, extract, add, replace or remove game resources stored in a `.cnd` file.

Extract:
 - animation files (`.key`)
  - material files (`.mat`)
  - sound files (`.wav`)

Modify:
 - animation files (`.key`)
 - material files (`.mat`)

**Using cndtool:**
```
Indiana Jones and the Infernal Machine CND file tool v0.2.0

Command line interface tool to extract and modify game
resources stored in a CND archive file.

  Usage: cndtool <command> [sub-command] [options]

Commands:                      Description:
  add                            Add or replace game resource
  extract                        Extract game resources
  list                           Print to the console stored game resources
  remove                         Remove one or more game resource
  help                           Show this message
```

Usage example:
   1. Extract resources:
   ```
     cndtool extract <path_to_cnd_file>
 ```
  2. Replace existing material:
  ```
     cndtool add material --replace <path_to_cnd_file> <path_to_mat_file>
 ```

## Building
To compile tools from source code a **C++17** compiler and **CMake** >= 3.6 is required.
How to compile on Linux and macOS:
  1. make sub-directory `build` in the root folder of source code
  2. cd to `build` folder and run:
   ```cmake -DCMAKE_BUILD_TYPE=Release ..```
   3. to compile run: ```make```

How to compile on Windows using VisualStudio 2019:
  1. make subdirectory `build` in the root folder of source code
  2. cd to `build` folder and run:
   ```cmake -DCMAKE_BUILD_TYPE=Release ..```
  3. open generated `.sln` project file with VisualStudio and
  4. compile project in VisualStudio
