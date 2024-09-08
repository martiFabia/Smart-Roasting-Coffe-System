package roastingmachine.unipi.it;

import roastingmachine.unipi.it.SQLControl;
import org.eclipse.californium.core.CoapServer;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Reg_Server extends CoapServer {

    public static void main(String args[]) {
        Logger.getLogger("org.eclipse.californium.core.network").setLevel(Level.WARNING);

        Reg_Server server = new Reg_Server();
        server.add(new SQLControl("registration"));
        server.start();
        System.out.println("\nServer start!\n");
    }

}