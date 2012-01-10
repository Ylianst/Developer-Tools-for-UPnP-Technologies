#ifndef _CDS_STRINGS_H
#define _CDS_STRINGS_H

/*
 *	Number of chars needed to represent a 32 bit integer, including sign and null char.
 */
#define SIZE_INT32_AS_CHARS				12

/*
 *	Defines a number of well-defined CDS related strings
 *	for DIDL-Lite as well as UPnP argument names.
 */

#define CDS_STRING_URN_CDS				"urn:schemas-upnp-org:service:ContentDirectory:1"
#define CDS_STRING_BROWSE				"Browse"
#define CDS_STRING_RESULT				"Result"
#define CDS_STRING_NUMBER_RETURNED		"NumberReturned"
#define CDS_STRING_TOTAL_MATCHES		"TotalMatches"
#define CDS_STRING_UPDATE_ID			"UpdateID"

#define CDS_ROOT_CONTAINER_ID			"0"

#define CDS_STRING_BROWSE_DIRECT_CHILDREN		"BrowseDirectChildren"
#define CDS_STRING_BROWSE_METADATA				"BrowseMetadata"

#define CDS_DIDL_TITLE							"\r\n<dc:title>%s</dc:title>"
#define CDS_DIDL_TITLE_LEN						23

#define CDS_DIDL_CREATOR						"\r\n<dc:creator>%s</dc:creator>"
#define CDS_DIDL_CREATOR_LEN					27

#define CDS_DIDL_CLASS							"\r\n<upnp:class>%s</upnp:class>"
#define CDS_DIDL_CLASS_LEN						27

#define CDS_DIDL_ITEM_START						"\r\n<item id=\"%s\" parentID=\"%s\" restricted=\"%d\">"
#define CDS_DIDL_ITEM_START_LEN					40

#define CDS_DIDL_REFITEM_START					"\r\n<item id=\"%s\" parentID=\"%s\" restricted=\"%d\" refID=\"%s\">"
#define CDS_DIDL_REFITEM_START_LEN				49

#define CDS_DIDL_ITEM_END						"\r\n</item>"
#define CDS_DIDL_ITEM_END_LEN					9

#define CDS_DIDL_CONTAINER_START				"\r\n<container id=\"%s\" parentID=\"%s\" restricted=\"%d\" searchable=\"%d\">"
#define CDS_DIDL_CONTAINER_START_LEN			59

#define CDS_DIDL_CONTAINER_END					"\r\n</container>"
#define CDS_DIDL_CONTAINER_END_LEN				16

#define CDS_DIDL_RES_START						"\r\n<res protocolInfo=\"%s\""
#define CDS_DIDL_RES_START_LEN					22

#define CDS_DIDL_ATTRIB_RESOLUTION				" resolution=\"%dx%d\""
#define CDS_DIDL_ATTRIB_RESOLUTION_LEN			15

#define CDS_DIDL_ATTRIB_DURATION				" duration=\"%s\""
#define CDS_DIDL_ATTRIB_DURATION_LEN			12

#define CDS_DIDL_ATTRIB_BITRATE					" bitrate=\"%d\""
#define CDS_DIDL_ATTRIB_BITRATE_LEN				11

#define CDS_DIDL_ATTRIB_COLORDEPTH				" colorDepth=\"%d\""
#define CDS_DIDL_ATTRIB_COLORDEPTH_LEN			14

#define CDS_DIDL_ATTRIB_SIZE					" size=\"%d\""
#define CDS_DIDL_ATTRIB_SIZE_LEN				8

#define CDS_DIDL_RES_VALUE						">%s</res>"
#define CDS_DIDL_RES_VALUE_LEN					7


#define CDS_DIDL_HEADER_ESCAPED					"&lt;DIDL-Lite xmlns=&quot;urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/&quot; xmlns:dc=&quot;http://purl.org/dc/elements/1.1/&quot; xmlns:upnp=&quot;urn:schemas-upnp-org:metadata-1-0/upnp/&quot;&gt;"
#define CDS_DIDL_HEADER_ESCAPED_LEN				197

