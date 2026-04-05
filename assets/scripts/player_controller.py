# ═══════════════════════════════════════════════════════════════
# PyEngine Example Script — Player Controller
# ═══════════════════════════════════════════════════════════════
# Attach this script to any entity to control it with WASD keys.
# This demonstrates the Python scripting API of PyEngine

import pyengine

# Movement speed (units per second)
SPEED = 5.0

# Entity transform — set by the engine before on_start()
_position = pyengine.Vec3(0.0, 0.0, 0.0)


def on_create():
    pyengine.log_info("PlayerController script created!")


def on_start():
    pyengine.log_info("PlayerController ready — Use WASD to move, Space/Ctrl for up/down")


def on_update(dt):
    global _position

    velocity = SPEED * dt

    # Forward / Backward
    if pyengine.is_key_pressed(pyengine.KEY_W):
        _position.z -= velocity
    if pyengine.is_key_pressed(pyengine.KEY_S):
        _position.z += velocity

    # Left / Right
    if pyengine.is_key_pressed(pyengine.KEY_A):
        _position.x -= velocity
    if pyengine.is_key_pressed(pyengine.KEY_D):
        _position.x += velocity

    # Up / Down
    if pyengine.is_key_pressed(pyengine.KEY_SPACE):
        _position.y += velocity
    if pyengine.is_key_pressed(pyengine.KEY_LEFT_CONTROL):
        _position.y -= velocity


def on_destroy():
    pyengine.log_info("PlayerController destroyed")
