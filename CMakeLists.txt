cmake_minimum_required(VERSION 3.16)
project(Scholar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Qt6_DIR "C:/msys64/mingw64/lib/cmake/Qt6")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC_EXECUTABLE "C:/msys64/mingw64/share/qt6/bin/moc.exe")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    src/main.cpp
    src/mainWindow.cpp
    src/scholarshipCalculator.cpp
    src/student.cpp
    src/studentDatabase.cpp
    src/studentDialogBuilder.cpp
    src/studentTableManager.cpp
    src/studentStatisticsUpdater.cpp
    src/studentHistoryDialog.cpp
    src/historyGradeGenerator.cpp
)

set(HEADERS
    include/exceptions.h
    include/mainWindow.h
    include/scholarshipcalculator.h
    include/student.h
    include/studentDatabase.h
    include/studentDialogBuilder.h
    include/studentTableManager.h
    include/studentStatisticsUpdater.h
    include/studentHistoryDialog.h
    include/historyGradeGenerator.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)