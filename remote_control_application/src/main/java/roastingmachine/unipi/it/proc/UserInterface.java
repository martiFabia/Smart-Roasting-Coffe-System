package roastingmachine.unipi.it.proc;

import roastingmachine.unipi.it.resources.ResourcesMan;
import org.json.simple.JSONObject;
import org.eclipse.paho.client.mqttv3.*;
import roastingmachine.unipi.it.sensors.*;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;


public class UserInterface extends Thread{

    private static final String broker = "tcp://127.0.0.1:1883";
    private static final String clientId = "RemoteControlApp";
    private static Map<String, Boolean> is_changed = new HashMap<>();

    @Override
    public void run(){

        BufferedReader reader = new BufferedReader(new InputStreamReader(System.in));

        while(true){

            print_intro();
            print_command();
            String comando;

            try {
                comando = reader.readLine(); //leggo l'input
            } catch (IOException e) {
                throw new RuntimeException(e);
            }

            comando = comando.toLowerCase(); //input è case unsensitive

            switch(comando){
                case "/help":
                    print_command();
                    break;
                case "/show_actuators_status":
                    System.out.println("|                                              |");
                    System.out.println("|  Type the actuator's name to see its status  |");
                    System.out.println("|  1. /vent                                    |");
                    System.out.println("|  2. /reg_temp                                |");
                    System.out.println("|  3. /alert                                   |");
                    System.out.println("|  4. /all                                     |");
                    System.out.println("|                                              |");
                    try {
                        comando = reader.readLine();
                        show_actuator(comando);
                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }
                    break;
                case "/show_parameters":
                    System.out.println("|                                              |");
                    System.out.println("| Type the sensor's name to see its parameters |");
                    System.out.println("|  1. /co2                                     |");
                    System.out.println("|  2. /humidity                                |");
                    System.out.println("|  3. /temperature                             |");
                    System.out.println("|                                              |");
                    try {
                        comando = reader.readLine();
                        show_params(comando);
                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }
                    break;

                case "/change_parameters":
                    System.out.println("|                                              |");
                    System.out.println("| Type the parameter you want to change,       |");
                    System.out.println("| then insert an integer                       |");
                    System.out.println("|  1. /min_humidity_parameter_FIRST            |");
                    System.out.println("|  2. /max_humidity_parameter_FIRST            |");
                    System.out.println("|  3. /min_humidity_parameter_SECOND           |");
                    System.out.println("|  4. /max_humidity_parameter_SECOND           |");
                    System.out.println("|  5. /min_humidity_parameter_THIRD            |");
                    System.out.println("|  6. /max_humidity_parameter_THIRD            |");
                    System.out.println("|  7. /min_co2_parameter                       |");
                    System.out.println("|  8. /max_co2_parameter                       |");
                    System.out.println("|  9. /min_temp_parameter                      |");
                    System.out.println("|  10. /max_temp_parameter                     |");
                    try {
                        comando = reader.readLine();
                        if(comando.equals("0")){
                               break;
                            }
                        String value = reader.readLine();
                        int val = Integer.parseInt(value);
                        change_parameters(comando,val);
                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }

                    try {
                        MqttClient client = new MqttClient(broker, clientId);
                        client.connect();
                        String content;
                        for (String topic : is_changed.keySet()) {
                            send_mqtt(client, "param/" + topic);
                        }
                        client.disconnect();

                    } catch (MqttException e) {
                        e.printStackTrace();
                    }
                    break;

                case "/change_actuators_status":
                default: 
                    print_help();
            }
        }

    }

    private void send_mqtt(MqttClient client, String topic) throws MqttException{

    }


