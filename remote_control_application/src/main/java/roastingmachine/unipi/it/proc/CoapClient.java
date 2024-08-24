package roastingmachine.unipi.it.proc;

import roastingmachine.unipi.it.resources.ResourcesMan;
import org.eclipse.californium.core.*;
import org.json.simple.JSONObject;

public class CoapClient extends Thread{

    private ResourcesMan resourcesMan;
    private String payload;

    public CoapClient(ResourcesMan resourcesMan, String payload) {
        this resourcesMan = resourcesMan;
        this.payload = payload;
    }

    public void run() {
        String uri = "coap://" resourcesMan.getIp()+"/" resourcesMan.getResource();
        CoapClient client = new CoapClient(uri);
        Request req = new Request(CoAP.Code.PUT);
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("action", payload);
        req.setPayload(jsonObject.toJSONString());
        req.getOptions().setAccept(MediaTypeRegistry.APPLICATION_JSON);
        CoapResponse response = client.advanced(req);
        if (response!=null) {
            CoAP.ResponseCode code = response.getCode();
            switch (code) {
                case CHANGED:
                 resourcesMan.updateStatus(payload);
                 System.out.println("Stato aggiornato!\n");
                    break;
                case BAD_REQUEST:
                    System.err.println("Internal application error!");
                    break;
                case BAD_OPTION:
                    System.err.println("BAD_OPTION error");
                    break;
                default:
                    System.err.println("Actuator error!");
                 resourcesMan.changeStatus("Error");
                    break;
            }
        }
    }
}