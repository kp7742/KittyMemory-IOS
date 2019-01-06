#import <UIKit/UIKit.h>
#import "KittyMemory-IOS/MemoryModifier.hpp"



%hook UnityAppController
-(void)applicationDidBecomeActive:(id)argument {

  NSLog(@"=========== I'M LOADED ============");
  

  // ==================================== 
  // Modify Health Example       
  
   uintptr_t playerBaseAddr = MemKitty::getBaseInfo().address + 0xPlayerAddress;
   uintptr_t playerPointer;
   // read player pointer
   if(readMemory(playerBaseAddr, &playerPointer, sizeof(uintptr_t))){
       void *playerHealthPointer = (void *)(playerPointer + 0xHealthOffset);
       int hackedPlayerHealth = 1337;
       MemoryModifier playerHealthMod(playerHealthPointer, &hackedPlayerHealth, sizeof(int));
       NSLog(@"playerHealth address: %p", playerHealthMod.get_Address());
      if(playerHealthMod.Modify()){
         NSLog(@"playerHealth has been modified successfully");
         NSLog(@"current playerHealth: %d", playerHealth);
      }

      int newHackedPlayerHealth = 999999;
      if(playerHealthMod.setNewModifier(&newHackedPlayerHealth, sizeof(int)) && playerHealthMod.Modify()){
         NSLog(@"playerHealth has been modified with new modifier successfully");
         NSLog(@"current playerHealth: %d", playerHealth);
      }

      if(playerHealthMod.Restore()){
         NSLog(@"playerHealth has been restored successfully");
         NSLog(@"current playerHealth: %d", playerHealth);
      }
   }
   
   

 // =============================================================================

 // Direct read & write, Camera Angles Example

   uintptr_t cameraBaseAddr = MemKitty::getBaseInfo().address + 0xCameraAddress;
   uintptr_t cameraPointer;
   // read camera pointer
   if(readMemory(cameraBaseAddr, &cameraPointer, sizeof(uintptr_t))){
   // read angles
      Vector2 currentCameraAngles;
   if(readMemory((void *)(cameraPointer + 0xAnglesOffset), &currentCameraAngles, sizeof(Vector2))){
       NSLog("CameraAngles(%f, %f)", currentCameraAngles.x, currentCameraAngles.y);
   }
   // write to angles
   Vector2 aimBotAngles = getAimbotAngles();
   if(writeMemory((void *)(cameraPointer + 0xAnglesOffset), &aimBotAngles, sizeof(Vector2))){
       NSLog("CameraAngles(%f, %f)", aimBotAngles.x, aimBotAngles.y);
    }
  }
%orig;

}
%end