    private void change_parameters(String comando, int value){
        switch(comando){
            case "/min_humidity_parameter_FIRST":
                Sensor_humidity.getInstance().setMin_Hum(1, value);
                is_changed.put("humidity", true);
                break;
            case "/max_humidity_parameter_FIRST":
                Sensor_humidity.getInstance().setMax_Hum(1, value);
                is_changed.put("humidity", true);
                break;
            case "/min_humidity_parameter_SECOND":
                Sensor_humidity.getInstance().setMin_Hum(2, value);
                is_changed.put("humidity", true);
                break;
            case "/max_humidity_parameter_SECOND":
                Sensor_humidity.getInstance().setMax_Hum(2, value);
                is_changed.put("humidity", true);
                break;
            case "/min_humidity_parameter_THIRD":
                Sensor_humidity.getInstance().setMin_Hum(3, value);
                is_changed.put("humidity", true);
                break;
            case "/max_humidity_parameter_THIRD":
                Sensor_humidity.getInstance().setMax_Hum(3, value);
                is_changed.put("humidity", true);
                break;
            case "/min_co2_parameter":
                Sensor_co2.getInstance().setMin(value);
                is_changed.put("co2", true);
                break;
            case "/max_co2_parameter":
                Sensor_co2.getInstance().setMax(value);
                is_changed.put("co2", true);
                break;
            case "/min_temp_parameter":
                Sensor_temp.getInstance().setMin(value);
                is_changed.put("temp", true);
                break;
            case "/max_temp_parameter":
                Sensor_co2.getInstance().setMax(value);
                is_changed.put("temp", true);
                break;
            default:
                print_help();
        }
    }

    private void show_params(String comando){
        comando = comando.toLowerCase();

        if(!comando.equals("/co2") && !comando.equals("/humidity") && !comando.equals("/temperature") && !comando.equals("/all")){
            print_help();
            return;
        }

        String resource = comando.substring(1); //per togliere /

        switch(resource){
            case "humidity":
                for(int i = 1; i < 4; i++){
                    System.out.println(String.format("Humidity, min e max intervallo %d:\n", i));
                    int minh = Sensor_humidity.getInstance().getMin_Hum(i);
                    int maxh = Sensor_humidity.getInstance().getMax_Hum(i);
                    System.out.println("min: " + minh);
                    System.out.println("max: " + maxh);
                    System.out.println("\n");
                }
                break;
            case "temperature":
                int mint = Sensor_temp.getInstance().getMin();
                int maxt = Sensor_temp.getInstance().getMax();
                System.out.println("min: " + mint);
                System.out.println("max: " + maxt);
                System.out.println("\n");
                break;
            case "co2":
                int minc = Sensor_co2.getInstance().getMin();
                int maxc = Sensor_co2.getInstance().getMax();
                System.out.println("min: " + minc);
                System.out.println("max: " + maxc);
                System.out.println("\n");
                break;
            default:
                System.out.println("Errore");
        }
 
    }
    private void show_actuator(String comando){

        comando = comando.toLowerCase();
        
        if(!comando.equals("/vent") && !comando.equals("/reg_temp") && !comando.equals("/alert") && !comando.equals("/all")){
            print_help();
            return;
        }

        if(comando.equals("/all")){
            ResourcesMan res_vent = ResourcesMan.retrieveInformation("vent");
            ResourcesMan res_reg = ResourcesMan.retrieveInformation("reg_temp");
            ResourcesMan res_al = ResourcesMan.retrieveInformation("alert");

            System.out.println(res_vent);
            System.out.println(res_reg);
            System.out.println(res_al);
        }else{

            String resource = comando.substring(1); //per togliere /

            ResourcesMan res = ResourcesMan.retrieveInformation(resource);
            System.out.println(res);
        }
    }

    private void print_intro(){
        System.out.println("**************** CONTROL CENTER ****************");
        System.out.println("|                                              |");
    }

    private void print_command(){
        System.out.println("|                                              |");
        System.out.println("|  Type the keyword of the command you desire  |");
        System.out.println("|  to run. Remember to type '/' first!         |");
        System.out.println("|                                              |");
        System.out.println("|  1. /SHOW_ACTUATORS_STATUS                   |");
        System.out.println("|  2. /SHOW_PARAMETERS                         |");
        System.out.println("|  2. /CHANGE_PARAMETERS                       |");
        System.out.println("|  3. /CHANGE_ACTUATORS_STATUS                 |");
        System.out.println("|                                              |");
    }

    private void print_help(){
        System.out.println("|                                              |");
        System.out.println("|  Error. Wrong command.                       |");
        System.out.println("|  Type '/help' to show all commands avaiable  |");
        System.out.println("|                                              |");
    }
    

    
}