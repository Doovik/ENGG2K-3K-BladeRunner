import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;
import java.util.Random;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.json.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class CCPCombined {
    private static final int CCP_PORT = 3014;
    private static final int BUFFER_SIZE = 256;
    private static final String mcpAddress = "10.20.30.1";
    private String currentBRStatus = "STOPC";
    private boolean ackRecieved = false;

    enum Status {
        STOPC,
        STOPO,
        FSLOWC,
        FFASTC,
        RSLOWC,
        ERR,
    }

    static String client_type;
    static String message;
    static String client_id;
    static int s_ccp;
    static String action;
    static String status;
    static String br_id;
    private static final Random RANDOM = new Random();
    private DatagramSocket udpSocket;

    static JSONParser parser = new JSONParser();
    static JSONObject jsonWrite = new JSONObject();

    final static String client = "ccp";
    private static String mostRecentPacket = "";

    public CCPCombined() {
        this.s_ccp = RANDOM.nextInt(29001) + 1000;
    }

    public static void main(String[] args) {
        CCPCombined ccp = new CCPCombined();
        ccp.run();
        Boolean checkedIn = false;
        Boolean statusChanged = false;

        try (DatagramSocket socket = new DatagramSocket(CCP_PORT)) {
            byte[] buffer = new byte[BUFFER_SIZE];

            System.out.println("Server is listening on port " + CCP_PORT);

            while (true) {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                socket.receive(packet);

                String received = new String(packet.getData(), 0, packet.getLength());
                System.out.println("Received from BR: " + received);

                InetAddress address = packet.getAddress();
                int clientPort = packet.getPort();

                JSONObject toSend = new JSONObject();

                if (received.trim().equals("I am alive")) {
                    System.out.println("Received 'I am alive' message from BR.");

                    String ackMessage = "ACK";
                    DatagramPacket ackPacket = new DatagramPacket(ackMessage.getBytes(), ackMessage.length(), address, clientPort);
                    socket.send(ackPacket);
                    System.out.println("Sent to BR: " + ackMessage);

                    continue;
                }

                try {
                    JSONObject json = new JSONObject(received);
                    String messageType = json.getString("message");

                    if (!messageType.equals("AKEX")) {
                        mostRecentPacket = received;
                    }

                    String clientId = json.getString("client_id");
                    String sequenceNumber = json.getString("sequence_number");

                    if (!checkedIn) {
                        System.out.println("Sending initialization message");
                        String initiationMessage = jsonWrite(toSend, "CCIN", "BRXX", "s_ccp");
                        packet = new DatagramPacket(initiationMessage.getBytes(), initiationMessage.length(), address, clientPort);
                        socket.send(packet);
                    }

                    if (messageType.equals("AKIN")) {
                        checkedIn = true;
                    } else {
                        continue;
                    }

                    if (messageType.equals("EXEC")) {
                        String acknowledge = jsonWrite(toSend, "AKEX", "BRXX", "s_ccp");
                        packet = new DatagramPacket(acknowledge.getBytes(), acknowledge.length(), address, clientPort);
                        socket.send(packet);
                    }

                    if (messageType.equals("STRQ")) {
                        String status = "STOPC";
                        JSONObject responseJson = new JSONObject();
                        responseJson.put("client_type", "CCP");
                        responseJson.put("message", "STAT");
                        responseJson.put("client_id", clientId);
                        responseJson.put("sequence_number", sequenceNumber);
                        responseJson.put("status", status);

                        String responseMessage = responseJson.toString();
                        DatagramPacket responsePacket = new DatagramPacket(responseMessage.getBytes(), responseMessage.length(), address, clientPort);
                        socket.send(responsePacket);
                        System.out.println("Sent to client: " + responseMessage);
                    }

                    DatagramPacket espPacket = new DatagramPacket(received.getBytes(), received.length(), address, clientPort);
                    socket.send(espPacket);
                    System.out.println("Forwarded to ESP: " + received);

                    if (messageType.equals("STAT")) {
                        DatagramPacket mcpPacket = new DatagramPacket(received.getBytes(), received.length(), address, clientPort);
                        socket.send(mcpPacket);
                        System.out.println("Forwarded STAT to MCP: " + received);
                    }

                    if (statusChanged) {
                        String statusMsg = jsonWrite(toSend, "STAT", "BRXX", "s_ccp", "ERR");
                        packet = new DatagramPacket(statusMsg.getBytes(), statusMsg.length(), address, clientPort);
                        socket.send(packet);

                        statusChanged = false;
                    }

                    System.out.println("Most recent packet: " + mostRecentPacket);
                } catch (Exception ex) {
                    System.out.println("Failed to parse JSON message: " + ex.getMessage());
                    ex.printStackTrace();
                }
            }
        } catch (Exception ex) {
            System.out.println("Server exception: " + ex.getMessage());
            ex.printStackTrace();
        }
    }

    @SuppressWarnings("unchecked")
    static String jsonWrite(JSONObject jobj, String message, String clientID, String sequence_number) {
        jobj.put("client_type", client);
        jobj.put("message", message);
        jobj.put("client_id", clientID);
        jobj.put("sequence_number", sequence_number);

        return jobj.toString();
    }

    static String jsonWrite(JSONObject jobj, String message, String clientID, String sequence_number, String status) {
        jobj.put("client_type", client);
        jobj.put("message", message);
        jobj.put("client_id", clientID);
        jobj.put("sequence_number", sequence_number);
        jobj.put("status", status);

        return jobj.toString();
    }

    public void run() {
        try {
            udpSocket = new DatagramSocket(); 
            new Thread(this::sendStartupStatusToMCP).start();

            new Thread(this::listenForMCPCommands).start();

            new Thread(this::listenForBRUpdates).start();
        } catch (SocketException e) {
            System.err.println("Failed to create UDP socket: " + e.getMessage());
        }
    }

    private void sendStartupStatusToMCP() {
        ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleAtFixedRate(() -> {
            if (!ackRecieved) {
                JSONObject startupStatus = new JSONObject();
                startupStatus.put("client_type", "CCP");
                startupStatus.put("message", "CCIN");
                startupStatus.put("client_id", "BR14");
                startupStatus.put("sequence_number", s_ccp);
                startupStatus.put("status", currentBRStatus);

                forwardToMCP(startupStatus);
            } else {
                scheduler.shutdown(); // Stop scheduling once AKIN is received
            }
        }, 0, 2, TimeUnit.SECONDS);
    }

    private void listenForMCPCommands() {
        try (DatagramSocket serverSocket = new DatagramSocket(CCP_PORT)) {
            byte[] buffer = new byte[1024];

            while (true) {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                serverSocket.receive(packet);

                // Parse the MCP command
                String request = new String(packet.getData(), 0, packet.getLength(), StandardCharsets.UTF_8);
                JSONObject mcpCommand = new JSONObject(request);

                switch (mcpCommand.getString("message")) {
                    case "AKIN":
                        ackRecieved = true;
                        break;
                    case "STRQ":
                        sendCurrentStatusToMCP(mcpCommand.getString("client_id"));
                        break;
                    case "EXEC":
                        forwardToBR(mcpCommand);
                        break;
                    default:
                        break;
                }
            }
        } catch (IOException e) {
            System.err.println("Could not listen on MCP port: " + e.getMessage());
        }
    }
}