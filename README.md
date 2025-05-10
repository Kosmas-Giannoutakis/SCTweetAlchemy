# SCTweetAlchemy

SCTweetAlchemy is a specialized utility for SuperCollider live coding performers. It allows users to browse, filter, and manage a collection of "SCTweets" - compact SuperCollider code snippets often designed to fit within character limits for sharing. Key functionality includes:

*   **Browse and Filter:** Users can search for specific tweets by name and filter them using categories like Author, UGen type, Sonic Characteristic, and Synthesis Technique.
*   **Favorites System:** Mark and quickly access preferred snippets.
*   **View and Study:** The interface displays the original code, a Ndef-encapsulated version, and metadata (author, tags, description) for each tweet, making it easy to understand what each snippet does before using it.
*   **Data Management:** Add new tweets, edit existing ones, and delete tweets from your collection. Changes are saved to a user-specific file.

This tool serves as a practical library and performance aid for SuperCollider musicians, providing quick access to a diverse collection of sound-generating code snippets that can be rapidly deployed during live performances.

## Planned Features

*   Advanced reformatting of dense SCTweet code into more readable versions using Tree-sitter for live coding contexts.
*   Options for Ndef generation (e.g., adding `.expand`, `.fadeTime`, `SplayAz`).
*   Seamless integration with various live coding IDEs through automatic copy-paste functionality (platform permitting).
*   MP3 preview playback for SCTweets.

## Dependencies

*   **Qt 6 Development Libraries (Widgets module)**: The application is built using the Qt framework.
*   **C++17 Compiler**: (e.g., GCC 7+, Clang 5+, MSVC 2017+)
*   **CMake**: Version 3.16 or higher.
*   **Git**: For cloning and managing submodules.

## Building from Source

1.  **Clone the Repository:**
    Open your terminal and clone the repository. It's recommended to clone with submodules, as this project uses them for the Tree-sitter library and its SuperCollider grammar.
    ```bash
    git clone --recurse-submodules https://github.com/your-username/SCTweetAlchemy_CPP_Project.git 
    cd SCTweetAlchemy_CPP_Project
    ```
    (Replace `your-username/SCTweetAlchemy_CPP_Project.git` with the actual URL of your repository).

    If you cloned without `--recurse-submodules`, navigate into the cloned directory and run:
    ```bash
    git submodule update --init --recursive
    ```

2.  **Create a Build Directory:**
    It's best practice to build out-of-source.
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure with CMake:**
    From within the `build` directory, run CMake to configure the project. You'll need to tell CMake where your Qt 6 installation is located if it's not in a standard system path.

    *   **Linux/macOS Example:**
        ```bash
        # Adjust the CMAKE_PREFIX_PATH to your Qt6 installation directory
        # e.g., /home/user/Qt/6.x.y/gcc_64 or /opt/Qt/6.x.y/clang_64
        cmake -DCMAKE_PREFIX_PATH=/path/to/your/Qt6 .. 
        ```
    *   **Windows (Visual Studio Generator Example):**
        ```bash
        # Ensure you are in a Developer Command Prompt for Visual Studio
        # Adjust the CMAKE_PREFIX_PATH to your Qt6 installation directory (e.g., C:/Qt/6.x.y/msvc2019_64)
        # Adjust the generator if needed (e.g., "Visual Studio 17 2022")
        cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.x.y/msvc2019_64 ..
        ```
    *   **Windows (MinGW Generator Example):**
        ```bash
        # Ensure MinGW and CMake are in your PATH
        # Adjust CMAKE_PREFIX_PATH (e.g., C:/Qt/6.x.y/mingw_XX)
        cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.x.y/mingw_XX ..
        ```
    The `..` at the end points CMake to the parent directory where your main `CMakeLists.txt` is located.

4.  **Build the Project:**
    After CMake configuration is successful, compile the project:
    ```bash
    cmake --build .
    ```
    For multi-core compilation (faster), you can often use:
    *   Linux/macOS: `make -j$(nproc)` (or `make -jN` where N is number of cores)
    *   Windows (Visual Studio): `cmake --build . --config Release` (or `Debug`)
       Alternatively, open the generated `.sln` file in Visual Studio and build from there.

5.  **Run the Application:**
    The executable `SCTweetAlchemy_CPP` (or `SCTweetAlchemy_CPP.exe` on Windows) will be located in your `build` directory (or a subdirectory like `build/Release/` or `build/Debug/` for Visual Studio builds).
    ```bash
    # Linux/macOS
    ./SCTweetAlchemy_CPP

    # Windows (example)
    ./Release/SCTweetAlchemy_CPP.exe 
    ```

## Usage

*   Launch the application.
*   Use the search bar to find tweets by ID or keywords in their description/tags (future).
*   Use the filter panel on the left to narrow down the list of tweets.
*   Select a tweet from the middle list to view its original code, metadata, and Ndef-encapsulated version.
*   Use `Ctrl+D` or the right-click context menu to mark/unmark tweets as favorites.
*   Use the "File" and "Edit" menus to add, edit, or delete tweets from your collection. User-added tweets are saved to a local JSON file (typically in your user's application data directory).

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests. (Add more specific contribution guidelines if you have them).

## License

(Specify your project's license here, e.g., MIT, GPLv3, etc.)
