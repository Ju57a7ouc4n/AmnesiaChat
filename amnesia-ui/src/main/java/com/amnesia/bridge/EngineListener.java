package com.amnesia.bridge;

public interface EngineListener {
    void onMessageReceived(String rawResponse);
    void onEngineError(String errorLog);
    void onEngineClosed();
}