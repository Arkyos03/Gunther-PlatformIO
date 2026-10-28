# Gunther.ino - PlatformIO development platform
# Copyright (c) 2026 Arkyos03 for AIR ESIEA - MIT License

import os

from platformio.public import PlatformBase

# SWD probes: (PlatformIO name, OpenOCD interface script, transport)
PROBES = (
    ("cmsis-dap", "interface/cmsis-dap.cfg", "swd"),
    ("stlink", "interface/stlink.cfg", "hla_swd"),
)


class GuntherPlatform(PlatformBase):

    def get_boards(self, id_=None):
        result = super().get_boards(id_)
        if not result:
            return result
        if id_:
            return self._add_debug_tools(result)
        for key in result:
            result[key] = self._add_debug_tools(result[key])
        return result

    def _add_debug_tools(self, board):
        debug = board.manifest.get("debug", {})
        tools = debug.setdefault("tools", {})
        target = debug.get("openocd_target", "atsame5x")
        for name, interface, transport in PROBES:
            if name in tools:
                continue
            tools[name] = {
                "server": {
                    "package": "tool-openocd",
                    "executable": "bin/openocd",
                    "arguments": [
                        "-s", "$PACKAGE_DIR/openocd/scripts",
                        "-s", "$PACKAGE_DIR/share/openocd/scripts",
                        "-f", interface,
                        "-c", "transport select %s" % transport,
                        "-f", "target/%s.cfg" % target,
                    ],
                },
                "onboard": False,
            }
        board.manifest["debug"] = debug
        return board
