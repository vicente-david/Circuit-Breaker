# Compilation instructions for Windows:

## PhysX download:

- Step 1: Download PhysX from https://drive.google.com/file/d/1pEJwnzxcU9dmLWXS2b9Mpow9S-b2PFo7/view?usp=sharing
- Step 2: Extract the folder to ```/dependencies``` 

If the naming is wrong cmake will throw an error, so double check the paths.<br>
<br>
<br>
## 
Cmake configuration will need to be changed depending on debug or release mode (more information in ```CMakelists windows.txt```) <br>

After renaming ```CMakelists windows.txt``` to ```CMakelists.txt``` you can compile natively. 
<br>
<br>
Most of the development team uses the MSVC compiler, so loading the source folder will enable Visual Studio to detect ```CMakelists.txt``` and build automatically.
If running a different compiler ```set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")``` will have to be changed in ```CMakelists.txt```.
Naturally if building the release version, it would be changed to the repective release config.

### As an alternative you may download the release version from the following link
