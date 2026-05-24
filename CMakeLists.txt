cmake_minimum_required(VERSION 3.10)

# ============================================================
# STEP 1 — Toolchain MUST come before project()
# ============================================================
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-none-eabi-gcc CACHE STRING "C compiler")
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc CACHE STRING "ASM compiler")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ============================================================
# STEP 2 — Project declaration
# ============================================================
project(DualElevator C ASM)

get_filename_component(TOOLCHAIN_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
if(TOOLCHAIN_DIR)
    set(CMAKE_OBJCOPY "${TOOLCHAIN_DIR}/arm-none-eabi-objcopy")
    set(CMAKE_SIZE    "${TOOLCHAIN_DIR}/arm-none-eabi-size")
else()
    set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
    set(CMAKE_SIZE    arm-none-eabi-size)
endif()

# ============================================================
# STEP 3 — MCU compiler/linker flags
# ============================================================
set(CPU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

# ============================================================
# STEP 4 — Shared driver sources (used by BOTH targets)
# ============================================================
set(SHARED_SOURCES
    Gpio/Gpio.c
    Rcc/Rcc.c
    Spi/Spi.c
    Timer/Timer.c
    Pwm/Pwm.c
    nvic/Nvic.c
    Exti/Exti.c
    Uart/Uart.c
    Ipc/Ipc.c
    Dma/Dma.c
    state_machine/State_Machine.c
    STM32F401xE.s
)

# ============================================================
# STEP 5 — Include paths (shared)
# ============================================================
set(INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/main
    ${CMAKE_SOURCE_DIR}/Gpio
    ${CMAKE_SOURCE_DIR}/Rcc
    ${CMAKE_SOURCE_DIR}/Spi
    ${CMAKE_SOURCE_DIR}/Timer
    ${CMAKE_SOURCE_DIR}/Pwm
    ${CMAKE_SOURCE_DIR}/Common
    ${CMAKE_SOURCE_DIR}/nvic
    ${CMAKE_SOURCE_DIR}/Exti
    ${CMAKE_SOURCE_DIR}/Uart
    ${CMAKE_SOURCE_DIR}/Ipc
    ${CMAKE_SOURCE_DIR}/state_machine
    ${CMAKE_SOURCE_DIR}/Dispatcher
    ${CMAKE_SOURCE_DIR}/Config
    ${CMAKE_SOURCE_DIR}/Dma

)

# ============================================================
# STEP 6 — Common compile options
# ============================================================
set(COMMON_COMPILE_OPTIONS
    ${CPU_FLAGS}
    -std=c11
    -Wall
    -ffunction-sections
    -fdata-sections
    -fno-common
)

# ============================================================
# STEP 7 — Common linker options
# ============================================================
set(COMMON_LINK_OPTIONS
    ${CPU_FLAGS}
    -L ${CMAKE_SOURCE_DIR}
    -T ${CMAKE_SOURCE_DIR}/STM32F401xE.ld
    -Wl,--gc-sections
    --specs=nosys.specs
    --specs=nano.specs
    -lm
)

# ============================================================
# STEP 8 — Master.elf target
# ============================================================
add_executable(Master.elf
    ${SHARED_SOURCES}
    Dispatcher/Dispatcher.c
    main/main_master.c
)

target_include_directories(Master.elf PRIVATE ${INCLUDE_DIRS})
target_compile_options(Master.elf PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(Master.elf PRIVATE BOARD_ROLE_MASTER=1)
target_link_options(Master.elf PRIVATE
    ${COMMON_LINK_OPTIONS}
    -Wl,-Map=Master.map
)

add_custom_command(TARGET Master.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex Master.elf Master.hex
    COMMENT "Generating Master.hex for Proteus"
)
add_custom_command(TARGET Master.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} --format=berkeley Master.elf
    COMMENT "Master ELF size"
)

# ============================================================
# STEP 9 — Slave.elf target
# ============================================================
add_executable(Slave.elf
    ${SHARED_SOURCES}
    main/main_slave.c
)

target_include_directories(Slave.elf PRIVATE ${INCLUDE_DIRS})
target_compile_options(Slave.elf PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(Slave.elf PRIVATE BOARD_ROLE_SLAVE=1)
target_link_options(Slave.elf PRIVATE
    ${COMMON_LINK_OPTIONS}
    -Wl,-Map=Slave.map
)

add_custom_command(TARGET Slave.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex Slave.elf Slave.hex
    COMMENT "Generating Slave.hex for Proteus"
)
add_custom_command(TARGET Slave.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} --format=berkeley Slave.elf
    COMMENT "Slave ELF size"
)