# Circuit Breaker

## Compilation instructions for Windows:

## PhysX download:

- Step 1: Download PhysX from https://drive.google.com/file/d/1pEJwnzxcU9dmLWXS2b9Mpow9S-b2PFo7/view?usp=sharing
- Step 2: Extract the folder to ```/dependencies``` 

If the naming is wrong cmake will throw an error, so double check the paths.<br>
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


## Build Instructions For Linux

1. add physX files to dependencies/physx as shown in tutorial 1.1
    - the entire physX/include folder is required, as well as the binaries for the build modes you want to use (debug, checked, profile, or release)
    - you shuold end up with 2 folders in dependencies/physX: include, and debug (plus any other build modes you want to use)

1. copy the linux cmake file
    ```
    cp "CMakeLists linux.txt" CMakeLists.txt
    ```

1. create build folder
    ```
    mkdir out
    cd out
    ```

1. run cmake/make (you wll need to resolve any dependencies you are missing)
    ```
    cmake ..
    make
    ```

1. run the generated execuatable!




## Members
Vicente David
vicente.david@ucalgary.ca

Matthew Milum
matthew.milum@ucalgary.ca

Keerat Singh
keerat.singh@ucalgary.ca

Adnan Twakkal
adnan.twakkal1@ucalgary.ca

Sofie Curtis
sofie.curtis@ucalgary.ca
