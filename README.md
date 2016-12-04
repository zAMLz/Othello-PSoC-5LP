# Othello-Reversi-For-the-PSoC-5LP
Othello for the CY8KIT-050 PSoC 5LP.

## Building the Code

To build the code you will need to ensure that certain dependencies are met.
It is preffered that you use PSoC Creator 3.3 to build the project, but in theory and version of PSoC creator should work as long as you specify the right device. The device we used ourselves was the **CY8C5868AXI-LP035** and it is part of the **PSoC 5 LP** family of devices.

You will also need to downlaod an external library to so that you can read from the SD Card. The Link to the SD card library is provided below and its folder *emFile_V322c* should be placed in the root of this directory. **Do Not Build This Project Without This Library** unless you remove all traces of the SD Card code and the *emFile_1* block from the project. 

http://www.cypress.com/documentation/component-datasheets/file-system-library-emfile

(Make sure the it is v3.22C)

Once those files are downloaded, you may proceed to build the code. If the code does not build because the library above is not correctly configured Please follow these steps to ensure that the library is correctly linked with the linker and the compiler.

  * In the project's top level design, search for EmFile in the component catalog and add it to the schematic.
  * Set up which GPIO pins to use for the SD Card by opening Othello.cydwr in PSoC creator. (These will vary, but if you are following my design, refer to the other documents in the docs folder!)
  * Click on *Project > Build Settings* and add the EmFile Library by going to *ARM GCC > Linker > Additional Libraries* Add a new directory titled "emf32nosnlfn".
  * On additional library directories, locate the folder of the EmFile Library you downloaded and add a new directory for each of the following. **\Code\Include\PSoC5\emf32nOS**, **\LinkLibrary\PSoC5** and **\LinkLibrary\PSoC5\GCC**.
  * Under *ARM GCC > Compiler*, on the additional include directories, add**\Code\Include\PSoC5** and **\Code\Include\PSoC5\emf32nOS**
  
At this point, the project should be able to build without any errors. However, the compiler might throw a warning stating that EmFile has some timing issues. This is not a problem and should be ignored.

## Playing the Game

*Work In Progress*

## Further Documentation

For more details to the project such as the Report and the API, refer to the docs folder.
