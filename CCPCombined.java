//UDP imports
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
//JSON imports
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;
import org.json.JSONObject;
import java.util.Scanner;
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
        try (DatagramSocket socket = new DatagramSocket(PORT)) {
            byte[] buffer = new byte[BUFFER_SIZE];
            Scanner scanner = new Scanner(System.in);

            System.out.println("Server is listening on port " + PORT);

            // Process real and mock inputs
            while (true) {
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                socket.receive(packet);

                String received = new String(packet.getData(), 0, packet.getLength());
                System.out.println("Received from BR: " + received);

                // Check if the message is "I am alive"
                if (received.trim().equals("I am alive")) {
                    System.out.println("Received 'I am alive' message from BR.");

                    // Send acknowledgment back to the client
                    String ackMessage = "ACK";
                    InetAddress address = packet.getAddress();
                    int clientPort = packet.getPort();
                    DatagramPacket ackPacket = new DatagramPacket(ackMessage.getBytes(), ackMessage.length(), address, clientPort);
                    socket.send(ackPacket);
                    System.out.println("Sent to BR: " + ackMessage);

                    // Prompt user for EXEC mock message
                    System.out.println("Enter EXEC mock message JSON:");
                    String execMockMessage = scanner.nextLine();
                    DatagramPacket execPacket = new DatagramPacket(execMockMessage.getBytes(), execMockMessage.length(), address, clientPort);
                    socket.send(execPacket);
                    System.out.println("Sent to BR: " + execMockMessage);

                    // Prompt user for STRQ mock message
                    System.out.println("Enter STRQ mock message JSON:");
                    String strqMockMessage = scanner.nextLine();
                    DatagramPacket strqPacket = new DatagramPacket(strqMockMessage.getBytes(), strqMockMessage.length(), address, clientPort);
                    socket.send(strqPacket);
                    System.out.println("Sent to BR: " + strqMockMessage);

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

                    InetAddress address = packet.getAddress();
                    int clientPort = packet.getPort();

                    if (messageType.equals("STRQ")) {
                        // Handle MCP Status Request Message
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
                    } else if (messageType.equals("EXEC")) {
                        // Handle MCP Command Message
                        String action = json.getString("action");
                        // Perform the action (not implemented in this example)

                        JSONObject responseJson = new JSONObject();
                        responseJson.put("client_type", "CCP");
                        responseJson.put("message", "AKEX");
                        responseJson.put("client_id", clientId);
                        responseJson.put("sequence_number", sequenceNumber);

                        String responseMessage = responseJson.toString();
                        DatagramPacket responsePacket = new DatagramPacket(responseMessage.getBytes(), responseMessage.length(), address, clientPort);
                        socket.send(responsePacket);
                        System.out.println("Sent to client: " + responseMessage);
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

    @SuppressWarnings("unchecked")
    static String jsonWrite(JSONObject jobj, String message, String clientID, String sequence_number, String status) {
        jobj.put("client_type", client);
        jobj.put("message", message);
        jobj.put("client_id", clientID);
        jobj.put("sequence_number", sequence_number);
        jobj.put("status", status);

        return jobj.toString();
    }

    static void jsonRead() {
        try {
            JSONObject obj = (JSONObject) parser.parse(new FileReader("C:\\ENGG3000_BR_2\\New folder\\ENGG2K-3K-BladeRunner\\test.json")); // Get JSONObject
            // fetching values from JSON Object
            client_type = (String) obj.get("client_type");
            message = (String) obj.get("message");
            client_id = (String) obj.get("client_id");
            sequence_number = (String) obj.get("sequence number");
            action = (String) obj.get("action");
            status = (String) obj.get("status");
            br_id = (String) obj.get("br_id");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ParseException e) {
            e.printStackTrace();
        }
    }
}