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

