import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import org.json.JSONObject;

public class CCP_ESP {
    public static void main(String[] args) {
        int port = 3014;
        try (DatagramSocket socket = new DatagramSocket(port)) {
            byte[] buffer = new byte[256];

            System.out.println("Server is listening on port " + port);

            // Sample JSON message for testing
            String sampleJson = "{\"client_type\": \"CCP\", \"message\": \"EXEC\", \"client_id\": \"BRXX\", \"sequence_number\": \"s_mcp\", \"action\": \"STOPO\"}";
            System.out.println("Sample JSON: " + sampleJson);

            // Parse JSON message
            JSONObject json = new JSONObject(sampleJson);
            String action = json.getString("action");
            String sequenceNumber = json.getString("sequence_number");

            // Create message to send to ESP32
            String messageToSend = sequenceNumber + "," + action;
            System.out.println("Message to send: " + messageToSend);

            while (true) {
                // Receive packet
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
                socket.receive(packet);

                String received = new String(packet.getData(), 0, packet.getLength());
                System.out.println("Received from client: " + received);

                // Send response to client
                InetAddress address = packet.getAddress();
                int clientPort = packet.getPort();
                DatagramPacket responsePacket = new DatagramPacket(messageToSend.getBytes(), messageToSend.length(), address, clientPort);
                socket.send(responsePacket);
                System.out.println("Sent to client: " + messageToSend);
            }
        } catch (Exception ex) {
            System.out.println("Server exception: " + ex.getMessage());
            ex.printStackTrace();
        }
    }
}