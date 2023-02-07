#include "state_management.h"

#include <communication.h>
#include <helper.h>

namespace sensint {
namespace state_management {

using namespace sensint::communication;
using namespace sensint::debug;

void HandleStartAugmentationMsg(AugmentationState &state) {
  using namespace sensint::debug;
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "should start augmentation");
#endif  // SENSINT_DEBUG
  state.should_augment = true;
}

void HandleStopAugmentationMsg(AugmentationState &state) {
  using namespace sensint::debug;
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "should stop augmentation");
#endif  // SENSINT_DEBUG
  state.should_augment = false;
}

void HandleAddMaterialMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                          MaterialLib &material_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "add single material");
#endif  // SENSINT_DEBUG
  Material material;
  std::vector<std::string> mat_tokens{tokens.begin() + 3, tokens.end()};
  if (!ParseMaterial(mat_tokens, material)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Parsing the material information failed!");
#endif  // SENSINT_DEBUG
    return;
  }
  if (!material_lib.AddMaterial(material)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "A material with this ID already exists!");
#endif  // SENSINT_DEBUG
    return;
  }
#ifdef SENSINT_DEBUG
  material_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleUpdateMaterialMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             MaterialLib &material_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "update single material");
#endif  // SENSINT_DEBUG
  Material mat;
  std::vector<std::string> mat_tokens{tokens.begin() + 3, tokens.end()};
  if (!ParseMaterial(mat_tokens, mat)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Parsing the material information failed!");
#endif  // SENSINT_DEBUG
    return;
  }
  if (!material_lib.UpdateMaterial(mat)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "A material with this ID does not exist!");
#endif  // SENSINT_DEBUG
    return;
  }
#ifdef SENSINT_DEBUG
  material_lib.PrintLib();
#endif  // SENSINT_DEBUG
  if (mat.id == state.current_material->id) {
    material_lib.GetMaterialByID(mat.id, *state.current_material);
    state.should_reinitialize_material = true;
  }
}

void HandleDeleteMaterialMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             MaterialLib &material_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "delete single material");
#endif  // SENSINT_DEBUG
  auto mat_id = atoi(tokens[3].c_str());
  if (mat_id == state.current_material->id) {
    state.current_material = &material_lib.GetDefaultMaterial();
    state.should_reinitialize_material = true;
  }
  if (!material_lib.DeleteMaterial(mat_id)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "A material with this ID does not exist!");
#endif  // SENSINT_DEBUG
    return;
  }
#ifdef SENSINT_DEBUG
  material_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleAddMaterialListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                              MaterialLib &material_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "add list of materials");
#endif  // SENSINT_DEBUG
  std::vector<Material> materials;
  int num_materials = atoi(tokens[2].c_str());
  std::vector<std::string> material_tokens{tokens.begin() + 3, tokens.end()};
  if (!ParseMaterialList(material_tokens, num_materials, materials)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Parsing the list of material information failed!");
#endif  // SENSINT_DEBUG
    return;
  }
  //! Hack start
  // TODO: This overwrites the material library each time!
  material_lib.Reset();
  state.current_material = &material_lib.GetDefaultMaterial();
  //! Hack end
  state.should_reinitialize_material = true;
  for (const auto &material : materials) {
    if (!material_lib.AddMaterial(material)) {
#ifdef SENSINT_DEBUG
      Log("UpdateConfig", "A material with this ID already exists!");
#endif  // SENSINT_DEBUG
    }
  }
#ifdef SENSINT_DEBUG
  material_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleUpdateMaterialListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 MaterialLib &material_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "update list of materials");
#endif  // SENSINT_DEBUG
  // example string for two materials:
  std::vector<Material> materials;
  int num_materials = atoi(tokens[2].c_str());
  std::vector<std::string> material_tokens{tokens.begin() + 3, tokens.end()};
  if (!ParseMaterialList(material_tokens, num_materials, materials)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Parsing the list of material information failed!");
#endif  // SENSINT_DEBUG
    return;
  }
  for (auto &material : materials) {
    if (!material_lib.UpdateMaterial(material)) {
#ifdef SENSINT_DEBUG
      Log("UpdateConfig", "A material with this ID does not exist!");
#endif  // SENSINT_DEBUG
      continue;
    }
    if (state.current_material->id == material.id) {
      state.current_material = &material;
      state.should_reinitialize_material = true;
    }
  }
#ifdef SENSINT_DEBUG
  material_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleDeleteMaterialListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 MaterialLib &material_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "delete list of materials");
