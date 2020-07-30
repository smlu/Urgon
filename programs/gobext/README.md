# Command-line tool for GOB file
The **gobext** tool extracts game resource files from a `*.gob` file  (e.g.: models, scripts, level files, etc...).  

*Note: For the extracted files to be used in the game they must be put in the `Resource` folder located in the root folder where the game is installed.*

**Using gobext:**
```
 gobext <path_to_gob_file>
```

To extract files to a specific folder use `-o` flag:
```
 gobext <path_to_gob_file> -o=<path_to_output_folder>
```
