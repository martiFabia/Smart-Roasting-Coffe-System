
/*

classe che ci permette di comunicare con gli attuatori

*/


package roastingmachine.unipi.it.proc;

import roastingmachine.unipi.it.resources.ResourcesMan;
import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Request;
import org.json.simple.JSONObject;

public class CoapClientSys extends Thread{

    private ResourcesMan resourcesMan;
    private String action;

    public CoapClientSys(ResourcesMan resourcesMan, String action) {
        this.resourcesMan = resourcesMan;
        this.action = action;
    }

    public void run() {
        String uri = "coap://"+ resourcesMan.getIp()+"/"+ resourcesMan.getResource();

        CoapClient client = new CoapClient(uri);
        
        Request request = new Request(CoAP.Code.PUT);
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("action", action);
        request.setPayload(jsonObject.toJSONString());
        request.getOptions().setAccept(MediaTypeRegistry.APPLICATION_JSON);
        CoapResponse response = client.advanced(request);
        if (response!=null) {
            CoAP.ResponseCode code = response.getCode();
            switch (code) {
                case CHANGED:
                    resourcesMan.updateStatus(action);
                    break;
                case BAD_REQUEST:
                    System.err.println("Bad Request");
                    break;
                case BAD_OPTION:
                    System.err.println("Bad Option");
                    break;
                default:
                    System.err.println("Errore");
                 resourcesMan.changeStatus("Error");
                    break;
            }
        }
    }
}