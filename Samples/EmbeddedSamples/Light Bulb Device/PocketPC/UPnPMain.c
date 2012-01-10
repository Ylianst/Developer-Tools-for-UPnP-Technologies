/*   
Copyright 2006 - 2011 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdio.h>
#include "UPnPMicroStack.h"

void SwitchPower_GetStatus(void* upnptoken)
{
  printf("UPnP Invoke: SwitchPower_GetStatus();\r\n");

  /* TODO: Place Action Code Here... */

  /* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UPnPResponse_SwitchPower_GetStatus(upnptoken,1);
}

void SwitchPower_SetTarget(void* upnptoken,int newTargetValue)
{
  printf("UPnP Invoke: SwitchPower_SetTarget(%d);\r\n",newTargetValue);

  /* TODO: Place Action Code Here... */

  /* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UPnPResponse_SwitchPower_SetTarget(upnptoken);
}

void DimmingService_GetLoadLevelStatus(void* upnptoken)
{
  printf("UPnP Invoke: DimmingService_GetLoadLevelStatus();\r\n");

  /* TODO: Place Action Code Here... */

  /* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UPnPResponse_DimmingService_GetLoadLevelStatus(upnptoken,250);
}

void DimmingService_GetMinLevel(void* upnptoken)
{
  printf("UPnP Invoke: DimmingService_GetMinLevel();\r\n");

  /* TODO: Place Action Code Here... */

  /* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UPnPResponse_DimmingService_GetMinLevel(upnptoken,250);
}

void DimmingService_SetLoadLevelTarget(void* upnptoken,unsigned char NewLoadLevelTarget)
{
  printf("UPnP Invoke: DimmingService_SetLoadLevelTarget(%d);\r\n",NewLoadLevelTarget);

  /* TODO: Place Action Code Here... */

  /* UPnPResponse_Error(upnptoken,404,"Method Not Implemented"); */
  UPnPResponse_DimmingService_SetLoadLevelTarget(upnptoken);
}

int main(void)
{
  void* UPnPFM_SwitchPower[] = {&SwitchPower_GetStatus,&SwitchPower_SetTarget};
  void* UPnPFM_DimmingService[] = {&DimmingService_GetLoadLevelStatus,&DimmingService_GetMinLevel,&DimmingService_SetLoadLevelTarget};
  UPnPSFP_SwitchPower(UPnPFM_SwitchPower);
  UPnPSFP_DimmingService(UPnPFM_DimmingService);

printf("Intel's UPnP MicroStack 1.0\r\nConnected & Extended PC Lab (CEL)\r\n\r\n");
  UPnPStart("fb852742-474d-431d-b1ad-628c92c533b2","0000001", 120);

  return 0;
}

