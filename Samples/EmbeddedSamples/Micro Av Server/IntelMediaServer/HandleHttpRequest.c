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
#include "HandleHttpRequest.h"
#include "ILibWebServer.h"

void UpnpPresentationRequest(void* upnptoken, struct packetheader *packet)
{
	/*
	 *	Requests on the MediaServer's default webserver directory will 
	 *	always result in a 404 the mapping of URIs to actual data (usually a local file)
	 *	is not done here.
	 *
	 *	It should be noted that a token is a webserver session. 
	 *	Bryan has noted that UpnpPresentationRequest() will be deprecated
	 *	in future Microstack models.
	 */
	ILibWebServer_Send_Raw((struct ILibWebServer_Session*)upnptoken,"HTTP/1.1 404 File Not Found\r\nContent-Length: 0\r\n\r\n",48,1,1);
}
