1. Install [Visual Studio Code](https://code.visualstudio.com/download) x64 version
2. Install the C/C++ extension in Visual Studio Code (For debugging and IntelliSense).
    * Open the Visual Studio Code
    * Select the extensions option in the side tab
    * Search for C++ extension in the marketplace
    * Install the C/C++ extension from the Microsoft
    ![Alt](images\c_cplusplus_visual_studio_code_extension.jpg "C/C++ extension")
3. Install the Mingw-w64 compiler for C++
    * Install the latest version of MSYS2 from https://www.msys2.org/ (Just follow the steps 1 to 5 mentioned on the site)
    * Open the MSYS2 UCRT64 terminal and run the command **pacman -S --needed base-devel mingw-w64-x86_64-toolchain** in the terminal. Proceed the installation with default options. This will install all the required compiler toolchain.
    * Add the path {MSYS2 installation folder}\mingw64\bin to the PATH environment variable.
    * To check that your Mingw-w64 tools are correctly installed and available, open a new Command Prompt and type **g++ --version**
4. Install [git](https://git-scm.com/download/win), if not installed