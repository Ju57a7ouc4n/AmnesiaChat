package com.amnesia.view;

import javax.swing.*;
import com.amnesia.controller.Controller;
import java.awt.*;

public class MainView extends JFrame {
    private JList<String> chatList;
    private DefaultListModel<String> listModel;
    private JTextArea chatArea;
    private JTextField inputField;
    private JButton sendButton;
    private JButton newChatButton;
    private Controller controller; 
    
    public MainView() {
        super("Amnesia P2P | By ju57a70uc4n");
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setSize(900, 600);
        this.setLocationRelativeTo(null);

        JTabbedPane tabs = new JTabbedPane();
        tabs.addTab("Chats", createChatsPanel());
        tabs.addTab("Donate", createDonatePanel());

        this.add(tabs);
    }

    private JPanel createChatsPanel() {
        JPanel panel = new JPanel(new BorderLayout());
        JPanel leftPanel = new JPanel(new BorderLayout());
        
        newChatButton = new JButton("+ New Chat / Beacon");
        newChatButton.setActionCommand("CMD_NEW_CHAT");
        leftPanel.add(newChatButton, BorderLayout.NORTH);

        listModel = new DefaultListModel<>();
        chatList = new JList<>(listModel);
        chatList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        JScrollPane listScroll = new JScrollPane(chatList);
        listScroll.setPreferredSize(new Dimension(200, 0));
        leftPanel.add(listScroll, BorderLayout.CENTER);

        JPanel rightPanel = new JPanel(new BorderLayout());
        
        chatArea = new JTextArea();
        chatArea.setEditable(false);
        chatArea.setLineWrap(true);
        rightPanel.add(new JScrollPane(chatArea), BorderLayout.CENTER);

        JPanel inputPanel = new JPanel(new BorderLayout());
        inputField = new JTextField();
        inputField.setActionCommand("CMD_SEND"); 
        
        sendButton = new JButton("Send");
        sendButton.setActionCommand("CMD_SEND");
        
        inputPanel.add(inputField, BorderLayout.CENTER);
        inputPanel.add(sendButton, BorderLayout.EAST);
        rightPanel.add(inputPanel, BorderLayout.SOUTH);

        JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, leftPanel, rightPanel);
        panel.add(splitPane, BorderLayout.CENTER);

        return panel;
    }

    private JPanel createDonatePanel() {
        JPanel panel = new JPanel(new GridBagLayout());
        panel.add(new JLabel("Billetera Monero (XMR): Próximamente..."));
        return panel;
    }
    
    public void appendToChat(String text) {
        chatArea.append(text + "\n");
        chatArea.setCaretPosition(chatArea.getDocument().getLength());
    }
    
    public void clearChat() { 
        chatArea.setText("");
    }
    
    public String getMessageText() { 
        return inputField.getText();
    }
    
    public void clearInput() { 
        inputField.setText(""); 
    }
    
    public void addChatToList(String chatName) {
        if (!listModel.contains(chatName)) listModel.addElement(chatName);
    }

 
    public void setController(Controller c) {
        this.controller = c;
        sendButton.addActionListener(this.controller);
        inputField.addActionListener(this.controller); 
        newChatButton.addActionListener(this.controller);
    }
}