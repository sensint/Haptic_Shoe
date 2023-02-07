#include "material_lib.h"

#include <debug.h>
#include <tactile_audio.h>

namespace sensint {

MaterialLib::MaterialLib() {
  default_material_.grain_params.duration = 5000.f;
  // waveform generator
  default_material_.grain_params.raw_signal_params.amplitude = 1.f;
  default_material_.grain_params.raw_signal_params.frequency = 200;
  default_material_.grain_params.raw_signal_params.waveform = Waveform::kSine;
  // envelope
  default_material_.grain_params.envelope_params.attack = 0.f;
  default_material_.grain_params.envelope_params.decay = 0.f;
  default_material_.grain_params.envelope_params.sustain = 1.f;
  default_material_.grain_params.envelope_params.release = 0.f;

  materials_.push_back(default_material_);
}

MaterialLib::~MaterialLib() {}

bool MaterialLib::AddMaterial(const sensint::Material &material) {
  for (size_t i = 0; i < materials_.size(); i++) {
    if (materials_[i].id == material.id) {
      return false;
    }
    if (materials_[i].id > material.id) {
      materials_.insert(materials_.begin() + i, material);
      return true;
    }
    if (i == materials_.size() - 1) {
      materials_.push_back(material);
      return true;
    }
  }
  return false;
}

bool MaterialLib::UpdateMaterial(const sensint::Material &material) {
  for (size_t i = 0; i < materials_.size(); i++) {
    if (materials_[i].id == material.id) {
      materials_[i] = material;
      return true;
    }
  }
  return false;
}

bool MaterialLib::DeleteMaterial(const uint8_t id) {
  for (size_t i = 0; i < materials_.size(); i++) {
    if (materials_[i].id == id) {
      materials_.erase(materials_.begin() + i);
      return true;
    }
  }
  return false;
}

bool MaterialLib::MaterialExists(const uint8_t id) const {
  for (const auto &mat : materials_) {
    if (mat.id == id) {
      return true;
    }
  }
  return false;
}

int MaterialLib::GetMaterialIndexByID(const uint8_t id) const {
  for (size_t idx = 0; idx < materials_.size(); idx++) {
    if (id == materials_[idx].id) {
      return idx;
    }
  }
  return -1;
}

bool MaterialLib::GetMaterialByID(const uint8_t id, sensint::Material &destination) const {
  for (size_t idx = 0; idx < materials_.size(); idx++) {
    if (id == materials_[idx].id) {
      destination = materials_[idx];
      return true;
    }
  }
  return false;
}

const sensint::Material &MaterialLib::GetDefaultMaterial() const { return default_material_; }

void MaterialLib::Reset() {
  materials_.clear();
  materials_.push_back(default_material_);
}

#ifdef SENSINT_DEBUG
void MaterialLib::PrintLib() const {
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    Serial.println("\n======== Material Lib ========");
    for (const auto &material : materials_) {
      PrintMaterial(material);
    }
  }
}
#endif  // SENSINT_DEBUG

}  // namespace sensint
