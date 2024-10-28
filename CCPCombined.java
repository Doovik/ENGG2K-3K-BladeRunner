import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import org.json.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class CCPCombined {
    private static final int PORT = 3014;
    private static final int BUFFER_SIZE = 256;

    // Enum for current carriage status
    enum Status {
        STOPC, // stopped + door closed
        STOPO, // stopped + door opened
        FSLOWC, // forward, slow, door closed
        FFASTC, // forward, fast, door closed
        RSLOWC, // backwards, slow, door closed
        ERR, // CPP <-> BRC contact lost
    }

    // Initialize empty values for jsonRead
    static String client_type;
    static String message;
    static String client_id;
    static String sequence_number;
    static String action;
    static String status;
    static String br_id;

    // Initialize JSONParser
    static JSONParser parser = new JSONParser();
    static JSONObject jsonWrite = new JSONObject();

    final static String client = "ccp";
    private static String mostRecentPacket = "";

    public static void main(String[] args) {
        Boolean checkedIn = false;
        Boolean statusChanged = false;

        try (DatagramSocket socket = new DatagramSocket(PORT)) {
            byte[] buffer = new byte[BUFFER_SIZE];

            System.out.println("Server is listening on port " + PORT);

            while (true) {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                socket.receive(packet);

                String received = new String(packet.getData(), 0, packet.getLength());
                System.out.println("Received from BR: " + received);

                InetAddress address = packet.getAddress();
                int clientPort = packet.getPort();

                JSONObject toSend = new JSONObject();

                // Check if the message is "I am alive"
                if (received.trim().equals("I am alive")) {
                    System.out.println("Received 'I am alive' message from BR.");

                    // Send acknowledgment back to the client
                    String ackMessage = "ACK";
                    DatagramPacket ackPacket = new DatagramPacket(ackMessage.getBytes(), ackMessage.length(), address, clientPort);
                    socket.send(ackPacket);
                    System.out.println("Sent to BR: " + ackMessage);

                    continue;
                }

                // Parse the received message
                try {
                    JSONObject json = new JSONObject(received);
                    String messageType = json.getString("message");

                    // Store the most recent packet if it's not an AKEX message
                    if (!messageType.equals("AKEX")) {
                        mostRecentPacket = received;
                    }

                    String clientId = json.getString("client_id");
                    String sequenceNumber = json.getString("sequence_number");

                    // Send initialisation message if not sent or not acknowledged
                    if (!checkedIn) {
                        System.out.println("Sending initialization message");
                        String initiationMessage = jsonWrite(toSend, "CCIN", "BRXX", "s_ccp");
                        packet = new DatagramPacket(initiationMessage.getBytes(), initiationMessage.length(), address, clientPort);
                        socket.send(packet);
                    }

                    // Check if initialisation was acknowledged
                    if (messageType.equals("AKIN")) {
                        checkedIn = true;
                    } else {
                        continue;
                    }

                    // Acknowledge received message
                    if (messageType.equals("EXEC")) {
                        String acknowledge = jsonWrite(toSend, "AKEX", "BRXX", "s_ccp");
                        packet = new DatagramPacket(acknowledge.getBytes(), acknowledge.length(), address, clientPort);
                        socket.send(packet);
                    }

                    // Handle MCP Status Request Message
                    if (messageType.equals("STRQ")) {
                        String status = "STOPC"; // Example status, replace with actual status
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

                    // CCP status message
                    // TODO send upon completing an action + actual status
                    if (statusChanged) {
                        String statusMsg = jsonWrite(toSend, "STAT", "BRXX", "s_ccp", "ERR");
                        packet = new DatagramPacket(statusMsg.getBytes(), statusMsg.length(), address, clientPort);
                        socket.send(packet);

                        statusChanged = false;
                    }

                    // Print the most recent packet
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
}