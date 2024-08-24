package roastingmachine.unipi.it.sensors;

import roastingmachine.unipi.it.resources.ResourcesMan;
// IMPORTARE THREAD PER COMUNICAZIONE COAP

public class Sensor_humidity extends Utility_sensor{

    private static Sensor_humidity INSTANCE; //per essere sicuri che esita una sola istanza
    private int currentInterval;

    // Valori di min e max per ciascun intervallo
    private final int[][] intervalLimits = {
        {40, 60},  // Intervallo 1
        {20, 40},  // Intervallo 2
        {10, 20}   // Intervallo 3
    };

    private Sensor_humidity() {
        setInterval(1);  // Inizia con il primo intervallo per default
    }

    public static Sensor_humidity getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new Sensor_humidity();
        }

        return INSTANCE;
    }

    public void setInterval(int interval) {
        if (interval < 1 || interval > 3) {
            throw new IllegalArgumentException("Intervallo non valido. Deve essere compreso tra 1 e 3.");
        }
        currentInterval = interval;
        min = intervalLimits[currentInterval - 1][0];
        max = intervalLimits[currentInterval - 1][1];
    }

    public void setActionMin(){  
    }
    public void setActionMax(){
        ResourcesMan resourcesMan = ResourcesMan.retrieveInformation("vent");
        if(resourcesMan.getStatus().equals("off"))
            //new CoapClientThread(resourcesMan, "on").start();
    }

}