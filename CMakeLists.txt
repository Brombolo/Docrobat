cmake_minimum_required(VERSION 3.16)
project(Docrobat LANGUAGES CXX)

# Configure C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(DOCROBAT_SOURCES
    src/main.cpp
    src/App.cpp
    src/MainWindow.cpp
    src/PDFDocument.cpp
    src/PDFView.cpp
    src/PDFObjectEditor.cpp
    src/PDFFormManager.cpp
    src/PDFSecurity.cpp
)

set(DOCROBAT_HEADERS
    src/Annotation.h
    src/App.h
    src/MainWindow.h
    src/PDFDocument.h
    src/PDFView.h
    src/PDFObjectEditor.h
    src/PDFFormManager.h
    src/PDFSecurity.h
)

# Executable target
add_executable(Docrobat ${DOCROBAT_SOURCES} ${DOCROBAT_HEADERS})

# Find native Haiku and poppler-cpp libraries
find_library(BE_LIBRARY be REQUIRED)
find_library(TRACKER_LIBRARY tracker REQUIRED)
find_library(TRANSLATION_LIBRARY translation REQUIRED)
find_library(POPPLER_CPP_LIBRARY poppler-cpp REQUIRED)

# Search for poppler-cpp headers
find_path(POPPLER_CPP_INCLUDE_DIR
    NAMES poppler-document.h
    PATH_SUFFIXES poppler/cpp poppler
)

# Include directories
target_include_directories(Docrobat PRIVATE
    src
    ${POPPLER_CPP_INCLUDE_DIR}
    /boot/system/develop/headers/poppler/cpp
    /boot/system/develop/headers/poppler
)

# Link native Haiku libraries and poppler-cpp (-lbe -ltracker -ltranslation -lpoppler-cpp)
target_link_libraries(Docrobat PRIVATE
    ${BE_LIBRARY}
    ${TRACKER_LIBRARY}
    ${TRANSLATION_LIBRARY}
    ${POPPLER_CPP_LIBRARY}
)
