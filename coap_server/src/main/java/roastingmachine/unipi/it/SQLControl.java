package roastingmachine.unipi.it;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

import java.net.InetAddress;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;

public class SQLControl extends CoapResource {

    private static final String url = "jdbc:mysql://localhost:3306/RoastingMachine";
    private static final String username = "root";
    private static final String password = "root1234";

    public SQLControl(String name) {  //Costruttore che inizializza la risorsa
        super(name);
    }


    public void handlePOST(CoapExchange exchange) {  //gestore delle richieste POST, ossia le richieste di registrazione mandate dagli attuatori
        byte[] request = exchange.getRequestPayload();
        String s = new String(request); //Trasformo il payload in stringa
        JSONObject json = null;
        try {
            JSONParser parser = new JSONParser(); 
            json = (JSONObject) parser.parse(s); //converto la stringa in oggetto json
        }catch (Exception err){
            System.err.println("Formato non valido");
        }

        Response response;
        if (json.containsKey("name")){  //controllo se è presente il campo "name"
            InetAddress addr = exchange.getSourceAddress(); //prendo l'indirizzo della risorsa
		    System.out.println(addr);
            try (Connection connection = DriverManager.getConnection(url, username, password)) { //mi connetto al db
                PreparedStatement ps = connection.prepareStatement("REPLACE INTO actuators (ip,resource,status) VALUES(?,?,?);");
                ps.setString(1,String.valueOf(addr).substring(1));
                ps.setString(2, (String)json.get("name"));
                ps.setString(3,(String)json.get("status"));
                ps.executeUpdate();
                if(ps.getUpdateCount()<1){ //se nessuna riga è stata inserita/modificata
                    response = new Response(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
                    System.out.println("nessuna riga inserita\n");
                }else{
                    response = new Response(CoAP.ResponseCode.CREATED);
                    System.err.println("Risorsa inserita!\n");
                }
            } catch (SQLException e) {
                response = new Response(CoAP.ResponseCode.INTERNAL_SERVER_ERROR);
                System.err.println("Impossibile connettersi al database\n");
            }

        }else{
            response = new Response(CoAP.ResponseCode.BAD_REQUEST);
        }
        exchange.respond(response);
    }
}