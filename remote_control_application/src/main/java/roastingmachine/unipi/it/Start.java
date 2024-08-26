package roastingmachine.unipi.it;


import roastingmachine.unipi.it.proc.ManagSys;
import roastingmachine.unipi.it.proc.UserInterface;

public class Start {
    public static void main(String args[]){
        
        ManagSys managSys = new ManagSys();
        UserInterface us = new UserInterface();

        
        managSys.start();
        us.start();
        System.out.println("\nManage system start!\n");
    }
}