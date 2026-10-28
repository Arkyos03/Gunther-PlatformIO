# Gunther.ino - Arduino framework for PlatformIO
# Uses exactly the same core, variants and libraries as the Arduino IDE package.
# Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License

import os

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

FRAMEWORK_DIR = os.path.join(platform.get_dir(), "framework")
assert os.path.isdir(FRAMEWORK_DIR), "framework folder missing in the platform"

CORE = board.get("build.core", "gunther")
CORE_DIR = os.path.join(FRAMEWORK_DIR, "cores", CORE)
VARIANT_DIR = os.path.join(FRAMEWORK_DIR, "variants", board.get("build.variant"))
LDSCRIPT_DIR = os.path.join(VARIANT_DIR, "linker_scripts", "gcc")

machine_flags = ["-mcpu=%s" % board.get("build.cpu"), "-mthumb",
                 "-mfloat-abi=hard", "-mfpu=fpv4-sp-d16"]

env.Append(
    ASFLAGS=machine_flags + ["-x", "assembler-with-cpp"],
    CFLAGS=["-std=gnu11"],
    CCFLAGS=machine_flags + [
        "-Os",
        "-ffunction-sections",
        "-fdata-sections",
        "-nostdlib",
        "--param", "max-inline-insns-single=500",
    ],
    CXXFLAGS=["-std=gnu++11", "-fno-threadsafe-statics", "-fno-rtti", "-fno-exceptions"],
    CPPDEFINES=[
        ("F_CPU", "$BOARD_F_CPU"),
        ("ARDUINO", 10819),
        "ARDUINO_ARCH_SAMD51GUNTHER",
        ("USB_VID", board.get("build.hwids")[0][0]),
        ("USB_PID", board.get("build.hwids")[0][1]),
    ],
    CPPPATH=[
        CORE_DIR,
        os.path.join(CORE_DIR, "cmsis", "Core", "Include"),
        os.path.join(CORE_DIR, "cmsis", "Device", "samd51", "include"),
        os.path.join(CORE_DIR, "tinyusb", "src"),
        VARIANT_DIR,
    ],
    LINKFLAGS=machine_flags + [
        "-Os",
        "-Wl,--gc-sections",
        "--specs=nano.specs",
        "-u", "_printf_float",
    ],
    LIBPATH=[LDSCRIPT_DIR],
    LIBS=["m", "c", "gcc"],
    LIBSOURCE_DIRS=[os.path.join(FRAMEWORK_DIR, "libraries")],
)

# board extra flags (-D__SAMD51J20A__ -DARDUINO_GUNTHER)
env.ProcessFlags(board.get("build.extra_flags", ""))

# linker script (UF2 bootloader by default, platformio.ini: board_build.ldscript to change it)
if not board.get("build.ldscript", ""):
    env.Replace(LDSCRIPT_PATH=os.path.join(
        LDSCRIPT_DIR, board.get("build.arduino.ldscript", "flash_with_bootloader.ld")))

# circular references between the core and the libraries
env.Prepend(_LIBFLAGS="-Wl,--start-group ")
env.Append(_LIBFLAGS=" -Wl,--end-group")

# core and variant compiled as objects (not archives): vector table always kept
env.BuildSources(os.path.join("$BUILD_DIR", "FrameworkArduinoVariant"), VARIANT_DIR)
env.BuildSources(os.path.join("$BUILD_DIR", "FrameworkArduino"), CORE_DIR)
