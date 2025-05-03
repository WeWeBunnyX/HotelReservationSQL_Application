cmake_minimum_required(VERSION 3.16)

project(HotelReservationSQL_Application VERSION 0.1 LANGUAGES CXX)

# Enable automatic handling of Qt-specific files
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)


# List your source files
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    # resources.qrc  # Uncomment when you add a .qrc file
)

# Create the executable and link Qt6 modules
add_executable(HotelReservationSQL_Application ${PROJECT_SOURCES}
    main.cpp
    database.cpp
    database.h
)

target_link_libraries(HotelReservationSQL_Application PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

# Final step for Qt6 to finish configuration (optional but recommended)
qt_finalize_executable(HotelReservationSQL_Application)
