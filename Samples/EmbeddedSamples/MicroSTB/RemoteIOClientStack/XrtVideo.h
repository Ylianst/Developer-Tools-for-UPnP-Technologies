#ifndef XRT_VIDEO_H
#define XRT_VIDEO_H

void XrtVideo_ConnectionChangedSink(char* PeerConnection);
void XrtVideo_ResetSink();
void XrtVideo_CommandSink(unsigned short command, char* data, int datalength);

#endif
