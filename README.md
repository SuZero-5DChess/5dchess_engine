5dchess_engine
==================

<bold style="color:#ff6347;">**IMPORTANT NOTE**</bold> This module rely on two separate submodules. It is impossible to build the python library without them. Make sure use
```sh
git clone --recurse-submodules <link-to-this-repo>
```
to download both this repository and the necessary submodules.

If interaction with the [graphics interface](https://github.com/SuZero-5DChess/5dchess_client) is preferred, please install `flask` and `flask_socketio` via `pip`.

### Usage (MacOS, etc.)

```sh
mkdir build
cd build
cmake ..
make
```

After that, *go to base directory* and run `host.py`. If the server starts smoothly, the chessboard can be found at <http://127.0.0.1:5000>.

### Usage (Windows)

The CMake program and a modern C++ complier (C++ 20 or newer) is required. I suggest Visual Studio Community version 2022.

```cmd
mkdir build
cd build
cmake ..
cmake --build .
```

The last step is same as above.

### Debugging
It is possible to run the c++ part of the code without interacting with python or web interface at all. It also makes sense to use a modern programming IDE:
```sh
mkdir build_xcode
cd build_xcode
cmake .. -DTEST=on -GXcode
```
On Windows, the last line should be:
```cmd
cmake .. -DTEST=on -G"Visual Studio 17 2022"
```

The performance of this code depends significantly on compiler optimizations. Without optimization, the plain (unoptimized) version may run x6 ~ x7 times slower compared to the same code compiled with `-O3` optimization.

To enable optimizations, configure the build using:

```
cmake .. -DCMAKE_BUILD_TYPE=Release -DTEST=on
```

### Documentation

For more detail, please read [this page](docs/index.md).

### TODOs

- &#9745; use generators to unify code that code that produces one incidence or more 
- &#9745; hide UV coordinate details, deprecate numbers_activated 
- &#9745; support even timeline game
- &#9744; 5dpgn reader for simplified moves
- &#9744; method to get check path
- &#9744; test hypercuboid algorithm
