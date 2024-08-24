package roastingmachine.unipi.it.sensors;

import roastingmachine.unipi.it.resources.ResourcesMan;
// IMPORTARE THREAD PER COMUNICAZIONE COAP

public class Sensor_temp extends Utility_sensor{
    private static Sensor_temp INSTANCE;

    private Sensor_temp() {
	    min = 180;
	    max = 220;
    }

    public static Sensor_temp getInstance() {
        if(INSTANCE == null) {
            INSTANCE = new Sensor_temp();
        }

        return INSTANCE;
    }

     public static Sensor_temp getInstance() {
        if(INSTANCE == null) {
            INSTANCE = new Sensor_temp();
        }

        return INSTANCE;
    }

    public void setActionMin(){
        ResourcesMan resourcesMan = ResourcesMan.retrieveInformation("reg_temp");
        if(resourcesMan.getStatus().equals("off") || resourcesMan.getStatus().equals("down") )
            //new CoapClientThread(resourcesMan, "up").start();
    }

     public void setActionMax(){
        ResourcesMan resourcesMan = ResourcesMan.retrieveInformation("reg_temp");
        if(resourcesMan.getStatus().equals("up") || resourcesMan.getStatus().equals("off"))
            //new CoapClientThread(resourcesMan, "down").start();
    }

}


/* da ricontrollare, se i valori tornano normali come si torna off?, è notte ora */