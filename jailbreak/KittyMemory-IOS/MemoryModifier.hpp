//
//  MemoryModifier.hpp
//
//
//  Created by MJ (Ruit) on 1/1/19.
//
//

#ifndef MemoryModifier_hpp
#define MemoryModifier_hpp

#include "KittyMemory.hpp"


class MemoryModifier {
private:
  const char *_binaryName;

  void * _address;
  size_t _size;

  uint8_t *_originalValue;
  uint8_t *_modValue;

public:
  MemoryModifier();

  // expects absolute address
  MemoryModifier(void *address, const void *modifier, size_t size);

  // expects binary name and relative address, if binary name is NULL then it'll consider the base executable
  MemoryModifier(const char *binName, void *address, const void *modifier, size_t size);
  

  ~MemoryModifier();

  // validate modifier
  bool isValid() const;

  // get mod size
  size_t get_size() const;

  // returns pointer to the target address
  void *get_Address() const;

  // final address pointer
  void *getAbsoluteAddress(void *addr) const;

  // restore to original value
  bool Restore();

  // apply modifications to target address
  bool Modify();

  // reset mod value/size
  bool setNewModifier(const void *modifier, size_t size);

 // returns current target address bytes as hex string
  std::string ToHexString();
};

#endif /* MemoryModifier_hpp */
