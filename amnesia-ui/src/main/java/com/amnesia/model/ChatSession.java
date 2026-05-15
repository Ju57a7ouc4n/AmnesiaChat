package com.amnesia.model;

import java.util.ArrayList;
import java.util.List;

public class ChatSession {
    private int chatId;
    private String peerIp;
    private List<ChatMessage> messages;

    public ChatSession(int chatId, String peerIp) {
        this.chatId = chatId;
        this.peerIp = peerIp;
        this.messages = new ArrayList<>();
    }

    public int getChatId() { 
    	return chatId; 
    }
    public String getPeerIp() { 
    	return peerIp; 
    }

    public List<ChatMessage> getMessages() { 
        return new ArrayList<>(messages); 
    }

    public void addMessage(ChatMessage message) {
        this.messages.add(message);
    }
}