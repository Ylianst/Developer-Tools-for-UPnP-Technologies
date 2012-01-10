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

#include "CdsMediaClass.h"

/*
 *	The relative order of strings within these arrays must correspond to the MMSCP_CLASS_MASK_xxx bitmask mappings.
 *	The first element must always be an empty string, to account for the fact that a sub-bitstring of the 
 *	mediaClass value may evaluate to 0.
 *
 *	All of these strings represent the normative set of media classes defined in UPnP AV.
 *
 *	It should be noted that none of these strings should EVER need to be change for XML escaping.
 */
const char* CDS_CLASS_OBJECT_TYPE[] = {"", "object.item", "object.container"};
const char* CDS_CLASS_MAJOR_TYPE[] = {"", "imageItem", "audioItem", "videoItem", "playlistItem", "textItem", "person", "playlistContainer", "album", "genre", "storageSystem", "storageVolume", "storageFolder"};
const char* CDS_CLASS_MINOR1_TYPE[] = {"", "photo", "musicTrack", "audioBroadcast", "audioBook", "movie", "videoBroadcast", "musicVideClip", "musicArtist", "musicAlbum", "photoAlbum", "musicGenre", "movieGenre"};
const char* CDS_CLASS_MINOR2_TYPE[] = {""};
