#include "sequence_lib.h"

#include <debug.h>
#include <tactile_audio.h>

namespace sensint {

SequenceLib::SequenceLib() {
  auto num_grains = 20;
  auto sensor_lim_l = 100;
  auto sensor_lim_u = 900;
  auto sensor_range = sensor_lim_u - sensor_lim_l;
  auto sensor_steps = (analog_sensor_t)(sensor_range / num_grains);
  auto half_step = (analog_sensor_t)(sensor_steps >> 1);
  for (auto idx = 0; idx < num_grains; idx++) {
    auto pos_start = static_cast<analog_sensor_t>((idx * sensor_steps) + sensor_lim_l + half_step);
    auto pos_end = pos_start;
    Grain grain{0x00, pos_start, pos_end};
    default_sequence_.grains.push_back(grain);
  }
  sequences_.push_back(default_sequence_);
}

SequenceLib::~SequenceLib() {}

bool SequenceLib::AddSequence(const sensint::GrainSequence &sequence) {
  for (size_t i = 0; i < sequences_.size(); i++) {
    if (sequences_[i].id == sequence.id) {
      return false;
    }
    if (sequences_[i].id > sequence.id) {
      sequences_.insert(sequences_.begin() + i, sequence);
      return true;
    }
    if (i == sequences_.size() - 1) {
      sequences_.push_back(sequence);
      return true;
    }
  }
  return false;
}

bool SequenceLib::UpdateSequence(const sensint::GrainSequence &sequence) {
  for (size_t i = 0; i < sequences_.size(); i++) {
    if (sequences_[i].id == sequence.id) {
      sequences_[i] = sequence;
      return true;
    }
  }
  return false;
}

bool SequenceLib::DeleteSequence(const uint8_t id) {
  for (size_t i = 0; i < sequences_.size(); i++) {
    if (sequences_[i].id == id) {
      sequences_.erase(sequences_.begin() + i);
      return true;
    }
  }
  return false;
}

bool SequenceLib::SequenceExists(const uint8_t id) const {
  for (const auto &sequence : sequences_) {
    if (sequence.id == id) {
      return true;
    }
  }
  return false;
}

int SequenceLib::GetSequenceIndexByID(const uint8_t id) const {
  for (size_t idx = 0; idx < sequences_.size(); idx++) {
    if (id == sequences_[idx].id) {
      return idx;
    }
  }
  return -1;
}

bool SequenceLib::GetSequenceByID(const uint8_t id, sensint::GrainSequence &destination) const {
  for (size_t idx = 0; idx < sequences_.size(); idx++) {
    if (id == sequences_[idx].id) {
      destination = sequences_[idx];
      return true;
    }
  }
  return false;
}

const sensint::GrainSequence &SequenceLib::GetDefaultSequence() const { return default_sequence_; }

void SequenceLib::Reset() {
  sequences_.clear();
  sequences_.push_back(default_sequence_);
}

#ifdef SENSINT_DEBUG
void SequenceLib::PrintLib() const {
  if (debug::kDebugLevel == debug::DebugLevel::verbose) {
    Serial.println("\n======== Sequence Lib ========");
    for (const auto &sequence : sequences_) {
      PrintGrainSequence(sequence);
    }
  }
}
#endif  // SENSINT_DEBUG

}  // namespace sensint
