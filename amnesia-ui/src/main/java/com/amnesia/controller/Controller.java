package com.amnesia.controller;

import com.amnesia.bridge.EngineBridge;
import com.amnesia.bridge.EngineListener;
import com.amnesia.view.MainView;

import javax.swing.JOptionPane;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;

public class Controller implements EngineListener, ActionListener {
    private static Controller instance = null;
    private MainView view;
    private EngineBridge bridge;
    private int currentActiveChatId = -1;

    private Controller() {
        super();
    }
    
    public static Controller getInstance() {
        if(instance == null) {
            instance = new Controller();
        }
        return instance;
    }

    public void setView(MainView v) {
        this.view = v;
    }

    public void start() {
        view.setVisible(true);
        try {
            bridge.start("../amnesia-engine/amnesia_engine");
        } catch (IOException e) {
            view.appendToChat("ERROR: Can't start engine: " + e.getMessage());
        }
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        String command = e.getActionCommand();
        
        if ("CMD_SEND".equals(command)) {
            String text = view.getMessageText();
            if (!text.isEmpty() && currentActiveChatId != -1) {
                bridge.sendCommand("SEND|" + currentActiveChatId + "|" + text);
                view.appendToChat("[Yo]: " + text);
                view.clearInput();
            }
            
        } else if ("CMD_NEW_CHAT".equals(command)) {
            String input = JOptionPane.showInputDialog(view, "Input IP:PORT to connect, or a port to start the beacon:");
            if (input != null && !input.trim().isEmpty()) {
                if (input.contains(":")) {
                    int newChatId = (int)(Math.random() * 1000);
                    currentActiveChatId = newChatId;
                    bridge.sendCommand("NEW|" + newChatId + "|" + input);
                } else {
                    bridge.sendCommand("FARO|0|" + input);
                }
            }
        }
    	else if ("CMD_PANIC".equals(command)) {
        bridge.sendCommand("PANIC|0|NOW");
    }
}

    @Override
    public void onMessageReceived(String rawResponse) {
        javax.swing.SwingUtilities.invokeLater(() -> {
            processEngineCommand(rawResponse);
        });
   	}
    
    private void processEngineCommand(String rawResponse) {
        String[] parts = rawResponse.split("\\|", 3);
        if (parts.length < 3) return;

        String cmd = parts[0];
        int chatId = Integer.parseInt(parts[1]);
        String payload = parts[2];

        switch (cmd) {
            case "FARO_OK":
                view.appendToChat("Beacon started on port " + payload);
                break;
            case "NEW_OK":
                view.addChatToList("Chat " + chatId);
                view.appendToChat("Connected to chat " + chatId);
                break;
            case "INC_CHAT":
                view.addChatToList("Chat " + chatId + " (" + payload + ")");
                view.appendToChat("¡Someone Connected! Chat ID: " + chatId);
                currentActiveChatId = chatId;
                break;
            case "INC_MSG":
                if (chatId == currentActiveChatId) {
                    view.appendToChat("[Peer]: " + payload);
                }
                break;
            case "HIST_CHUNK":
                if (chatId == currentActiveChatId) {
                    view.appendToChat("[Peer]: " + payload);
                }
                break;
        }
    }

    @Override
    public void onEngineError(String errorLog) {
        System.err.println("Engine: " + errorLog);
    }

    @Override
    public void onEngineClosed() {
        System.out.println("Engine Off.");
        System.exit(0);
    }
    
    public void setBridge(EngineBridge bridge) {
        this.bridge = bridge;
        this.bridge.setListener(this); 
    }
}