cmake_minimum_required(VERSION 3.0)
project(sticker)

set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)

# also tested with Qt6. Gui for drawing routines.
# Designed for high portability *within a Qt universe*
find_package(Qt5 COMPONENTS Gui REQUIRED)


add_executable(sticker main.cpp StickerGenerator.cpp Entities.cpp)
target_link_libraries(sticker Qt::Gui)
