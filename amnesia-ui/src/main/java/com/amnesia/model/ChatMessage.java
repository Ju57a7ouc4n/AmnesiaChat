package com.amnesia.model;

public class ChatMessage {
    private String text;
    private boolean isOutgoing;
    private long timestamp;

    public ChatMessage(String text, boolean isOutgoing) {
        this.text = text;
        this.isOutgoing = isOutgoing;
        this.timestamp = System.currentTimeMillis();
    }

    public String getText() { 
    	return text; 
    }
    public boolean isOutgoing() {
    	return isOutgoing; 
    }
    public long getTimestamp() { 
    	return timestamp; 
    }
}