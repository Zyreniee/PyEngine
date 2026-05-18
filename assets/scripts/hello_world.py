# ═══════════════════════════════════════════════════════════════
# PyEngine — Hello World Script (Module-Level)
# ═══════════════════════════════════════════════════════════════
# Simplest possible script — no class needed.
# Just define module-level functions.

import pyengine

def on_create():
    pyengine.log_info("Hello from PyEngine!")
    pyengine.log_info("This is the simplest script format.")

def on_start():
    pyengine.log_info("Script started!")

def on_update(dt):
    # This runs every frame
    if pyengine.is_key_pressed(pyengine.KEY_H):
        pyengine.log_info("H key pressed!")

def on_destroy():
    pyengine.log_info("Goodbye from PyEngine!")
