#ifndef __SENSINT_MATERIAL_LIB_H__
#define __SENSINT_MATERIAL_LIB_H__

#include <types.h>

#include <string>
#include <vector>

namespace sensint {

class MaterialLib final {
 public:
  MaterialLib();
  ~MaterialLib();

  /**
   * @brief Add a material to the library. Materials are ordered by their IDs.
   * If a material with the same ID exists already the library remains as is.
   *
   * @param material the material object to add
   * @return true material was added
   * @return false material was not added
   */
  bool AddMaterial(const sensint::Material &material);

  /**
   * @brief Update the properties of an existing material. If the material does
   * not exist the library remains as is.
   *
   * @param material the material to update
   * @return true update was successful
   * @return false no material with this ID found
   */
  bool UpdateMaterial(const sensint::Material &material);

  /**
   * @brief Delete a material from the library.
   *
   * @param id of the material to delete
   * @return true delete was successful
   * @return false no material with this ID found
   */
  bool DeleteMaterial(const uint8_t id);

  /**
   * @brief Check if a material with a given ID exists in the library.
   *
   * @param material_id id of the material
   * @return true material exists
   * @return false material does not exist
   */
  bool MaterialExists(const uint8_t id) const;

  /**
   * @brief Find a material in a vector based on its unique identifier
   *
   * @param materials vector of materials
   * @param material_id unique identifier
   * @return int the index of the vector where the material is placed. Returns
   * -1 if the material does not exist.
   */
  int GetMaterialIndexByID(const uint8_t id) const;

  /**
   * @brief Get a material by its ID.
   *
   * @param id
   * @param destination reference of the placeholder for the material
   * @return true if material exists
   * @return false if material does not exist
   */
  bool GetMaterialByID(uint8_t id, sensint::Material &destination) const;

  /**
   * @brief Get a material by its ID.
   * @return the default material
   */
  const sensint::Material &GetDefaultMaterial() const;

  /**
   * @brief Reset the entire library, i.e. all materials are removed but the
   * default material remains in the library.
   */
  void Reset();

#ifdef SENSINT_DEBUG
  /**
   * @brief Print all materials in the library to the serial port.
   */
  void PrintLib() const;
#endif  // SENSINT_DEBUG

 private:
  // there is a default material which is used when the controller is booted
  sensint::Material default_material_;
  // the library data
  std::vector<sensint::Material> materials_;
};

}  // namespace sensint

#endif  // __SENSINT_MATERIAL_LIB_H__