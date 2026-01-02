cmake_minimum_required(VERSION 3.10)
project(sticker)

set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)

# Tested with Qt5 and Qt6. Gui for drawing routines.
# Designed for high portability *within a Qt universe*
find_package(Qt6 COMPONENTS Gui QUIET)
if (Qt6_FOUND)
    set(QT_GUI_LIB Qt6::Gui)
else()
    find_package(Qt5 COMPONENTS Gui REQUIRED)
    set(QT_GUI_LIB Qt5::Gui)
endif()


add_executable(sticker main.cpp StickerGenerator.cpp Entities.cpp)
target_link_libraries(sticker Qt::Gui)
