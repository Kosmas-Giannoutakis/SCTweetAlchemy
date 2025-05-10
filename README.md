# SCTweetAlchemy

SCTweetAlchemy is a specialized utility for SuperCollider live coding performers. It allows users to browse, filter, and manage a collection of "SCTweets" - compact SuperCollider code snippets often designed to fit within character limits for sharing. Key functionality includes:

*   **Browse and Filter:** Users can search for specific tweets by name and filter them using categories like Author, UGen type, Sonic Characteristic, and Synthesis Technique.
*   **Favorites System:** Mark and quickly access preferred snippets.
*   **View and Study:** The interface displays the original code, a Ndef-encapsulated version (with formatting options), and metadata (author, tags, description) for each tweet.
*   **Data Management:** Add new tweets, edit existing ones, and delete tweets from your collection. Changes are saved to a user-specific file, preserving the original dataset.

This tool serves as a practical library and performance aid for SuperCollider musicians, providing quick access to a diverse collection of sound-generating code snippets that can be rapidly deployed during live performances.

## Dependencies

*   **Qt 6 Development Libraries (Widgets module)**: The application is built using the Qt framework.
*   **C++17 Compiler**: (e.g., GCC 7+, Clang 5+, MSVC 2017+)
*   **CMake**: Version 3.16 or higher.
*   **Git**: For cloning the repository and its submodules.
*   **Submodules:** This project uses Git submodules to include:
    *   The [Tree-sitter](https://github.com/tree-sitter/tree-sitter) parsing library runtime.
    *   The [tree-sitter-supercollider](https://github.com/madskjeldgaard/tree-sitter-supercollider) grammar.
    These are fetched automatically when cloning with the `--recurse-submodules` flag or by running `git submodule update --init --recursive`.

## Building from Source

1.  **Clone the Repository (with Submodules):**
    Open your terminal and clone the repository. The `--recurse-submodules` flag is important.
    ```bash
    git clone --recurse-submodules https://github.com/Kosmas-Giannoutakis/SCTweetAlchemy.git
    cd SCTweetAlchemy
    ```

    If you have already cloned without `--recurse-submodules`, navigate into the cloned directory and run:
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
        # Adjust CMAKE_PREFIX_PATH (e.g., C:/Qt/6.x.y/msvc2019_64) and Generator if needed.
        cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.x.y/msvc2022_64 ..
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
    cmake --build . --config Release
    ```
    (Change `Release` to `Debug` for a debug build).
    For multi-core compilation (faster) with Makefiles or Ninja, you can often append `-jN` (e.g., `make -j8` or `cmake --build . --config Release -- -j8`).

5.  **Run the Application:**
    The executable `SCTweetAlchemy_CPP` (or `SCTweetAlchemy_CPP.exe` on Windows) will be located in your `build` directory (or a configuration-specific subdirectory like `build/Release/` for Visual Studio and some other generators).
    ```bash
    # Linux/macOS (if built directly in 'build')
    ./SCTweetAlchemy_CPP

    # Windows (example, if built in 'build/Release')
    ./Release/SCTweetAlchemy_CPP.exe 
    ```

## Usage

*   Launch the application.
*   Use the search bar to find tweets by ID.
*   Use the filter panel on the left to narrow down the list of tweets based on Author, Sonic Characteristic, Synthesis Technique, or UGen.
*   Select a tweet from the middle list to view its original code, associated metadata, and an Ndef-encapsulated version.
*   **Mark/Unmark Favorites:**
    *   Double-click a tweet in the list.
    *   Select a tweet and press `Ctrl+D` (or `Cmd+D` on macOS).
    *   Right-click a tweet and select "Toggle Favorite" from the context menu.
*   Use the "File" and "Edit" menus to add new tweets, edit existing tweet metadata/code, or delete tweets. User-added/modified tweets are saved to `SCTweets_user.json` in your user's application data directory.
*   The Ndef panel has options to control the formatting of the generated Ndef string.

## Contributing

Please feel free to submit issues or pull requests.

## License

This project is free software available under Version 3 the GNU General Public License. See [COPYING](COPYING) for details.
