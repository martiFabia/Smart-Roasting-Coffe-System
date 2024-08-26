package roastingmachine.unipi.it.sensors;

import roastingmachine.unipi.it.resources.ResourcesMan;
import roastingmachine.unipi.it.proc.CoapClientSys;
// IMPORTARE THREAD PER COMUNICAZIONE COAP

public class Sensor_co2 extends Utility_sensor{
    private static Sensor_co2 INSTANCE;

    private Sensor_co2() {
	    min = 500;
	    max = 1000;
    }

    public static Sensor_co2 getInstance() {
        if(INSTANCE == null) {
            INSTANCE = new Sensor_co2();
        }

        return INSTANCE;
    }



    public void setActionMin(){
    }

    public void setActionMax() {
        // Gestione dell'allerta CO2
        if (Sensor_co2.getInstance().getValue() > Sensor_co2.getInstance().getMax() + 100) { // Se il valore supera il massimo di 200
            ResourcesMan alertResourcesMan = ResourcesMan.retrieveInformation("alert");
            if (alertResourcesMan.getStatus().equals("off")) {
                new CoapClientSys(alertResourcesMan, "on").start();
            }
            
        }
        // Gestione della ventola
        ResourcesMan ventResourcesMan = ResourcesMan.retrieveInformation("vent");
        if (ventResourcesMan.getStatus().equals("off")) {
            new CoapClientSys(ventResourcesMan, "on").start();
        }
    }

    public void setActionOK() {
        // Gestione dell'allerta CO2
        ResourcesMan alertResourcesMan = ResourcesMan.retrieveInformation("alert");
        if (alertResourcesMan.getStatus().equals("on")) {
            new CoapClientSys(alertResourcesMan, "off").start();
        }

        // Gestione della ventola
        ResourcesMan ventResourcesMan = ResourcesMan.retrieveInformation("vent");
        if (ventResourcesMan.getStatus().equals("on")) {
            new CoapClientSys(ventResourcesMan, "off").start();
        }
    }
}


