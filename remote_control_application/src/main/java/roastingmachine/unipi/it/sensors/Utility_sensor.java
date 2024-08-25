package roastingmachine.unipi.it.sensors;

/*
classe utilizzata per implementare le varie funzioni di utilità utilizzate dalle altre classi
*/ 

public abstract class Utility_sensor{ //classe che per essere utilizzata deve essere estesa da altre classi

    protected int min;
    protected int max;
    protected int value;
    protected int interval;


    public void setMin(int min) {
        this.min = min;
    }

    public void setMax(int max) {
        this.max = max;
    }

    public void setValue(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public int getMin() {
        return min;
    }

    public int getMax() {
        return max;
    }

    public int getInterval() {
        return this.interval;
    }

    public abstract void setActionMin();
    public abstract void setActionMax();
    public abstract void setActionOK();


}