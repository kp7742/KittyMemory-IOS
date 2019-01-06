//
//  KittyMemory.cpp
//
//
//  Created by MJ (Ruit) on 1/1/19.
//
//

#include "KittyMemory.hpp"

bool MemKitty::hasASLR() {
 // PIE applies ASLR to the binary
   return (_dyld_get_image_header(0)->flags & MH_PIE);
}


 kern_return_t MemKitty::getPageInfo(uintptr_t addr, vm_region_submap_short_info_64 *outInfo) {
   vm_address_t region  = (vm_address_t) addr;
   vm_size_t region_len = 0;
   mach_msg_type_number_t info_count = VM_REGION_SUBMAP_SHORT_INFO_COUNT_64;
   natural_t max_depth  = 99999;
   kern_return_t kr = vm_region_recurse_64(mach_task_self(), &region, &region_len,
                                           &max_depth,
                                           (vm_region_recurse_info_t) outInfo,
                                           &info_count);
   if(kr == KERN_SUCCESS)
      outInfo->protection &= (PROT_READ | PROT_WRITE | PROT_EXEC);

   return kr;
}


void *MemKitty::MemCopy(void* dst, const void* src, size_t size){
    uint8_t *destination = (uint8_t*) dst;
    uint8_t *source      = (uint8_t*) src;

    int step = sizeof(uint8_t);

    for(int i = 0; i < size; i++){
        *((uint8_t*)destination) = *((uint8_t*)source);
        destination += step;
        source      += step;
    }
    return dst;
}


// write in memory at given address (copies src into dst)
void *MemKitty::writeMemory(void *dst, const void *src, size_t len) {
  if (!src || !dst || len < 1)
      return NULL;

  void     *address   = (void *)_FIXED_ADDR_(dst);
  uintptr_t startPage = _START_PAGE_OF_(address);

  vm_region_submap_short_info_64 info;
  if(getPageInfo(startPage, &info) != KERN_SUCCESS)
     return NULL;

  // check write rights
   if(info.protection & PROT_WRITE)
      return MemCopy(dst, src, len);
   
  return NULL;
}

// reads from memory at given address (copies dst into the buffer)
void *MemKitty::readMemory(void *dst, void *buffer, size_t len) {
  if (!dst || len < 1)
      return NULL;

  void     *address    = (void *)_FIXED_ADDR_(dst);
  uintptr_t startPage  = _START_PAGE_OF_(address);

  vm_region_submap_short_info_64 info;
  if(getPageInfo(_START_PAGE_OF_(address), &info) != KERN_SUCCESS)
     return NULL;

  // check read rights
  if(info.protection & PROT_READ)
     return MemCopy(buffer, address, len);

  return NULL;
}


// reads bytes into hex string at the given address
std::string MemKitty::read2HexStr(void *addr, size_t len) {
  char tmp[len];
  memset(tmp, 0, len);

  char buffer[len*2];
  memset(buffer, 0, len*2);

  std::string ret  = "0x";

  if(readMemory(addr, tmp, len) == NULL)
    return ret;

  for(int i = 0; i < len; i++){
    sprintf(&buffer[i*2], "%02X", (unsigned char)tmp[i]);
  }

  ret += buffer;
  return ret;
}


MemKitty::mach_info MemKitty::getBaseInfo(){
 mach_info _info = {
   0,
   _dyld_get_image_header(0),
   _dyld_get_image_name(0),
   (uintptr_t)_dyld_get_image_vmaddr_slide(0)
 };
 return _info;
}



MemKitty::mach_info MemKitty::getMemoryMachInfo(const char *fileName){
  mach_info _info = {};

  int imageCount = _dyld_image_count();

   for(int i = 0; i < imageCount; i++) {
     const char *name = _dyld_get_image_name(i);
     const mach_header *header = _dyld_get_image_header(i);
       if(!strstr(name, fileName)) continue;

       mach_info new_info = {
         i, header, name, (uintptr_t)_dyld_get_image_vmaddr_slide(i)
       };

       _info = new_info;
     }
     return _info;
   }