#define CDS_DIDL_FOOTER_ESCAPED					"\r\n&lt;/DIDL-Lite&gt;"
#define CDS_DIDL_FOOTER_ESCAPED_LEN				20

#define CDS_DIDL_TITLE_ESCAPED					"\r\n&lt;dc:title&gt;%s&lt;/dc:title&gt;"
#define CDS_DIDL_TITLE_ESCAPED_LEN				35

#define CDS_DIDL_TITLE1_ESCAPED					"\r\n&lt;dc:title&gt;"
#define CDS_DIDL_TITLE2_ESCAPED					"&lt;/dc:title&gt;"

#define CDS_DIDL_CREATOR_ESCAPED				"\r\n&lt;dc:creator&gt;%s&lt;/dc:creator&gt;"
#define CDS_DIDL_CREATOR_ESCAPED_LEN			39

#define CDS_DIDL_CREATOR1_ESCAPED				"\r\n&lt;dc:creator&gt;"
#define CDS_DIDL_CREATOR2_ESCAPED				"&lt;/dc:creator&gt;"

#define CDS_DIDL_CLASS_ESCAPED					"\r\n&lt;upnp:class&gt;%s&lt;/upnp:class&gt;"
#define CDS_DIDL_CLASS_ESCAPED_LEN				39

#define CDS_DIDL_CLASS1_ESCAPED					"\r\n&lt;upnp:class&gt;"
#define CDS_DIDL_CLASS2_ESCAPED					"&lt;/upnp:class&gt;"

#define CDS_DIDL_ITEM_START_ESCAPED				"\r\n&lt;item id=&quot;%s&quot; parentID=&quot;%s&quot; restricted=&quot;%d&quot;&gt;"
#define CDS_DIDL_ITEM_START_ESCAPED_LEN			76

#define CDS_DIDL_ITEM_START1_ESCAPED			"\r\n&lt;item id=&quot;"
#define CDS_DIDL_ITEM_START2_ESCAPED			"&quot; parentID=&quot;"
#define CDS_DIDL_ITEM_START3_ESCAPED			"&quot; restricted=&quot;"
#define CDS_DIDL_ITEM_START4_ESCAPED			"&quot;&gt;"

#define CDS_DIDL_REFITEM_START_ESCAPED			"\r\n&lt;item id=&quot;%s&quot; parentID=&quot;%s&quot; restricted=&quot;%d&quot; refID=&quot;%s&quot;&gt;"
#define CDS_DIDL_REFITEM_START_ESCAPED_LEN		95

#define CDS_DIDL_REFITEM_START1_ESCAPED			"\r\n&lt;item id=&quot;"
#define CDS_DIDL_REFITEM_START2_ESCAPED			"&quot; parentID=&quot;"
#define CDS_DIDL_REFITEM_START3_ESCAPED			"&quot; restricted=&quot;"
#define CDS_DIDL_REFITEM_START4_ESCAPED			"&quot; refID=&quot;"
#define CDS_DIDL_REFITEM_START5_ESCAPED			"&quot;&gt;"

#define CDS_DIDL_ITEM_END_ESCAPED				"\r\n&lt;/item&gt;"
#define CDS_DIDL_ITEM_END_ESCAPED_LEN			15

#define CDS_DIDL_CONTAINER_START_ESCAPED		"\r\n&lt;container id=&quot;%s&quot; parentID=&quot;%s&quot; restricted=&quot;%d&quot;&gt; searchable=&quot;%d&quot; childCount=&quot;%d&quot;"
#define CDS_DIDL_CONTAINER_START_ESCAPED_LEN	129

