cmake_minimum_required(VERSION 3.16)

project(SCTweetAlchemy_CPP VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- Tree-sitter Integration ---
# Include directory for the Tree-sitter CORE API headers (from the runtime clone)
include_directories(extern/tree-sitter-runtime/lib/include)

# Include directory for the SC grammar's local headers (it might have its own tree_sitter/parser.h)
# This is needed for SC's parser.c to find its includes if they are relative like "tree_sitter/parser.h"
include_directories(extern/tree-sitter-supercollider/src) 

add_executable(SCTweetAlchemy_CPP
    main.cpp
    # ... (your existing .cpp and .h files for SCTweetAlchemy) ...
    mainwindow.h mainwindow.cpp
    searchlineedit.h searchlineedit.cpp
    tweetdata.h 
    tweetrepository.h tweetrepository.cpp
    favoritesmanager.h favoritesmanager.cpp
    filterpanelwidget.h filterpanelwidget.cpp
    tweetfilterengine.h tweetfilterengine.cpp
    tweeteditdialog.h tweeteditdialog.cpp 
    ndefgenerator.h ndefgenerator.cpp
    sccodeprettyprinter.h sccodeprettyprinter.cpp
    # sccodeprettyprinter.h sccodeprettyprinter.cpp # We will add this class next

    # Tree-sitter runtime library source (compiles lib.c which includes the others from its own dir)
    extern/tree-sitter-runtime/lib/src/lib.c 

    # SuperCollider grammar parser and scanner sources
    extern/tree-sitter-supercollider/src/parser.c
    extern/tree-sitter-supercollider/src/scanner.c
    
    resources.qrc
)

# Optional: Suppress warnings for external C code
set_source_files_properties(
    extern/tree-sitter-runtime/lib/src/lib.c
    extern/tree-sitter-supercollider/src/parser.c
    extern/tree-sitter-supercollider/src/scanner.c
    PROPERTIES COMPILE_FLAGS "-w" 
)

target_link_libraries(SCTweetAlchemy_CPP PRIVATE Qt6::Widgets)