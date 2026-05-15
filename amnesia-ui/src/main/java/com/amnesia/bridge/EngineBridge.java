package com.amnesia.bridge;

import java.io.*;
import java.nio.charset.StandardCharsets;

public class EngineBridge {
    private Process process;
    private BufferedWriter writer;
    private BufferedReader reader;
    private BufferedReader errorReader;
    private EngineListener listener;
    private boolean running;

    public EngineBridge() {
        super();
    }

    public void start(String enginePath) throws IOException {
        ProcessBuilder pb = new ProcessBuilder(enginePath);
        this.process = pb.start();
        this.running = true;
        this.writer = new BufferedWriter(new OutputStreamWriter(process.getOutputStream(), StandardCharsets.UTF_8));
        this.reader = new BufferedReader(new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8));
        this.errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream(), StandardCharsets.UTF_8));
        startListenerThread();
        startErrorThread();
    }

    public void sendCommand(String command) {
        try {
            if (writer != null) {
                writer.write(command);
                writer.newLine();
                writer.flush();
            }
        } catch (IOException e) {
            listener.onEngineError("Failed to send command: " + e.getMessage());
        }
    }

    private void startListenerThread() {
        new Thread(() -> {
            try {
                String line;
                while (running && (line = reader.readLine()) != null) {
                    listener.onMessageReceived(line);
                }
            } catch (IOException e) {
                if (running) listener.onEngineError("Read Error: " + e.getMessage());
            } finally {
                stop();
            }
        }, "Amnesia-Engine-Listener").start();
    }

    private void startErrorThread() {
        new Thread(() -> {
            try {
                String line;
                while (running && (line = errorReader.readLine()) != null) {
                    listener.onEngineError(line);
                }
            } catch (IOException e) {
            }
        }, "Amnesia-Engine-Logs").start();
    }

    public void stop() {
        running = false;
        try {
            if (process != null) {
                process.destroy();
            }
            if (writer != null) writer.close();
            if (reader != null) reader.close();
            if (errorReader != null) errorReader.close();
            listener.onEngineClosed();
        } catch (IOException e) {
        	listener.onEngineError("FAILED TO STOP ENGINE. KILL THE PROCESS");
        }
    }
    
    public void setListener(EngineListener e) {
    	this.listener = e;
    } 
}