#define CDS_DIDL_CONTAINER_START1_ESCAPED		"\r\n&lt;container id=&quot;"
#define CDS_DIDL_CONTAINER_START2_ESCAPED		"&quot; parentID=&quot;"
#define CDS_DIDL_CONTAINER_START3_ESCAPED       "&quot; restricted=&quot;"
#define CDS_DIDL_CONTAINER_START4_ESCAPED       "&quot; searchable=&quot;"
#define CDS_DIDL_CONTAINER_START5_ESCAPED       "&quot; childCount=&quot;%d"
#define CDS_DIDL_CONTAINER_START6_ESCAPED		"&quot;&gt;"

#define CDS_DIDL_CONTAINER_END_ESCAPED			"\r\n&lt;/container&gt;"
#define CDS_DIDL_CONTAINER_END_ESCAPED_LEN		20

#define CDS_DIDL_RES_START_ESCAPED				"\r\n&lt;res protocolInfo=&quot;%s&quot;"
#define CDS_DIDL_RES_START_ESCAPED_LEN			35

#define CDS_DIDL_RES_START1_ESCAPED				"\r\n&lt;res protocolInfo=&quot;"
#define CDS_DIDL_RES_START2_ESCAPED				"&quot;"

#define CDS_DIDL_ATTRIB_RESOLUTION_ESCAPED		" resolution=&quot;%dx%d&quot;"
#define CDS_DIDL_ATTRIB_RESOLUTION_ESCAPED_LEN	24

#define CDS_DIDL_ATTRIB_DURATION_ESCAPED		" duration=&quot;%s&quot;"
#define CDS_DIDL_ATTRIB_DURATION_ESCAPED_LEN	22

#define CDS_DIDL_ATTRIB_BITRATE_ESCAPED			" bitrate=&quot;%d&quot;"
#define CDS_DIDL_ATTRIB_BITRATE_ESCAPED_LEN		21

#define CDS_DIDL_ATTRIB_COLORDEPTH_ESCAPED		" colorDepth=&quot;%d&quot;"
#define CDS_DIDL_ATTRIB_COLORDEPTH_ESCAPED_LEN	24

#define CDS_DIDL_ATTRIB_SIZE_ESCAPED			" size=&quot;%d&quot;"
#define CDS_DIDL_ATTRIB_SIZE_ESCAPED_LEN		18

#define CDS_DIDL_RES_VALUE_ESCAPED				"&gt;%s&lt;/res&gt;"
#define CDS_DIDL_RES_VALUE_ESCAPED_LEN			16

#define CDS_DIDL_RES_VALUE1_ESCAPED				"&gt;"
#define CDS_DIDL_RES_VALUE2_ESCAPED				"&lt;/res&gt;"

#define CDS_FILTER_CREATOR						"dc:creator"
#define CDS_FILTER_CREATOR_LEN					10

#define CDS_FILTER_RES							"res"
#define CDS_FILTER_RES_LEN						3

#define CDS_FILTER_RES_RESOLUTION				"res@resolution"
#define CDS_FILTER_RES_RESOLUTION_LEN			14

#define CDS_FILTER_RES_DURATION					"res@duration"
#define CDS_FILTER_RES_DURATION_LEN				12

#define CDS_FILTER_RES_BITRATE					"res@bitrate"
#define CDS_FILTER_RES_BITRATE_LEN				11

#define CDS_FILTER_RES_COLORDEPTH				"res@colorDepth"
#define CDS_FILTER_RES_COLORDEPTH_LEN			14

#define CDS_FILTER_RES_SIZE						"res@size"
#define CDS_FILTER_RES_SIZE_LEN					8

#define CDS_FILTER_CONTAINER_CHILDCOUNT			"container@childCount"
#define CDS_FILTER_CONTAINER_CHILDCOUNT_LEN		20

#define CDS_FILTER_CHILDCOUNT					"@childCount"
#define CDS_FILTER_CHILDCOUNT_LEN				11

#define CDS_FILTER_CONTAINER_SEARCHABLE			"container@searchable"
#define CDS_FILTER_CONTAINER_SEARCHABLE_LEN		20

#define CDS_FILTER_SEARCHABLE					"@searchable"
#define CDS_FILTER_SEARCHABLE_LEN				11

#endif
