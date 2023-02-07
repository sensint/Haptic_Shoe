#ifndef __SENSINT_SEQUENCE_LIB_H__
#define __SENSINT_SEQUENCE_LIB_H__

#include <types.h>

#include <string>
#include <vector>

namespace sensint {

class SequenceLib final {
 public:
  SequenceLib();
  ~SequenceLib();

  /**
   * @brief Add a sequence to the library. Sequences are ordered by their IDs.
   * If a sequence with the same ID exists already the library remains as is.
   *
   * @param sequence the sequence object to add
   * @return true sequence was added
   * @return false sequence was not added
   */
  bool AddSequence(const sensint::GrainSequence &sequence);

  /**
   * @brief Update the properties of an existing sequence. If the sequence does
   * not exist the library remains as is.
   *
   * @param sequence the sequence to update
   * @return true update was successful
   * @return false no sequence with this ID found
   */
  bool UpdateSequence(const sensint::GrainSequence &sequence);

  /**
   * @brief Delete a sequence from the library.
   *
   * @param id of the sequence to delete
   * @return true delete was successful
   * @return false no sequence with this ID found
   */
  bool DeleteSequence(const uint8_t id);

  /**
   * @brief Check if a sequence with a given ID exists in the library.
   *
   * @param sequence_id id of the sequence
   * @return true sequence exists
   * @return false sequence does not exist
   */
  bool SequenceExists(const uint8_t id) const;

  /**
   * @brief Find a sequence in a vector based on its unique identifier
   *
   * @param sequences vector of sequences
   * @param sequence_id unique identifier
   * @return int the index of the vector where the sequence is placed. Returns
   * -1 if the sequence does not exist.
   */
  int GetSequenceIndexByID(const uint8_t id) const;

  /**
   * @brief Get a sequence by its ID.
   *
   * @param id
   * @param destination reference of the placeholder for the sequence
   * @return true if sequence exists
   * @return false if sequence does not exist
   */
  bool GetSequenceByID(uint8_t id, sensint::GrainSequence &destination) const;

  /**
   * @brief Get a sequence by its ID.
   * @return the default sequence
   */
  const sensint::GrainSequence &GetDefaultSequence() const;

  /**
   * @brief Reset the entire library, i.e. all sequences are removed but the
   * default sequence remains in the library.
   */
  void Reset();

#ifdef SENSINT_DEBUG
  /**
   * @brief Print all sequences in the library to the serial port.
   */
  void PrintLib() const;
#endif  // SENSINT_DEBUG

 private:
  // there is a default sequence which is used when the controller is booted
  sensint::GrainSequence default_sequence_;
  // the library data
  std::vector<sensint::GrainSequence> sequences_;
};

}  // namespace sensint

#endif  // __SENSINT_SEQUENCE_LIB_H__