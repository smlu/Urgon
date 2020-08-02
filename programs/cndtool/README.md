# Command-line tool for CND level file
**cndtool** is a multi-purpose tool for compact game level files (`.cnd`).
The tool can list, extract, add, replace, or remove game assets stored in a `CND` file. It can also extract and convert level geometry resources (level surface vertices and surface UV texture vertices) to [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) file format.

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

## Docs
  * [Command description and usage](../../docs/cndtool.md)
  * [Tutorial converting CND to OBJ and importing in Blender](../../docs/cnd2obj.md)