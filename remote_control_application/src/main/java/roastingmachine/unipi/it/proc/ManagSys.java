package roastingmachine.unipi.it.proc;

import roastingmachine.unipi.it.sensors.*;

import java.sql.*;
import java.util.*;

/* Con questa classe periodicamente controllo i valori inseriti nel db e verifico se è necessario avviare
l'azione di un attuatore in caso valori anormali */


public class ManagSys extends Thread{

    private static final String url = "jdbc:mysql://localhost:3306/RoastingMachine";
    private static final String username = "root";
    private static final String password = "root1234";

    private static Map<String, Sensor> type; //hash map per i tipi di sensori
    private static Timestamp lastTimestamp; //mantengo l'ultimo timestamp per leggere solo eventuali valori nuovi
    private static Map<String,Integer> values; //hash map per i nuovi valori letti

    public ManagSys{
        Date now = new Date();
        lastTimestamp = new java.sql.Timestamp(now.getTime());

        values = new HashMap<>();
        type = new HashMap<>();

        type.put("humidity", Sensor_humidity.getInstance());
        type.put("co2", Sensor_co2.getInstance());
        type.put("temperature", Sensor_temp.getInstance());
    }

    @Override
    public void run() {
	
	while(true){        
		try {
		    sleep(2*1000);     //2 seconds
		    // mi connetto al database
		    try (Connection connection = DriverManager.getConnection(url, username, password)) {
			
		        Date now = new Date();
		        Timestamp tempTimestamp = new java.sql.Timestamp(now.getTime());     
				for (String str : type.keySet()) { //le key sono i tipi di sensore, quindi facciamo la stessa cosa per ogni sensore
		            PreparedStatement ps = connection.prepareStatement(
		                    "SELECT value FROM dataSensed WHERE timestamp > ? AND type = ? ORDER BY timestamp DESC LIMIT 1;");
		            ps.setTimestamp(1, lastTimestamp);
		            ps.setString(2, str);
		            ResultSet res = ps.executeQuery();
		            while (res.next()) {
		                int value = res.getInt("value");

		                values.put(str, value);
		                type.get(str).setValue(value);
		            }
		        }
			lastTimestamp = tempTimestamp; //aggiorno il timestamp
		    } catch (SQLException e) {
		        System.err.println("Cannot connect the database!");
		    }
		    // check values
		    if(!values.isEmpty()){                      // ResultSet non è vuoto ----> sono stati registrati nuovi valori
                for(String type : type.keySet()){
		            if(values.containsKey(type)){
		                int value = values.get(type);
				
		                String action = null;
		            
		                if(value < type.get(type).getMin()){
		                    type.get(type).setActionMin();
		                }else if(value >= type.get(type).getMax()){
		                    type.get(type).setActionMax();
		                }else{ //se sono qui vuol dire che i valori sono nello standard
                            type.get(type).setActionOK();
                        }
		            }
		        }

		    }

		} catch (InterruptedException e) {
		    throw new RuntimeException(e);
		}
		values.clear();
	}
    }

}