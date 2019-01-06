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


bool MemKitty::x_protect(void *addr, size_t length, int protection) {
   void     *pageStart = (void *)_START_PAGE_OF_(addr);
   uintptr_t pageSize  = _SIZE_PAGE_OF_(addr, length);

   return (
     mprotect(pageStart, pageSize, protection) != -1
 );
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

/* refs to:
https://github.com/comex/substitute/blob/master/lib/darwin/execmem.
https://github.com/everettjf/AppleTrace/blob/master/hookzz/src/zzdeps/darwin/memory-utils-darwin.c
*/
void *MemKitty::writeWrapper(void *dst, const void *src, size_t len) {

   uintptr_t startPage  = _START_PAGE_OF_(dst);
   uintptr_t offsetPage = _OFFSET_PAGE_OF_(dst);
   uintptr_t pageSize   = _SIZE_PAGE_OF_(dst, len);

   vm_region_submap_short_info_64 info;
   if(getPageInfo(startPage, &info) != KERN_SUCCESS)
       return NULL;

  // check if write permission is already there
   if(info.protection & PROT_WRITE)
       return MemCopy(dst, src, len);

   // map new page for our changes
   void *newMap = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
   if (newMap == NULL)
        return NULL;

   mach_port_t selfTask = mach_task_self();
   kern_return_t kr;

 // copy dst page into our new mapped page
   kr = vm_copy(selfTask, startPage, pageSize, (vm_address_t)newMap);
   if (kr != KERN_SUCCESS)
       return NULL;

   // write changes into our page
   MemCopy((void *)((uintptr_t)newMap + offsetPage), src, len);

   // change protection to match dst protection
   if(mprotect(newMap, pageSize, info.protection) == -1)
       return NULL;

  // re-map our page and overwrite dst
   mach_vm_address_t dstTarget = (mach_vm_address_t)startPage;
   vm_prot_t c, m;
   kr = mach_vm_remap(selfTask, &dstTarget, pageSize, 0, VM_FLAGS_OVERWRITE,
     selfTask, (mach_vm_address_t)newMap, /*copy*/ TRUE, &c, &m, info.inheritance);

   if (kr != KERN_SUCCESS)
       return NULL;

   // clean up
    munmap(newMap, pageSize);

    return dst;
}

// write in memory at given address (copies src into dst)
void *MemKitty::writeMemory(void *dst, const void *src, size_t len) {
  if (!src || !dst || len < 1)
      return NULL;

  void *address = (void *)_FIXED_ADDR_(dst);
  return writeWrapper(address, src, len);
}

// reads from memory at given address (copies dst bytes into the buffer)
void *MemKitty::readMemory(void *dst, void *buffer, size_t len) {
  if (!dst || len < 1)
      return NULL;

  void *address = (void *)_FIXED_ADDR_(dst);

  vm_region_submap_short_info_64 info;
  if(getPageInfo(_START_PAGE_OF_(address), &info) != KERN_SUCCESS)
     return NULL;

  // check read permission
  if(info.protection & PROT_READ)
     return MemCopy(buffer, address, len);

  if(x_protect(address, len, info.protection | PROT_READ)){
     void *ret = MemCopy(buffer, address, len);
     x_protect(address, len, info.protection);
     return ret;
  }

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
