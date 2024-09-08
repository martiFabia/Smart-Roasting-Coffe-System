package roastingmachine.unipi.it;

import java.util.logging.Level;
import java.util.logging.Logger;
import roastingmachine.unipi.it.proc.ManagSys;
import roastingmachine.unipi.it.proc.UserInterface;

public class Start {
    public static void main(String args[]){
        
        Logger.getLogger("org.eclipse.californium.core.network").setLevel(Level.WARNING);

        ManagSys managSys = new ManagSys();
        UserInterface us = new UserInterface();

        
        managSys.start();
        us.start();
        System.out.println("\nManage system start!\n");
    }
}