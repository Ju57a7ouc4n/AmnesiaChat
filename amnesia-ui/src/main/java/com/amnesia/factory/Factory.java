package com.amnesia.factory;

import com.amnesia.bridge.EngineBridge;
import com.amnesia.controller.Controller;
import com.amnesia.view.MainView;

public class Factory {
    private static Factory instance = null;
    
    private Factory() {
        super();
    }
    
    public static Factory getInstance() {
        if(instance == null) {
            instance = new Factory();
        }
        return instance;
    }
    
    public Controller getController() {
        return Controller.getInstance();
    }
    
    public MainView getView() {
        return new MainView();
    }
    
    public EngineBridge getBridge() {
        return new EngineBridge(); 
    }
}