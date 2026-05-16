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
    private JButton panicButton;
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

        panicButton = new JButton("⚠️ PANIC BUTTON");
        panicButton.setActionCommand("CMD_PANIC");
        panicButton.setBackground(new Color(180, 0, 0));
        panicButton.setForeground(Color.WHITE);
        panicButton.setFont(new Font("SansSerif", Font.BOLD, 13));
        panicButton.setFocusPainted(false);
        leftPanel.add(panicButton, BorderLayout.SOUTH);

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
        GridBagConstraints gbc = new GridBagConstraints();
        
        gbc.insets = new Insets(12, 12, 12, 12);
        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.anchor = GridBagConstraints.CENTER;

        JLabel label = new JLabel("<html><div style='text-align: center; font-family: sans-serif; font-size: 11px;'>"
                + "<b style='font-size: 14px;'>Thanks for using AmnesiaChat!</b><br><br>"
                + "You can tip me on the following Bitcoin Wallet:</div></html>");
        panel.add(label, gbc);
        
        gbc.gridy = 1;
        String walletAddress = "bc1qv5m6ydtdx3et3dys8kwk5m59jr5p4r3vqn760u";
        JTextField walletField = new JTextField(walletAddress);
        walletField.setEditable(false);
        walletField.setFont(new Font("Monospaced", Font.PLAIN, 12));
        walletField.setBackground(new Color(240, 240, 240));
        walletField.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(Color.LIGHT_GRAY),
                BorderFactory.createEmptyBorder(8, 10, 8, 10)
        ));
        panel.add(walletField, gbc);

        gbc.gridy = 2;
        JButton copyButton = new JButton("📋 Copy Address");
        copyButton.setFocusPainted(false);
        copyButton.setFont(new Font("SansSerif", Font.BOLD, 12));
        
        copyButton.addActionListener(e -> {
            try {
                java.awt.datatransfer.StringSelection selection = new java.awt.datatransfer.StringSelection(walletAddress);
                java.awt.datatransfer.Clipboard clipboard = java.awt.Toolkit.getDefaultToolkit().getSystemClipboard();
                clipboard.setContents(selection, null);
                
                JOptionPane.showMessageDialog(panel, "Address successfully copied to clipboard!", "Copied", JOptionPane.INFORMATION_MESSAGE);
            } catch (Exception ex) {
                JOptionPane.showMessageDialog(panel, "Error accessing the clipboard.", "Error", JOptionPane.ERROR_MESSAGE);
            }
        });
        
        panel.add(copyButton, gbc);

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
        panicButton.addActionListener(this.controller); // <-- Registramos el botón al controlador
    }
}