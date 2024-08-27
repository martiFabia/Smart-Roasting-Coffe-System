package roastingmachine.unipi.it.sensors;

import roastingmachine.unipi.it.resources.ResourcesMan;
import roastingmachine.unipi.it.proc.CoapClientSys;


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

    public void setInterval(int currentInterval) {
        if (currentInterval < 1 || currentInterval > 3) {
            throw new IllegalArgumentException("Intervallo non valido. Deve essere compreso tra 1 e 3.");
        }
        this.interval = currentInterval;
        this.min = intervalLimits[currentInterval - 1][0];
        this.max = intervalLimits[currentInterval - 1][1];
    }

      // Metodo per ottenere il valore minimo per un intervallo
    public int getMin_Hum(int inter) {
        if (inter < 1 || inter > intervalLimits.length) {
            throw new IllegalArgumentException("Intervallo non valido");
        }
        return intervalLimits[inter - 1][0];
    }

    // Metodo per ottenere il valore massimo per un intervallo
    public int getMax_Hum(int inter) {
        if (inter < 1 || inter > intervalLimits.length) {
            throw new IllegalArgumentException("Intervallo non valido");
        }
        return intervalLimits[inter - 1][1];
    }

    public void setMin_Hum(int inter, int value){
        if (inter < 1 || inter > intervalLimits.length) {
        throw new IllegalArgumentException("Intervallo non valido");
    }
        intervalLimits[inter - 1][0] = value;
    }

    public void setMax_Hum(int inter, int value){
        if (inter < 1 || inter > intervalLimits.length) {
        throw new IllegalArgumentException("Intervallo non valido");
    }
        intervalLimits[inter - 1][1] = value;
    }

    public void setActionMin(){  
        ResourcesMan resourcesMan = ResourcesMan.retrieveInformation("reg_temp");
        if(resourcesMan.getStatus().equals("off") || resourcesMan.getStatus().equals("down") )
             new CoapClientSys(resourcesMan, "up").start();

        ResourcesMan res_vent = ResourcesMan.retrieveInformation("vent");
        if(res_vent.getStatus().equals("on")){
            new CoapClientSys(res_vent, "off").start();
        }
    }
    public void setActionMax(){
        ResourcesMan resourcesMan = ResourcesMan.retrieveInformation("vent");
        if(resourcesMan.getStatus().equals("off"))
             new CoapClientSys(resourcesMan, "on").start();
    }

    public void setActionOK(){
        ResourcesMan resourcesMan = ResourcesMan.retrieveInformation("vent");
        if(resourcesMan.getStatus().equals("on"))
             new CoapClientSys(resourcesMan, "off").start();

        ResourcesMan res_reg = ResourcesMan.retrieveInformation("reg_temp");
        if(res_reg.getStatus().equals("up") || res_reg.getStatus().equals("down")){
             new CoapClientSys(res_reg, "off").start();
        }
    }

}