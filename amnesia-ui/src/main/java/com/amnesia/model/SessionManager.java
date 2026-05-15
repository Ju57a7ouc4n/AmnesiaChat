package com.amnesia.model;

import java.util.HashMap;
import java.util.Map;

public class SessionManager {
    private Map<Integer, ChatSession> sessions;

    public SessionManager() {
        this.sessions = new HashMap<>();
    }

    public void createSession(int chatId, String peerIp) {
        if (!sessions.containsKey(chatId)) {
            sessions.put(chatId, new ChatSession(chatId, peerIp));
        }
    }

    public ChatSession getSession(int chatId) {
        return sessions.get(chatId);
    }

    public void removeSession(int chatId) {
        sessions.remove(chatId);
    }

    public void clearAll() {
        sessions.clear();
    }

    public boolean hasSession(int chatId) {
        return sessions.containsKey(chatId);
    }
}