package roastingmachine.unipi.it.resources;

import java.sql.*;

public class ResourcesMan {

    private static final String url = "jdbc:mysql://localhost:3306/RoastingMachine";
    private static final String username = "root";
    private static final String password = "root1234";

    private String ip;
    private String resource;
    private String status;

    public ResourcesMan(String ip, String resource) {
        this.ip = ip;
        this.resource = resource;
    }

    public ResourcesMan(String ip, String resource, String status) {
        this.ip = ip;
        this.resource = resource;
        this.status = status;
    }

     public static ResourcesMan retrieveInformation(String actuator){
        String resource = null;
        ResourcesMan resourcesMan = null;
        
        switch(actuator){
            case "reg_temp":
                resource = "actuator_reg_temp";
                break;
            case "alert":
                resource = "actuator_alert";
                break;
            case "vent":
                resource = "actuator_vent";
		        break;
	        default:
		        return resourcesMan;
        }
        try (Connection connection = DriverManager.getConnection(url, username, password)) {

            PreparedStatement ps = connection.prepareStatement("SELECT ip,status FROM actuators WHERE resource = ? LIMIT 1;");
            ps.setString(1,resource);
            ResultSet res = ps.executeQuery();
            while(res.next()){
                String ip = res.getString("ip");
                String status = res.getString("status");
                resourcesMan = new ResourcesMan(ip,resource,status);
            }
        } catch (SQLException e) {
            System.err.println("Cannot connect the database!");
        }
        return resourcesMan;
    }


    public void changeStatus(String new_status){

        if(new_status.equals(status)){
            return;
        }

        try (Connection connection = DriverManager.getConnection(url, username, password)) {

            PreparedStatement ps = connection.prepareStatement("UPDATE actuators SET status = ? WHERE ip=?;");
            ps.setString(1,new_status);
            ps.setString(2,ip);
            int row_changed = ps.executeUpdate();
            if (row_changed == 0)
                    throw new Exception();
            else
                status = new_status;
        } catch (SQLException e) {
            System.err.println("Cannot connect the database!");
        }catch (Exception e){
            System.err.println("Error while updating the status on the DB!");
        }
    }

    public void updateStatus(String command){
        if (resource.equals("actuator_vent")){
            if(command.equals("on"))
                changeStatus("on");
            else if(command.equals("off"))
                changeStatus("off");
        }else if(resource.equals("actuator_alert")){
            if(command.equals("on"))
                changeStatus("on");
            else if(command.equals("off"))
                changeStatus("off");
        }else if(resource.equals("actuator_reg_temp")){
            if(command.equals("off"))
                changeStatus("off");
            else if(command.equals("up"))
                changeStatus("up");
            else if(command.equals("down"))
                changeStatus("down");
        }
    }

     
    public String getIp() {
        return "["+ip+"]";
    }

    public String getResource() {
        return resource;
    }

    public String getStatus() {
        return status;
    }

    @Override
    public String toString() {
        return "Actuator status{\n" +
                "\tip = [" + ip + ']' +
                ",\n\tresource = '" + resource + '\'' +
                ",\n\tstatus = '" + status + '\'' +
                "\n}";
    }

}