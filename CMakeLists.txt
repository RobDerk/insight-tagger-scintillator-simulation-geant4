# First Test and Simulation Project with Geant4

cmake_minimum_required(VERSION 3.16...3.27)
project(FirstSimulation)

find_package(Geant4 REQUIRED ui_all vis_all)

# Location of the soruces and header files fpr this project
file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/*.cc
)

file(GLOB headers
    ${PROJECT_SOURCE_DIR}/include/*.hh
)

# Execuatble
add_executable(
    Scintillation_Simulation
    Scintillation_Simulation.cc
    ${sources}
    ${headers}
)

target_include_directories(
    Scintillation_Simulation
    PRIVATE
    include
)

target_link_libraries(
    Scintillation_Simulation
    PRIVATE
    ${Geant4_LIBRARIES}
)

add_custom_target(FirstSimulation DEPENDS Scintillation_Simulation)

set(FSIM_MACROS
    init_vis.mac
    run.mac
    vis.mac
)

foreach(macro ${FSIM_MACROS})
    configure_file(
        ${PROJECT_SOURCE_DIR}/${macro}
        ${PROJECT_BINARY_DIR}/${macro}
        COPYONLY
    )
endforeach()