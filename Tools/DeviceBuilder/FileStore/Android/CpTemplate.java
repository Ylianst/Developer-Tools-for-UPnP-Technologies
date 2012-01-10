package {{{PACKAGE}}};

import java.util.jar.Attributes;
import opentools.upnp.UPnPService;
import opentools.upnp.GenericInvokeHandler;
import opentools.upnp.UPnPServiceEventHandler;
import opentools.upnp.UPnPService_FinishedParsingSCPD;
import opentools.upnp.UPnPStateVariable;

public class {{{CLASSNAME}}} 
{
	public static String ServiceType = "{{{SERVICETYPE}}}";
	public EventHandler EventCallback;
{{{METHODS}}}
		
	private UPnPService mService;
	private UPnPServiceEventHandler mEventHandler = new UPnPServiceEventHandler()
	{
		@Override
		public void OnUPnPEvent(UPnPService sender, Attributes eventedParameters) 
		{
{{{EVENTPROCESSOR}}}
		}
	};
	
	public {{{CLASSNAME}}} (UPnPService service)
	{
		mService = service;
		mService.EventedParametersCallback = mEventHandler;
		EventCallback = null;
		RefreshActions();
	}
	


	public void RefreshActions()
	{
{{{REFRESH_ACTIONS}}}
	}
	public void SubscribeEvents()
	{
		mService.SubscribeForEvents();
	}

{{{INNER_CLASSES}}}
{{{INTERFACES}}}
{{{QUERY_METHODS}}}

}