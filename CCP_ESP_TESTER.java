import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Scanner;
import org.json.JSONObject;

public class CCP_ESP_TESTER {
    private static final int PORT = 3014;
    private static final int BUFFER_SIZE = 256;

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

                    // Prompt user for EXEC mock message file name
                    System.out.println("Enter EXEC mock message file name (without .json):");
                    String execFileName = scanner.nextLine() + ".json";
                    String execMockMessage = new String(Files.readAllBytes(Paths.get(execFileName)));
                    DatagramPacket execPacket = new DatagramPacket(execMockMessage.getBytes(), execMockMessage.length(), address, clientPort);
                    socket.send(execPacket);
                    System.out.println("Sent to BR: " + execMockMessage);

                    // Prompt user for STRQ mock message file name
                    System.out.println("Enter STRQ mock message file name (without .json):");
                    String strqFileName = scanner.nextLine() + ".json";
                    String strqMockMessage = new String(Files.readAllBytes(Paths.get(strqFileName)));
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
}