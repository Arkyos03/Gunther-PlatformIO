# Gunther.ino - PlatformIO build script (compile, upload, size)
# Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License

import os
import sys

from SCons.Script import (ARGUMENTS, COMMAND_LINE_TARGETS, AlwaysBuild,
                          Builder, Default, DefaultEnvironment)

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

env.Replace(
    AR="arm-none-eabi-gcc-ar",
    AS="arm-none-eabi-as",
    CC="arm-none-eabi-gcc",
    CXX="arm-none-eabi-g++",
    GDB="arm-none-eabi-gdb",
    OBJCOPY="arm-none-eabi-objcopy",
    RANLIB="arm-none-eabi-gcc-ranlib",
    SIZETOOL="arm-none-eabi-size",
    ARFLAGS=["rc"],
    SIZEPROGREGEXP=r"^(?:\.text|\.data|\.rodata|\.text\.align|\.ARM\.exidx)\s+(\d+).*",
    SIZEDATAREGEXP=r"^(?:\.data|\.bss|\.noinit)\s+(\d+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",
    PROGSUFFIX=".elf",
)

# some toolchains have no gcc-ar / gcc-ranlib
if not any(os.path.isfile(os.path.join(p, "arm-none-eabi-gcc-ar"))
           for p in env["ENV"].get("PATH", "").split(os.pathsep)):
    env.Replace(AR="arm-none-eabi-ar", RANLIB="arm-none-eabi-ranlib")

env.Append(
    BUILDERS=dict(
        ElfToBin=Builder(
            action=env.VerboseAction(
                "$OBJCOPY -O binary $SOURCES $TARGET", "Building $TARGET"),
            suffix=".bin",
        )
    )
)

if not env.get("PIOFRAMEWORK"):
    sys.stderr.write("Error: use  framework = arduino  in platformio.ini\n")
    env.Exit(1)

# ------------------------------------------------------------------ build
if "nobuild" in COMMAND_LINE_TARGETS:
    target_elf = os.path.join("$BUILD_DIR", "${PROGNAME}.elf")
    target_firm = os.path.join("$BUILD_DIR", "${PROGNAME}.bin")
else:
    target_elf = env.BuildProgram()
    target_firm = env.ElfToBin(os.path.join("$BUILD_DIR", "${PROGNAME}"), target_elf)
    env.Depends(target_firm, "checkprogsize")

AlwaysBuild(env.Alias("nobuild", target_firm))
target_buildprog = env.Alias("buildprog", target_firm, target_firm)

target_size = env.Alias(
    "size", target_elf, env.VerboseAction("$SIZEPRINTCMD", "Calculating size $SOURCE"))
AlwaysBuild(target_size)

# ------------------------------------------------------------------ upload
upload_protocol = env.subst("$UPLOAD_PROTOCOL") or "sam-ba"
upload_options = board.get("upload", {})
offset = upload_options.get("offset_address", "0x4000")
upload_actions = []


def list_ports():
    try:
        from platformio.public import list_serial_ports
    except ImportError:  # older PlatformIO
        from platformio.util import get_serial_ports as list_serial_ports
    return list_serial_ports()


def before_upload(target, source, env):  # pylint: disable=unused-argument
    env.AutodetectUploadPort()
    before_ports = list_ports()
    if upload_options.get("use_1200bps_touch", False):
        env.TouchSerialPort("$UPLOAD_PORT", 1200)      # reboot into the UF2 bootloader
    if upload_options.get("wait_for_upload_port", False):
        env.Replace(UPLOAD_PORT=env.WaitForNewSerialPort(before_ports))
    # bossac wants the port name only on Linux / macOS
    port = env.subst("$UPLOAD_PORT")
    if port.startswith("/dev/"):
        env.Replace(UPLOAD_PORT=os.path.basename(port))


if upload_protocol == "sam-ba":
    env.Replace(
        UPLOADER="bossac",
        UPLOADERFLAGS=["--port", '"$UPLOAD_PORT"', "-U", "-i",
                       "--offset=%s" % offset, "-w", "-v", "-R"],
        UPLOADCMD="$UPLOADER $UPLOADERFLAGS $SOURCES",
    )
    if int(ARGUMENTS.get("PIOVERBOSE", 0)):
        env.Prepend(UPLOADERFLAGS=["-i", "-d"])
    upload_actions = [
        env.VerboseAction(before_upload, "Looking for upload port..."),
        env.VerboseAction("$UPLOADCMD", "Uploading $SOURCE"),
    ]

elif upload_protocol in ("cmsis-dap", "stlink"):
    interface = {"cmsis-dap": ("interface/cmsis-dap.cfg", "swd"),
                 "stlink": ("interface/stlink.cfg", "hla_swd")}[upload_protocol]
    ocd = platform.get_package_dir("tool-openocd") or ""
    env.Replace(
        UPLOADER=os.path.join(ocd, "bin", "openocd"),
        UPLOADERFLAGS=[
            "-s", os.path.join(ocd, "openocd", "scripts"),
            "-s", os.path.join(ocd, "share", "openocd", "scripts"),
            "-f", interface[0],
            "-c", '"transport select %s"' % interface[1],
            "-f", "target/%s.cfg" % board.get("debug.openocd_target", "atsame5x"),
            "-c", '"program {$SOURCE} verify reset %s; shutdown"' % offset,
        ],
        UPLOADCMD="$UPLOADER $UPLOADERFLAGS",
    )
    upload_actions = [env.VerboseAction("$UPLOADCMD", "Uploading $SOURCE")]

elif upload_protocol == "custom":
    upload_actions = [env.VerboseAction("$UPLOADCMD", "Uploading $SOURCE")]

else:
    sys.stderr.write("Warning! Unknown upload protocol %s\n" % upload_protocol)

AlwaysBuild(env.Alias("upload", target_firm, upload_actions))

Default([target_buildprog, target_size])
