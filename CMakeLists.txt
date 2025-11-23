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
    src/ui/mainWindow.cpp
    src/services/scholarshipCalculator.cpp
    src/entities/scholarship.cpp
    src/entities/student.cpp
    src/managers/studentDatabase.cpp
    src/ui/studentDialogBuilder.cpp
    src/managers/studentTableManager.cpp
    src/services/studentStatisticsUpdater.cpp
    src/ui/studentHistoryDialog.cpp
    src/services/historyGradeGenerator.cpp
)

set(HEADERS
    include/exceptions/exceptions.h
    include/ui/mainWindow.h
    include/services/scholarshipCalculator.h
    include/entities/scholarship.h
    include/entities/student.h
    include/managers/studentDatabase.h
    include/ui/studentDialogBuilder.h
    include/managers/studentTableManager.h
    include/services/studentStatisticsUpdater.h
    include/ui/studentHistoryDialog.h
    include/services/historyGradeGenerator.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)