# ═══════════════════════════════════════════════════════════════
# PyEngine — Player Controller Script
# ═══════════════════════════════════════════════════════════════
# WASD movement controller. Attach to any entity with a Transform.
# Similar to Unity's CharacterController but using direct transform.

import pyengine

class PlayerController:
    """Basic WASD movement controller for PyEngine."""

    def on_create(self):
        self.move_speed = 5.0
        self.sprint_speed = 10.0
        self.jump_force = 8.0
        self.is_grounded = True
        pyengine.Debug.log("PlayerController initialized")

    def on_start(self):
        pyengine.Debug.log(f"PlayerController started on entity {self.entity_id}")

    def on_update(self, dt):
        speed = self.move_speed

        # Sprint
        if pyengine.Input.is_key_pressed(pyengine.KEY_LEFT_SHIFT):
            speed = self.sprint_speed

        # Movement direction
        move_dir = pyengine.Vec3.zero()

        if pyengine.Input.is_key_pressed(pyengine.KEY_W):
            move_dir.z -= 1.0
        if pyengine.Input.is_key_pressed(pyengine.KEY_S):
            move_dir.z += 1.0
        if pyengine.Input.is_key_pressed(pyengine.KEY_A):
            move_dir.x -= 1.0
        if pyengine.Input.is_key_pressed(pyengine.KEY_D):
            move_dir.x += 1.0

        # Normalize and apply speed
        length = move_dir.length()
        if length > 0.001:
            move_dir = move_dir.normalized() * speed * dt

        # Jump
        if pyengine.Input.is_key_pressed(pyengine.KEY_SPACE) and self.is_grounded:
            move_dir.y = self.jump_force * dt
            self.is_grounded = False

    def on_collision_enter(self, other_entity):
        self.is_grounded = True
        pyengine.Debug.log(f"Player landed! Collided with entity {other_entity}")

    def on_destroy(self):
        pyengine.Debug.log("PlayerController destroyed")
