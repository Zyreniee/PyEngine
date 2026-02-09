# PyEngine Roadmap

Future features and improvements planned for PyEngine.

## Iteration 2: Enhanced Rendering

### PBR Materials
- [ ] Physically Based Rendering pipeline
- [ ] Metallic-roughness workflow
- [ ] Normal mapping
- [ ] Emissive textures
- [ ] Material instancing

### Lighting System
- [ ] Point lights
- [ ] Spot lights
- [ ] Directional lights
- [ ] Shadow mapping (cascaded for directional)
- [ ] Light culling

### Post-Processing
- [ ] HDR rendering
- [ ] Tone mapping (ACES, Reinhard)
- [ ] Bloom
- [ ] SSAO (Screen Space Ambient Occlusion)
- [ ] FXAA/TAA (anti-aliasing)

---

## Iteration 3: Performance & Optimization

### Render Graph
- [ ] High-level rendering abstraction
- [ ] Automatic resource barriers
- [ ] Render pass merging
- [ ] GPU timeline visualization

### GPU-Driven Rendering
- [ ] Compute shader frustum culling
- [ ] Occlusion culling
- [ ] GPU instancing
- [ ] Indirect drawing
- [ ] meshlet rendering

### Multi-threading
- [ ] Parallel command buffer recording
- [ ] Job system for CPU tasks
- [ ] Background asset loading

---

## Iteration 4: Advanced Graphics

### Deferred Rendering
- [ ] G-buffer pass
- [ ] Light accumulation pass
- [ ] Deferred decals
- [ ] SSGI (Screen Space Global Illumination)

### Ray Tracing (Future/Optional)
- [ ] VK_KHR_ray_tracing support
- [ ] Hybrid ray-traced reflections
- [ ] Ray-traced shadows
- [ ] Path-traced global illumination

### Volumetric Effects
- [ ] Volumetric fog
- [ ] God rays
- [ ] Particle systems (GPU-driven)

---

## Iteration 5: Physics & Simulation

### Physics Integration
- [ ] Jolt Physics or PhysX integration
- [ ] Rigidbody dynamics
- [ ] Collision detection
- [ ] Character controller
- [ ] Ragdolls

### Cloth & Soft Bodies
- [ ] Cloth simulation
- [ ] Soft body physics
- [ ] GPU-accelerated simulation

---

## Iteration 6: Audio

### Audio System
- [ ] OpenAL or FMOD integration
- [ ] 3D spatial audio
- [ ] Audio asset management
- [ ] Music streaming
- [ ] Sound effects pooling

---

## Iteration 7: Scripting

### Scripting Language
- [ ] Lua integration
- [ ] Script hot-reloading
- [ ] C++ ↔ Lua bindings
- [ ] Scriptable components

### Visual Scripting (Future)
- [ ] Node-based scripting
- [ ] Blueprint-like system

---

## Iteration 8: Animation

### Skeletal Animation
- [ ] Bone hierarchy
- [ ] Animation blending
- [ ] State machines
- [ ] IK (Inverse Kinematics)

### Procedural Animation
- [ ] Physics-based animation
- [ ] Procedural locomotion

---

## Iteration 9: Editor

### In-Engine Editor
- [ ] Scene editor
- [ ] Entity inspector
- [ ] Asset browser
- [ ] Viewport gizmos (translate, rotate, scale)
- [ ] Play-in-editor mode

### Tooling
- [ ] Profiler (CPU/GPU)
- [ ] Memory debugger
- [ ] Render debugging (wireframe, normals, overdraw)
- [ ] Shader hot-reload

---

## Iteration 10: Ecosystem

### Asset Pipeline
- [ ] Automatic texture compression
- [ ] LOD generation
- [ ] Asset dependency graph
- [ ] Asset version control integration

### Networking (Future)
- [ ] Client-server architecture
- [ ] Entity replication
- [ ] Lag compensation

### VR Support (Future)
- [ ] OpenXR integration
- [ ] Stereoscopic rendering
- [ ] VR input handling

---

## Platform Support

### Additional Platforms
- [ ] Windows support
- [ ] macOS support (MoltenVK)
- [ ] Android support (mobile Vulkan)

---

## Community & Docs

- [ ] Tutorials and examples
- [ ] API documentation (Doxygen)
- [ ] Sample projects
- [ ] Discord community
- [ ] Video tutorials

---

## Long-Term Vision

Transform PyEngine into a **production-ready game engine** suitable for:
- Indie game development
- Technical demos
- Research and education
- Prototyping AAA features

**Target**: Feature parity with Unity/Unreal for mid-sized 3D projects.

---

## Contributing

Community contributions are encouraged! See issues tagged with `good-first-issue` or `help-wanted`.

## Versioning

- **v0.1.x**: Core rendering + ECS
- **v0.2.x**: PBR + lighting
- **v0.3.x**: Deferred rendering + optimizations
- **v0.4.x**: Physics integration
- **v0.5.x**: Editor tools
- **v1.0.0**: Production-ready release

---

**Status**: PyEngine v0.1.0 (Alpha) - Core rendering functional, architecture stable.
