Authors:
- Omer Ovadia – ********
- Gal Jacoby – ********
- Shay Toledo – ********

Project Instructions:

All project files and folders should be placed in the same root directory.

Before compiling and running the program, follow these steps:

1. Grant execution permission to the script (only required once):

      chmod +x fetch_blocks.sh

2. Run the script to generate the "data" folder containing the block text files.  
   Example (to generate 3 blocks):

      ./fetch_blocks.sh 3

3. Compile the project using the Makefile:

      make

4. Run the desired executable, for example:

      ./main5_menu.out


Notes:
- The `data` folder is required for the main programs to function correctly.
- The environment variable `LD_LIBRARY_PATH=.` must be set each time you open a new terminal session, unless added to your `.bashrc`.
