# Indiana Jones and the Infernal Machine Mod Tools
Repository contains command command-line tools: [**gobext**](programs/gobext), [**cndtool**](programs/cndtool) and [**matool**](programs/matool) for extracting and modifying game assets of the game Indiana Jones and the Infernal Machine.  

**The latest tools can be downloaded from [RELEASES](https://github.com/smlu/Urgon/releases) page.**

If you need tool to edit `3DO` models and `KEY` animations use blender add-on: [**blender-sith**](https://github.com/smlu/blender-sith).  
To edit `MAT` texture files use gimp plugin: [**gimp-ijim**](https://github.com/smlu/gimp-ijim).

<img src="docs/images/cyn.png" alt="Canyonlands opened in Blender" width="600"/>  

*(Canyonlands imported into Blender)*

## Content
  ### Docs
  Documentation folder which contains basic info about the tools.
  More documentation for the game can be found at: https://github.com/Jones3D-The-Infernal-Engine/Documentation

  ### Libraries
  - [**libim**](libraries/libim) - C++ library for parsing and writing game resources (CND/NDY, GOB, MAT, KEY).

  ### Programs
  - [**cndtool**](programs/cndtool) - A multi-purpose tool for compact game level files (`.cnd`).  
    For more info see [README](programs/cndtool/README.md).  

    The cndtool can:
       - add, extract, list replace and remove game assets stored in `CND` file(s). 
       - convert CND file format to [NDY level format](https://github.com/Jones3D-The-Infernal-Engine/Documentation/blob/main/ndy.md) and vice versa.
       - extract and convert level geometry (level surface vertices and surface UV texture vertices) to [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format.

  - [**gobext**](programs/gobext) - A command-line tool for extracting all game resource files (e.g.: models, scripts, level files etc..) from `*.gob` files.  
  For more info see [README](programs/gobext/README.md).

  - [**matool**](programs/matool) - A command-line tool for editing and generating `MAT` texture files.  
  For more info see [README](programs/matool/README.md).

## Building from Scratch

### Prerequisites
  - [**git**](https://git-scm.com/) scm
  - [**CMake**](https://cmake.org/download/) >= 3.15
  - **C++20** supported compiler (gcc, clang, VisualStudio)

### Configure and Build
  1. Clone repository and dependencies:  
  ```
     git clone --recursive https://github.com/smlu/Urgon.git
  ```
  2. Move into directory `Urgon`:
  ```
     cd Urgon
  ```
  3. Make subdirectory `build`: 
  ```
     mkdir build
  ```
  4. Run cmake configure:
  ```
    cmake -DCMAKE_BUILD_TYPE=Release -B build
  ```
  5. Compile
  <pre>
  cmake --build build
  <i>Note: On Windows, when using <b>VisualStudio</b> to configure cmake you can
        open generated <b>*.sln</b> project in VisualStudio and compile it there.</i></pre>