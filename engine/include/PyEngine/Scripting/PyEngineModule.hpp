#pragma once

// Forward declare — the actual bindings are registered in PyEngineModule.cpp
// This header just provides the registration function signature.

namespace PyEngine {

// Called by PythonEngine::Initialize() to register the embedded 'pyengine' module
void RegisterPyEngineModule();

}  // namespace PyEngine
