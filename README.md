# Circuit Breaker

## Windows Binaries:
Release builds for windows can be downloaded from [this drive link](https://drive.google.com/drive/folders/1yXyVN6x6JoO25Hn-unKxrLh_0AZRWiRO)

## Windows Build Instructions:

1. Download PhysX from [this drive link](https://drive.google.com/file/d/1B7-nrPuDbISO0T2DwgRJ0B8NHrhkncgF/view?usp=sharing)
    - if this isn't working you can also download just physX from [here](https://drive.google.com/file/d/1pEJwnzxcU9dmLWXS2b9Mpow9S-b2PFo7/view?usp=sharing)
    and add the rapidjson to the include folder yourself (this can be dowloaded from [here](https://drive.google.com/file/d/1-qeJBIDXejTcsveHHSgDjzXxQQXB2zJs/view?usp=sharing))

1. Extract folder to /dependencies/physx
    - you should end up with dependencies/physX/include, dependencies/physX/debug, etc.

3. Open project in Visual Studio. it should detect `CmakeLists.txt` and build/run automatically.
    - If running a different compiler ```set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")``` will have to be changed in ```CMakelists.txt```.
    - Naturally if building the release version, it would be changed to the repective release config.


---
## Build Instructions For Linux:

1. add physX files to dependencies/physx as shown in tutorial 1.1
    - the entire physX/include folder is required, as well as the binaries for the build modes you want to use (debug, checked, profile, or release)
    - you should end up with 2 folders in dependencies/physX: include, and debug (plus any other build modes you want to use)

1. add rapidjson folder to the physx/include folder
    - this can be from the physx base folder, or this [drive link](https://drive.google.com/file/d/1-qeJBIDXejTcsveHHSgDjzXxQQXB2zJs/view?usp=sharing)

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
    ```
    ./circuit-breaker
    ```

---
## Game Rules
- In order to win the race, a player must complete 3 full laps around the race track.
- Each player begins with maximum health points (HP), and zero boost.
    - The amount of boost that a player has is inversely proportional to the amount of health that they have.
    - To regain boost, a player must drift. Using the handbrake can help accomplish this.
- If a player loses all their health, they are destroyed and are forfeited from the race.
      - A player can deplete another players' health by driving or ramming into them. This can also be done by shimmying into another player.  

## Controls:

### Controller:
    - L-Stick: Turn 
    - R-Stick: Rotate Camera 
    - R-Trigger: accelerate
    - LB/L1: Shimmy left
    - RB/R1: Shimmy right
    - L-Trigger: Brake
    - LT + RT: Reverse
    - B/◯: Boost
    - Y/△: Look Backwards
    - A/X: Handbrake

### Keyboard:
    - A/D: Turn
    - Q/E: Rotate Camera
    - W: Accelerate
    - S: Brake
    - S + W: Reverse
    - J: Shimmy left
    - L: Shimmy right
    - Space: Boost
    - Back Space: Reset
    - X: Look Backwards
    - K: Handbrake

- Reversing happens when the brake and throttle are both active when the car is stationary.

---
## Members:
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
