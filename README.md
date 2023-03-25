# CPSMPlongees



## ---- Build ----

<br/><br/>

### Setup

<br/>

#### 1- Protobuf

1. Install *MSys2*
2. Install the [Protobuf package](https://packages.msys2.org/package/mingw-w64-x86_64-protobuf) (version **21.12-1**) in *MSys2*
    ```bash
    pacman -S mingw-w64-x86_64-protobuf
    ```

<br/><br/>

### CMake configuration

Useful variables:
- `BASE_PROTO_PATH`: base protobuf installation path. This folder shall contains the following subfolders: `include`, `lib`, `bin`
    > ***Note***: default is `C:\\msys64\\mingw64`