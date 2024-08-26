package roastingmachine.unipi.it.proc;

import roastingmachine.unipi.it.sensors.Sensor_co2;
import roastingmachine.unipi.it.sensors.Sensor_humidity;
import roastingmachine.unipi.it.sensors.Sensor_temp;
import roastingmachine.unipi.it.sensors.Utility_sensor;

import java.sql.*;
import java.util.HashMap;
import java.util.Map;
import java.util.Date;

/* Con questa classe periodicamente controllo i valori inseriti nel db e verifico se è necessario avviare
l'azione di un attuatore in caso valori anormali */

public class ManagSys extends Thread {

    private static final String url = "jdbc:mysql://localhost:3306/RoastingMachine";
    private static final String username = "root";
    private static final String password = "root1234";

    private static Map<String, Utility_sensor> sens; // Hash map per i tipi di sensori
    private static Timestamp lastTimestamp; // Mantengo l'ultimo timestamp per leggere solo eventuali valori nuovi
    private static Map<String, Integer> values; // Hash map per i nuovi valori letti

    public ManagSys() {
        java.util.Date date = new java.util.Date();
        lastTimestamp = new java.sql.Timestamp(date.getTime());

        values = new HashMap<>();
        sens = new HashMap<>();

        sens.put("humidity", Sensor_humidity.getInstance());
        sens.put("co2", Sensor_co2.getInstance());
        sens.put("temperature", Sensor_temp.getInstance());
    }

    @Override
    public void run() {
        while (true) {
            try {
                sleep(2 * 1000); // Dormi per 2 secondi

                // Connessione al database
                try (Connection connection = DriverManager.getConnection(url, username, password)) {

                    java.util.Date date = new java.util.Date();
                    Timestamp tempTimestamp = new java.sql.Timestamp(date.getTime());

                    for (String str : sens.keySet()) {
                        PreparedStatement ps = connection.prepareStatement(
                                "SELECT value, `interval` FROM dataSensed WHERE timestamp > ? AND type = ? ORDER BY timestamp DESC LIMIT 1;");
                        ps.setTimestamp(1, lastTimestamp);
                        ps.setString(2, str);
                        ResultSet res = ps.executeQuery();
                        while (res.next()) {
                            int value = res.getInt("value");
                            int interv = res.getInt("interval");

                            values.put(str, value);
                            sens.get(str).setValue(value);

                             if (str.equals("humidity")) {
                                // Setta l'intervallo per il sensore di umidità in base a quello letto dal database
                                ((Sensor_humidity) sens.get(str)).setInterval(interv);
                            }
                        }
                    }
                    lastTimestamp = tempTimestamp; // Aggiorna il timestamp

                } catch (SQLException e) {
                    System.err.println("Cannot connect to the database!");
                    e.printStackTrace();
                }

                // Controlla i valori
                if (!values.isEmpty()) {
                    for (String type : sens.keySet()) {
                        if (values.containsKey(type)) {
                            int value = values.get(type);
                                if(value < sens.get(type).getMin()){
                                    sens.get(type).setActionMin();
                                } else if (value >= sens.get(type).getMax()) {
                                    sens.get(type).setActionMax();
                                } else {
                                    sens.get(type).setActionOK();
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
