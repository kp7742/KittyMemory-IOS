//
//  KittyMemory.hpp
//
//
//  Created by MJ (Ruit) on 1/1/19.
//
//

#ifndef KittyMemory_hpp
#define KittyMemory_hpp

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/mman.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include "darwin/mach_vm.h"


#define _FIXED_ADDR_(x) ((uintptr_t)x & ~(uintptr_t)1)

#define _SYS_PAGE_SIZE_ (sysconf(_SC_PAGE_SIZE))

#define _START_PAGE_OF_(x) ((uintptr_t)x & ~(uintptr_t)(_SYS_PAGE_SIZE_ - 1))

#define _END_PAGE_OF_(x, len) (_START_PAGE_OF_((uintptr_t)x + len - 1))

#define _OFFSET_PAGE_OF_(x) ((uintptr_t)x - _START_PAGE_OF_(x));

#define _SIZE_PAGE_OF_(x, len)(_END_PAGE_OF_(x, len) - _START_PAGE_OF_(x) + _SYS_PAGE_SIZE_)


namespace MemKitty {

    typedef struct {
        int index;
        const mach_header *header;
        const char *name;
        uintptr_t address;
    } mach_info;


     bool hasASLR();
     bool x_protect(void *addr, size_t length, int protection);
     kern_return_t getPageInfo(uintptr_t addr, vm_region_submap_short_info_64 *outInfo);
     void *MemCopy(void *dst, const void *src, size_t len);
     void *writeWrapper(void *dst, const void *src, size_t len);
     void *writeMemory(void *dst, const void *src, size_t len);
     void *readMemory(void *src, void *buffer, size_t len);
     std::string read2HexStr(void *addr, size_t len);
     mach_info getBaseInfo();
     mach_info getMemoryMachInfo(const char *fileName);
};

#endif /* KittyMemory_hpp */
