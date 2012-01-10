package {{{PACKAGE}}};

import java.util.UUID;

import opentools.ILib.RefParameter;
import opentools.upnp.UPnPControlPoint;
import opentools.upnp.UPnPDevice;
import opentools.upnp.UPnPDeviceHandler;
import opentools.upnp.UPnPService;
import opentools.upnp.UPnPStateVariable;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class {{{PROJECTNAME}}} extends Activity 
{
//{{{CP_DECLARATION}}}
//{{{DV_DECLARATION}}}

	
//{{{CP_HANDLERS}}}

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
//{{{DV_INIT}}}
        
//{{{DV_HANDLERS}}}

        final Button startButton = (Button)findViewById(R.id.Button01);
        final Button stopButton = (Button)findViewById(R.id.Button02);
        stopButton.setEnabled(false);
        
        startButton.setOnClickListener(new OnClickListener()
        {
			@Override
			public void onClick(View v) 
			{
//{{{CP_INIT}}}
//{{{DV_START}}}
				startButton.setEnabled(false);
				stopButton.setEnabled(true);
			}
        });
        stopButton.setOnClickListener(new OnClickListener()
        {
			@Override
			public void onClick(View v) 
			{
//{{{CP_STOP}}}
//{{{DV_STOP}}}
				finish();
			}
        });
    }
}