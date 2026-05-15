package com.amnesia.main;

import com.amnesia.bridge.EngineBridge;
import com.amnesia.controller.Controller;
import com.amnesia.factory.Factory;
import com.amnesia.view.MainView;

public class Main {
    
    public static void main(String[] args) {
        Factory factory = Factory.getInstance();
        Controller controller = factory.getController();
        MainView view = factory.getView();
        EngineBridge bridge = factory.getBridge();
        controller.setView(view);
        controller.setBridge(bridge);
        view.setController(controller);
        controller.start();
    }
}