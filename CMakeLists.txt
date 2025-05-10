cmake_minimum_required(VERSION 3.16)

project(SCTweetAlchemy_CPP VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Find Qt ---
# Find the QtWidgets component, which transitively includes Core and Gui
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# --- Qt Automatic Helpers ---
# Enable AUTOMOC, AUTOUIC, AUTORCC for simplifying Qt development
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# --- Add Executable ---
add_executable(SCTweetAlchemy_CPP
    main.cpp
    searchlineedit.h searchlineedit.cpp
    tweetdata.h 
    tweetrepository.h tweetrepository.cpp
    favoritesmanager.h favoritesmanager.cpp
    filterpanelwidget.h filterpanelwidget.cpp
    tweetfilterengine.h tweetfilterengine.cpp
    mainwindow.h mainwindow.cpp
    tweeteditdialog.h tweeteditdialog.cpp
    ndefgenerator.h ndefgenerator.cpp
    resources.qrc
)

# --- Link Libraries ---
target_link_libraries(SCTweetAlchemy_CPP PRIVATE Qt6::Widgets)

# --- Optional: Define location of SCTweets.json ---
# This is tricky. Simplest for now: assume it's copied next to the executable.
# More robust solutions involve resource files or install steps.
# We'll handle finding it in the C++ code for now.

# --- (Future) Add Tree-sitter ---
# We will add sections here later to find/compile/link Tree-sitter
# and the SuperCollider grammar.