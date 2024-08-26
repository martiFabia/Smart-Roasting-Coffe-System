package roastingmachine.unipi.it;

import roastingmachine.unipi.it.SQLControl;
import org.eclipse.californium.core.CoapServer;

public class Reg_Server extends CoapServer {

    public static void main(String args[]) {
        Reg_Server server = new Reg_Server();
        server.add(new SQLControl("registration"));
        server.start();
        System.out.println("\nServer start!\n");
    }

}