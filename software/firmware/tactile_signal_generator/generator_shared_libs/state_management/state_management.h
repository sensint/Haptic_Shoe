#ifndef __SENSINT_STATE_MANAGEMENT_H__
#define __SENSINT_STATE_MANAGEMENT_H__

#include <communication.h>
#include <material_lib.h>
#include <sequence_lib.h>
#include <tactile_audio.h>
#include <types.h>

#include <string>
#include <vector>

namespace sensint {

struct AugmentationState {
  // the currently selected grain sequence
  GrainSequence *current_sequence = nullptr;
  // the most recent sensor value
  analog_sensor_t current_sensor_value = 0;
  // the closest grain from the current sensor value
  Grain *closest_grain = nullptr;
  // the last grain that was triggered
  Grain *last_grain = nullptr;
  // the most recent material used to play a grain
  Material *current_material = nullptr;
  // If true a playing grain would be interrupted and a new one is started
  // immediately. If false the current grain will be played in full length which
  // might lead to "missed grains".
  bool allow_retrigger = true;
  // If the sensor is close to the grain position it might retrigger it
  // continuously due to sensor jitter. To prevent this we implemented a "safe
  // region" (i.e. threshold). "is_jitter" and "grain_was_triggered" are flags
  // used for establishing this filter procedure.
  bool is_jitter = false;
  bool grain_was_triggered = false;
  bool cv_was_triggered = false;
  bool should_update_config = false;
  bool should_send_sensor_data = false;
  bool should_augment = true;
  bool should_reinitialize_material = false;
};

namespace state_management {

/**
 * @brief Handle a message received from a controller device. This message is
 * used to start (activate) the augmentation. If no grain sequence was defined
 * before sending this message a default sequence will be used.
 *
 * @param state reference to the system's augmentation state
 */
void HandleStartAugmentationMsg(AugmentationState &state);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to stop (deactivate) the augmentation.
 *
 * @param state reference to the system's augmentation state
 */
void HandleStopAugmentationMsg(AugmentationState &state);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to add a single material to the local material library. If a material
 * with the same ID exists already in the material library it will not be added.
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param material_lib reference to the local material library
 */
void HandleAddMaterialMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                          MaterialLib &material_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to update a single material to the local material library. If the
 * specified material (ID) does not exist in the material library it will not be
 * modified (nor added).
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param material_lib reference to the local material library
 */
void HandleUpdateMaterialMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             MaterialLib &material_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to delete a single material to the local material library. If the
 * specified material (ID) does not exist in the material library it will not be
 * deleted.
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param material_lib reference to the local material library
 */
void HandleDeleteMaterialMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             MaterialLib &material_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to add a list of materials to the local material library. If a material
 * with the same ID exists already in the material library it will not be added.
 *
 * @param tokens tokenized message (i.e. parameters as strings)
 * @param state reference to the system's augmentation state
 * @param material_lib reference to the local material library
 */
void HandleAddMaterialListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                              MaterialLib &material_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to update a list of materials to the local material library. If one of
 * the materials with the specified ID does not exist in the material library it
 * will not be modified (nor added).
 *
 * @param tokens tokenized message (i.e. parameters as strings)
 * @param state reference to the system's augmentation state
 * @param material_lib reference to the local material library
 */
void HandleUpdateMaterialListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 MaterialLib &material_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to delete a list of materials from the local material library. If one of
 * the materials with the specified ID does not exist in the material library it
 * will not be deleted.
 *
 * @param tokens tokenized message (i.e. parameters as strings)
 * @param state reference to the system's augmentation state
 * @param material_lib reference to the local material library
 */
void HandleDeleteMaterialListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 MaterialLib &material_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to delete all materials from the local material library. Only the
 * default material will be accessible until a new material is added.
 *
 * @param tokens tokenized message (i.e. parameters as strings)
 * @param state reference to the system's augmentation state
 * @param material_lib reference to the local material library
 */
void HandleDeleteAllMaterialsMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 MaterialLib &material_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to add a single sequence to the local sequence library. If a Sequence
 * with the same ID does not exist in the sequence library it will not be
 * modified (nor added).
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param sequence_lib reference to the local sequence library
 */
void HandleAddSequenceMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                          SequenceLib &sequence_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to update a single sequence in the local sequence library. If a Sequence
 * with the same ID does not exist already in the sequence library it will not
 * be added.
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param sequence_lib reference to the local sequence library
 */
void HandleUpdateSequenceMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             SequenceLib &sequence_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to delete a single sequence from the local sequence library. If a
 * Sequence with the same ID does not exist in the sequence library it will not
 * be deleted.
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param sequence_lib reference to the local sequence library
 */
void HandleDeleteSequenceMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             SequenceLib &sequence_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to add a list of sequences to the local sequence library. If a Sequence
 * with the same ID exists already in the sequence library it will not be
 * added.
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param sequence_lib reference to the local sequence library
 */
void HandleAddSequenceListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                              SequenceLib &sequence_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to update a list of sequences in the local sequence library. If a
 * Sequence with the same ID does not exist in the sequence library it will not
 * be modified (nor added).
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param sequence_lib reference to the local sequence library
 */
void HandleUpdateSequenceListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 SequenceLib &sequence_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to delete a list of sequences from the local sequence library. If a
 * Sequence with the same ID does not exist in the sequence library it will not
 * be deleted.
 *
 * @param tokens parameters of the material
 * @param state reference to the system's augmentation state
 * @param sequence_lib reference to the local sequence library
 */
void HandleDeleteSequenceListMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 SequenceLib &sequence_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to delete all sequences from the local sequence library. Only the
 * default sequence will be accessible until a new sequence is added.
 *
 * @param tokens tokenized message (i.e. parameters as strings)
 * @param state reference to the system's augmentation state
 * @param sequence_lib reference to the local sequence library
 */
void HandleDeleteAllSequencesMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                                 SequenceLib &sequence_lib);

/**
 * @brief Handle a message received from a controller device. This message is
 * used to selected the active sequences from the local sequence library.
 *
 * @param tokens tokenized message (i.e. parameters as strings)
 * @param state reference to the system's augmentation state
 * @param sequence_lib reference to the local sequence library
 */
void HandleSelectSequenceMsg(const std::vector<std::string> &tokens, AugmentationState &state,
                             SequenceLib &sequence_lib);

/**
 * @brief Handle a message (string) received from a controller device. This
 * function forwards the tokenized message to the dedicated handler.
 *
 * @param msg_type the received type of message
 * @param tokens tokenized message (i.e. parameters as strings)
 * @param state reference to the system's augmentation state
 * @param material_lib reference to the local material library
 * @param sequence_lib reference to the local sequence library
 */
void UpdateConfig(const communication::MessageTypes msg_type,
                  const std::vector<std::string> &tokens, AugmentationState &state,
                  MaterialLib &material_lib, SequenceLib &sequence_lib);

/**
 * @brief Check if a continuous vibration should be started.
 *
 * @param grain_idx
 * @param state
 * @param audio
 * @param material_lib
 * @param jitterThreshold
 *
 * @return true if a cv has been started
 * @return false if no cv has been started
 */
bool CheckAndStartContinuousVibration(const int grain_idx, AugmentationState &state,
                                      tactile_audio::MonoAudio &audio, MaterialLib &material_lib,
                                      const sensint::analog_sensor_t jitterThreshold = 2);

/**
 * @brief Check if a continuous vibration should be stopped.
 *
 * @param grain_idx
 * @param state
 * @param audio
 *
 * @return true if a cv has been started
 * @return false if no cv has been started
 */
bool CheckAndStopContinuousVibration(const int grain_idx, AugmentationState &state,
                                     tactile_audio::MonoAudio &audio);

/**
 * @brief  Check if a new grain should be started.
 *
 * @param grain_idx
 * @param state
 * @param audio
 * @param material_lib
 * @param jitterThreshold
 *
 * @return true if a grain has been stopped
 * @return false if no grain has been stopped
 */
bool CheckAndStartGrain(const int grain_idx, AugmentationState &state,
                        tactile_audio::MonoAudio &audio, MaterialLib &material_lib,
                        const sensint::analog_sensor_t jitterThreshold = 2);

/**
 * @brief  Check if a new grain should be stopped.
 *
 * @param grain_idx
 * @param state
 * @param audio
 *
 * @return true if a grain has been stopped
 * @return false if no grain has been stopped
 */
bool CheckAndStopGrain(const int grain_idx, AugmentationState &state,
                       tactile_audio::MonoAudio &audio);

/**
 * @brief Check if a new material should be applied. If true -> apply the
 * material properties to the audio objects and update the augmentation state.
 *
 * @param state
 * @param audio
 * @param material_lib
 * @return true if a new material was applied
 * @return false if no material was applied
 */
bool CheckAndApplyMaterialChange(AugmentationState &state, tactile_audio::MonoAudio &audio,
                                 MaterialLib &material_lib);

}  // namespace state_management
}  // namespace sensint

#endif  // __SENSINT_STATE_MANAGEMENT_H__