#endif  // SENSINT_DEBUG
  int num_materials = atoi(tokens[2].c_str());
  for (int i = 0; i < num_materials; i++) {
    auto id = atoi(tokens[i + 3].c_str());
    if (id == state.current_material->id) {
      state.current_material = &material_lib.GetDefaultMaterial();
      state.should_reinitialize_material = true;
    }
    if (!material_lib.DeleteMaterial(id)) {
#ifdef SENSINT_DEBUG
      Log("UpdateConfig", "A material with this ID does not exist!");
#endif  // SENSINT_DEBUG
    }
  }
#ifdef SENSINT_DEBUG
  material_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleDeleteAllMaterialsMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 MaterialLib &material_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "delete all materials");
#endif  // SENSINT_DEBUG
  material_lib.Reset();
  state.current_material = &material_lib.GetDefaultMaterial();
  state.should_reinitialize_material = true;
#ifdef SENSINT_DEBUG
  material_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleAddSequenceMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                          SequenceLib &sequence_lib) {
  auto id = atoi(tokens[3].c_str());
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "add grain sequence id: " + String(id));
#endif  // SENSINT_DEBUG
  if (sequence_lib.SequenceExists(id)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Sequence with this ID already exists!");
#endif  // SENSINT_DEBUG
    return;
  }
  int num_grains = atoi(tokens[2].c_str());
  if (num_grains == 0) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Sequence without grains!");
#endif  // SENSINT_DEBUG
    return;
  }
  GrainSequence sequence;
  std::vector<std::string> grain_tokens{tokens.begin() + 3, tokens.end()};
  if (!ParseGrainSequence(grain_tokens, num_grains, sequence)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Parsing the sequence information failed!");
#endif  // SENSINT_DEBUG
    return;
  }
  sequence_lib.AddSequence(sequence);
#ifdef SENSINT_DEBUG
  sequence_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleUpdateSequenceMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             SequenceLib &sequence_lib) {
  auto id = atoi(tokens[3].c_str());
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "update grain sequence id: " + String(id));
#endif  // SENSINT_DEBUG
  if (!sequence_lib.SequenceExists(id)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Sequence with this ID does not exist!");
#endif  // SENSINT_DEBUG
    return;
  }
  int num_grains = atoi(tokens[2].c_str());
  if (num_grains == 0) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Sequence without grains!");
#endif  // SENSINT_DEBUG
    return;
  }
  GrainSequence sequence;
  std::vector<std::string> grain_tokens{tokens.begin() + 3, tokens.end()};
  if (!ParseGrainSequence(grain_tokens, num_grains, sequence)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Parsing the sequence information failed!");
#endif  // SENSINT_DEBUG
    return;
  }
  sequence_lib.UpdateSequence(sequence);
#ifdef SENSINT_DEBUG
  sequence_lib.PrintLib();
#endif  // SENSINT_DEBUG
  if (sequence.id == state.current_sequence->id) {
    sequence_lib.GetSequenceByID(sequence.id, *state.current_sequence);
    state.should_reinitialize_material = true;
  }
}

void HandleDeleteSequenceMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             SequenceLib &sequence_lib) {
  auto id = atoi(tokens[3].c_str());
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "delete grain sequence id: " + String(id));
#endif  // SENSINT_DEBUG
  if (id == state.current_sequence->id) {
    state.current_sequence = &sequence_lib.GetDefaultSequence();
  }
  if (!sequence_lib.DeleteSequence(id)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "A sequence with this ID does not exist!");
#endif  // SENSINT_DEBUG
    return;
  }
#ifdef SENSINT_DEBUG
  sequence_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleAddSequenceListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                              SequenceLib &sequence_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "add list of grain sequences");
  Log("NOT IMPLEMENTED YET");
#endif  // SENSINT_DEBUG
        // TODO: implement
#ifdef SENSINT_DEBUG
  // sequence_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleUpdateSequenceListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 SequenceLib &sequence_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "update list of grain sequences");
  Log("NOT IMPLEMENTED YET");
#endif  // SENSINT_DEBUG
        // TODO: implement
#ifdef SENSINT_DEBUG
  // sequence_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleDeleteSequenceListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 SequenceLib &sequence_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "delete list of grain sequences");
  Log("NOT IMPLEMENTED YET");
#endif  // SENSINT_DEBUG
        // TODO: implement
#ifdef SENSINT_DEBUG
  // sequence_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleDeleteAllSequencesMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 SequenceLib &sequence_lib) {
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "delete all grain sequences");
#endif  // SENSINT_DEBUG
  sequence_lib.Reset();
  state.current_sequence = &sequence_lib.GetDefaultSequence();
#ifdef SENSINT_DEBUG
  sequence_lib.PrintLib();
#endif  // SENSINT_DEBUG
}

void HandleSelectSequenceMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             SequenceLib &sequence_lib) {
  auto id = atoi(tokens[3].c_str());
#ifdef SENSINT_DEBUG
  Log("UpdateConfig", "select grain sequence " + String(id));
#endif  // SENSINT_DEBUG
  if (!sequence_lib.SequenceExists(id)) {
#ifdef SENSINT_DEBUG
    Log("UpdateConfig", "Selected sequence ID does not exist!");
#endif  // SENSINT_DEBUG
    return;
  }
  sequence_lib.GetSequenceByID(id, *state.current_sequence);
  state.should_reinitialize_material = true;
#ifdef SENSINT_DEBUG
  Log("======== Active Sequence ========");
  PrintGrainSequence(*state.current_sequence);
#endif  // SENSINT_DEBUG
}

void UpdateConfig(const MessageTypes msg_type, const std::vector<std::string> &tokens,
                  AugmentationState &state, MaterialLib &material_lib, SequenceLib &sequence_lib) {
  switch (msg_type) {
    case MessageTypes::kStartAugmentation: {
      HandleStartAugmentationMsg(state);
      break;
    }
    case MessageTypes::kStopAugmentation: {
      HandleStopAugmentationMsg(state);
      break;
    }
    case MessageTypes::kAddMaterial: {
      HandleAddMaterialMsg(tokens, state, material_lib);
      break;
    }
    case MessageTypes::kUpdateMaterial: {
      HandleUpdateMaterialMsg(tokens, state, material_lib);
      break;
    }
    case MessageTypes::kDeleteMaterial: {
      HandleDeleteMaterialMsg(tokens, state, material_lib);
      break;
    }
    case MessageTypes::kAddMaterialList: {
      HandleAddMaterialListMsg(tokens, state, material_lib);
      break;
    }
    case MessageTypes::kUpdateMaterialList: {
      HandleUpdateMaterialListMsg(tokens, state, material_lib);
      break;
    }
    case MessageTypes::kDeleteMaterialList: {
      HandleDeleteMaterialListMsg(tokens, state, material_lib);
      break;
    }
    case MessageTypes::kDeleteAllMaterials: {
      HandleDeleteAllMaterialsMsg(tokens, state, material_lib);
      break;
    }
    case MessageTypes::kSelectGrainSequence: {
      HandleSelectSequenceMsg(tokens, state, sequence_lib);
      break;
    }
    case MessageTypes::kAddGrainSequence: {
      HandleAddSequenceMsg(tokens, state, sequence_lib);
      break;
    }
    case MessageTypes::kUpdateGrainSequence: {
      HandleUpdateSequenceMsg(tokens, state, sequence_lib);
      break;
    }
    case MessageTypes::kDeleteGrainSequence: {
      HandleDeleteSequenceMsg(tokens, state, sequence_lib);
      break;
    }
    case MessageTypes::kAddGrainSequenceList: {
      HandleAddSequenceListMsg(tokens, state, sequence_lib);
      break;
    }
    case MessageTypes::kUpdateGrainSequenceList: {
      HandleUpdateSequenceListMsg(tokens, state, sequence_lib);
      break;
    }
    case MessageTypes::kDeleteGrainSequenceList: {
      HandleDeleteSequenceListMsg(tokens, state, sequence_lib);
      break;
    }
    case MessageTypes::kDeleteAllGrainSequences: {
      HandleDeleteAllSequencesMsg(tokens, state, sequence_lib);
      break;
    }
    default: {
#ifdef SENSINT_DEBUG
      Log("UpdateConfig", "undefined message type");
#endif  // SENSINT_DEBUG
      break;
    }
  }
}

