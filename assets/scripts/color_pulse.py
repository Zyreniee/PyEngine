# ═══════════════════════════════════════════════════════════════
# PyEngine — Color Pulse Script
# ═══════════════════════════════════════════════════════════════
# Makes an object pulse between two colors. Demonstrates
# how to interact with material properties from Python.

import pyengine
import math

class ColorPulse:
    """Smoothly pulses an entity's color between two values."""

    def on_create(self):
        self.color_a = pyengine.Vec4(0.1, 0.5, 0.9, 1.0)  # Blue
        self.color_b = pyengine.Vec4(0.9, 0.2, 0.3, 1.0)  # Red
        self.speed = 2.0
        self.time = 0.0
        pyengine.Debug.log("ColorPulse initialized")

    def on_update(self, dt):
        self.time += dt * self.speed
        t = (math.sin(self.time) + 1.0) * 0.5  # 0..1 oscillation

    def on_destroy(self):
        pyengine.Debug.log("ColorPulse destroyed")
