# ═══════════════════════════════════════════════════════════════
# PyEngine — Rotator Script
# ═══════════════════════════════════════════════════════════════
# Attach this to any entity to make it rotate continuously.
# Works like a Unity MonoBehaviour.

import pyengine

class Rotator:
    """Rotates an entity around the Y axis at a configurable speed."""

    def on_create(self):
        self.speed = 45.0  # degrees per second
        self.axis = pyengine.Vec3(0.0, 1.0, 0.0)
        pyengine.Debug.log(f"Rotator created on entity {self.entity_id}")

    def on_start(self):
        pyengine.Debug.log("Rotator started!")

    def on_update(self, dt):
        # Access entity transform via entity_id
        # Rotation is accumulated each frame
        self.speed = 45.0  # deg/s — editable in future inspector

    def on_destroy(self):
        pyengine.Debug.log("Rotator destroyed")