bool CheckAndStartContinuousVibration(const int grain_idx, AugmentationState &state,
                                      tactile_audio::MonoAudio &audio, MaterialLib &material_lib,
                                      const sensint::analog_sensor_t jitterThreshold) {
  using namespace sensint::tactile_audio;
  if (state.current_sensor_value < state.closest_grain->pos_start ||
      state.current_sensor_value > state.closest_grain->pos_end) {
    return false;
  }
  if (state.cv_was_triggered) {
    return false;
  }
  CheckAndApplyMaterialChange(state, audio, material_lib);
  audio.raw_signal.phase(0.0);
  audio.raw_signal.amplitude(state.current_material->grain_params.raw_signal_params.amplitude);
  audio.envelope.noteOn();
  audio.is_playing = true;
  state.last_grain = state.closest_grain;
  state.cv_was_triggered = true;
#ifdef SENSINT_DEBUG
  Log("CheckAndStartContinuousVibration",
      "entered continuous vibration - grain idx: " + String(grain_idx) +
          " pos_start:" + String(state.closest_grain->pos_start) +
          " pos_end:" + String(state.closest_grain->pos_end) +
          " | sensor:" + String((int)state.current_sensor_value) +
          " | mat:" + String((int)state.closest_grain->material_id),
      DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  return true;
}

bool CheckAndStopContinuousVibration(const int grain_idx, AugmentationState &state,
                                     tactile_audio::MonoAudio &audio) {
  if (state.cv_was_triggered && (state.current_sensor_value < state.last_grain->pos_start ||
                                 state.current_sensor_value > state.last_grain->pos_end)) {
    audio.envelope.noteOff();
    audio.raw_signal.amplitude(0.0);
    audio.is_playing = false;
    state.cv_was_triggered = false;
#ifdef SENSINT_DEBUG
    Log("CheckAndStopContinuousVibration",
        "left continuous vibration - grain idx: " + String(grain_idx) +
            " pos_start:" + String(state.closest_grain->pos_start) +
            " pos_end:" + String(state.closest_grain->pos_end) +
            " | sensor:" + String((int)state.current_sensor_value) +
            " | mat:" + String((int)state.closest_grain->material_id),
        DebugLevel::verbose);
#endif  // SENSINT_DEBUG
    return true;
  }
  return false;
}

bool CheckAndStartGrain(const int grain_idx, AugmentationState &state,
                        tactile_audio::MonoAudio &audio, MaterialLib &material_lib,
                        const sensint::analog_sensor_t jitterThreshold) {
  using namespace sensint::tactile_audio;
  // Once a grain has been triggered, the sensor needs to be moved/pressed/etc.
  // a bit before it allows new triggers (incl. retrigger).
  if (state.grain_was_triggered) {
    auto distance = abs(state.closest_grain->pos_start - state.current_sensor_value);
    if (distance < jitterThreshold) {
      state.is_jitter = true;
      return false;
    } else {
      state.grain_was_triggered = false;
      state.is_jitter = false;
    }
  }
  if (state.closest_grain->pos_start != state.current_sensor_value) {
    return false;
  }
  // A grain could have a very long duration. If the `allow_retrigger` flag is
  // set to true the grain will be stopped and the most recent starts.
  if (!state.allow_retrigger && audio.is_playing) {
    return false;
  }
  CheckAndApplyMaterialChange(state, audio, material_lib);
  audio.raw_signal.phase(0.0);
  audio.envelope.noteOn();
  audio.play_time = 0;
  audio.is_playing = true;
  state.last_grain = state.closest_grain;
  state.grain_was_triggered = true;
#ifdef SENSINT_DEBUG
  Log("CheckAndStartGrain",
      "trigger grain - idx:" + String(grain_idx) +
          " pos:" + String(state.closest_grain->pos_start) +
          " | sensor:" + String((int)state.current_sensor_value) +
          " | mat:" + String((int)state.closest_grain->material_id),
      DebugLevel::verbose);
#endif  // SENSINT_DEBUG
  return true;
}

bool CheckAndStopGrain(const int grain_idx, AugmentationState &state,
                       tactile_audio::MonoAudio &audio) {
  if (!state.current_material->grain_params.is_continuous && audio.is_playing &&
      audio.play_time >= state.current_material->grain_params.duration) {
    audio.envelope.noteOff();
    audio.is_playing = false;
    audio.play_time = 0;
#ifdef SENSINT_DEBUG
    Log("CheckAndStopGrain",
        "stop grain - idx:" + String(grain_idx) + " pos:" + String(state.closest_grain->pos_start) +
            " | sensor:" + String((int)state.current_sensor_value) +
            " | mat:" + String((int)state.closest_grain->material_id),
        DebugLevel::verbose);
#endif  // SENSINT_DEBUG
    return true;
  }
  return false;
}

bool CheckAndApplyMaterialChange(AugmentationState &state, tactile_audio::MonoAudio &audio,
                                 MaterialLib &material_lib) {
  if (state.should_reinitialize_material ||
      state.closest_grain->material_id != state.last_grain->material_id) {
    state.should_reinitialize_material = false;
    auto mat_idx = material_lib.GetMaterialIndexByID(state.closest_grain->material_id);
    if (mat_idx != -1) {
      material_lib.GetMaterialByID(mat_idx, *state.current_material);
#ifdef SENSINT_DEBUG
      Log("CheckAndApplyMaterialChange",
          "change to material " + String((int)state.current_material->id), DebugLevel::verbose);
#endif  // SENSINT_DEBUG
      ApplyGrainParameters(state.current_material->grain_params, audio);
      return true;
    }
  }
  return false;
}

}  // namespace state_management
}  // namespace sensint
