//
//  MemoryModifier.cpp
//
//
//  Created by MJ (Ruit) on 1/1/19.
//
//

#include "MemoryModifier.hpp"


MemoryModifier::MemoryModifier(){
  _binaryName    = NULL;
  _address       = NULL;
  _size          = 0;
  _originalValue = NULL;
  _modValue      = NULL;
}

MemoryModifier::MemoryModifier(void *address, const void *modifier, size_t size){
    MemoryModifier();

    if(address && modifier && size > 0){

      _address = address;
      _size    = size;

      _originalValue = new uint8_t[size];
      _modValue      = new uint8_t[size];

    // backups
      MemKitty::readMemory(_address, _originalValue, size);
      MemKitty::readMemory((void *)modifier, _modValue, size);
    }
  }

MemoryModifier::MemoryModifier(const char *binName, void *address, const void *modifier, size_t size){
    MemoryModifier();

    if(address && modifier && size > 0){

      _binaryName = binName;

      _address = getAbsoluteAddress(address);
      _size    = size;

      _originalValue = new uint8_t[size];
      _modValue      = new uint8_t[size];

    // backups
      MemKitty::readMemory(_address, _originalValue, size);
      MemKitty::readMemory((void *)modifier, _modValue, size);
    }
  }

   MemoryModifier::~MemoryModifier(){
     // clean up
     if(_originalValue){
       delete[] _originalValue;
       _originalValue = NULL;
     }

     if(_modValue){
       delete[] _modValue;
      _modValue = NULL;
     }
   }

  bool MemoryModifier::isValid() const{
    return (_address && _size > 0 && _originalValue && _modValue);
  }

  size_t MemoryModifier::get_size() const{
    return _size;
  }

  void *MemoryModifier::get_Address() const{
    return _address;
  }

  void *MemoryModifier::getAbsoluteAddress(void *addr) const{
    if(_binaryName == NULL){
      return MemKitty::hasASLR() ? (void *)(MemKitty::getBaseInfo().address + ((uintptr_t)addr)) : addr;
    }

    return (void *)(MemKitty::getMemoryMachInfo(_binaryName).address + ((uintptr_t)addr));
  }

  bool MemoryModifier::Restore(){
    if(!isValid()) return false;
    return MemKitty::writeMemory(_address, _originalValue, _size);
  }

  bool MemoryModifier::Modify(){
    if(!isValid()) return false;
    return MemKitty::writeMemory(_address, _modValue, _size);
  }

  bool MemoryModifier::setNewModifier(const void *modifier, size_t size){
    if(!modifier || size < 1) return false;

    _size = size;

    if(_modValue){
      delete[] _modValue;
      _modValue = NULL;
    }

    _modValue = new uint8_t[size];
    return MemKitty::writeMemory(_modValue, modifier, size);
  }

  std::string MemoryModifier::ToHexString(){
    if(!isValid()) return std::string("0xInvalid");
    return MemKitty::read2HexStr(_address, _size);
  }
