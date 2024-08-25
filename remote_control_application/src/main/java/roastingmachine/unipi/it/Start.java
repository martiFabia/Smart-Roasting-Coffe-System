package roastingmachine.unipi.it;


import roastingmachine.unipi.it.proc.ManagSys;

public class Start {
    public static void main(String args[]){
        
        ManagSys managSys = new ManagSys();

        
        managSys.start();
        System.out.println("\nManage system start!\n");
    }
}